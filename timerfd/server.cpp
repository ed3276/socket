#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <iostream>
#include <string>
#include <algorithm>

extern int h_errno;

int setnonblocking(int fd) {
	int flags;

	if ((flags=fcntl(fd, F_GETFL, 0)) == -1) {
		flags = 0;
	}
	return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: ./server 8279" << std::endl;
		exit(0);
	}
	int listenfd;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("socket");
		exit(0);
	}

	int optval = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	setsockopt(listenfd, TCP_NODELAY, SO_KEEPALIVE, &optval, sizeof(optval));

    setnonblocking(listenfd);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(std::stoi(argv[1]));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind");
		exit(0);
	}

	if (listen(listenfd, 128) < 0) {
		perror("listen");
		exit(0);
	}

	std::cout << "listen on socket fd: " << listenfd << std::endl;

	unsigned long long totalClientNum = 0, activeClientNum = 0;
	int clientfd;
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);

	int timeout = 10000;
	int retval = 0;
	int epfd = -1;
	struct epoll_event ev;
	const int maxfd = 10;
	struct epoll_event evs[maxfd];
	ev.events = EPOLLIN;
	//ev.events = EPOLLIN|EPOLLET;
	ev.data.fd = listenfd;
	
	epfd = epoll_create(1);
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
		perror("epoll_ctl");
		exit(1);
	}
/*
    //把定时器加入epoll
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
    struct itimerspec timerout;
    bzero(&timerout, sizeof(timerout));
    timerout.it_value.tv_sec = 5;
    timerout.it_value.tv_nsec = 0;
    timerfd_settime(tfd, 0, &timerout, 0);
    ev.data.fd = tfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);
*/
    //把信号加入epoll
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    sigprocmask(SIG_BLOCK, &sigset, 0);
    int sigfd = signalfd(-1, &sigset, 0);
    ev.data.fd = sigfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sigfd, &ev);

	while(true) {
		retval = epoll_wait(epfd, &evs[0], maxfd, timeout);
		if (retval < 0) {
			perror("poll()");
			exit(1);
		} else if (retval == 0) {
			std::cout << "epoll timeout..." << std::endl;
			continue;
		} else {
			for (int i = 0; i < retval; ++i) {
				struct epoll_event &tmp = evs[i];
                /*
                if (tmp.data.fd == tfd) {
                    printf("定时器的时间到了。\n");
                    timerfd_settime(tfd, 0, &timerout, 0);
                    continue;
                }
                */
                
                if (tmp.data.fd == sigfd) {
                    struct signalfd_siginfo siginfo;
                    int s = read(sigfd, &siginfo, sizeof(struct signalfd_siginfo));
                    printf("收到了信号 %d。\n", siginfo.ssi_signo);
                    continue;
                }

				if (tmp.data.fd == listenfd) {
					if ((clientfd = accept(tmp.data.fd, (struct sockaddr*)&clientAddr, &len)) < 0) {
						perror("accept()");
					} else {
						setnonblocking(clientfd);
						ev.data.fd = clientfd;
						ev.events = EPOLLIN|EPOLLET;
						if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev) < 0) {
							perror("epoll_ctl");
							continue;
						}
						++totalClientNum, ++activeClientNum;
                        printf("accept client[%llu/%llu] %s:%d ok\n", activeClientNum, totalClientNum,
						       inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
					}
				} else {
                    if (evs[i].events & EPOLLRDHUP) {
						printf("client fd(%d) %s:%d disconnected\n", tmp.data.fd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); 
						close(tmp.data.fd);
					} else if (evs[i].events & (EPOLLIN | EPOLLPRI)) {
						std::string sendBuf, recvBuf;
						ssize_t byteN = 1024, recvN = 0;
						while (true) {
							recvBuf.resize(byteN);
							recvN = recv(tmp.data.fd, &recvBuf[0], recvBuf.size(), 0);
							if (recvN > 0) {
								printf("receive from fd(%d) [%s]\n", tmp.data.fd, recvBuf.c_str());
								sendBuf = std::string(recvBuf, 0, recvN);
								send(tmp.data.fd, sendBuf.data(), sendBuf.size(), 0);
							} else if (recvN < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
								break;
							} else if (recvN < 0 && errno == EINTR) {
							    continue;
							} else if (recvN == 0) {
							    printf("client fd(%d) %s:%d disconnected\n", tmp.data.fd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); 
							    close(tmp.data.fd);
								--activeClientNum;
                                break;
							} else {
								perror("recv");
							    close(tmp.data.fd);
								--activeClientNum;
                                break;
							}
						}
					} else if (evs[i].events & EPOLLOUT) {

					} else {
					    printf("client fd(%d) %s:%d error\n", tmp.data.fd, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); 
					    close(tmp.data.fd);
					    --activeClientNum;
					}
				}
			}
		}
	}

	close(listenfd);
	std::cout << "colsed" << std::endl;
	close(epfd);
	exit(0);
}
