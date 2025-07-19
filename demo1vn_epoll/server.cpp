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
#include <iostream>
#include <string>
#include <algorithm>

extern int h_errno;

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

	int reuse = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt");
		close(listenfd);
		exit(1);
	}
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(std::stoi(argv[1]));
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind");
		exit(0);
	}

	if (listen(listenfd, 5) < 0) {
		perror("listen");
		exit(0);
	}

	std::cout << "listen on socket fd: " << listenfd << std::endl;

	unsigned long long totalClientNum = 0, activeClientNum = 0;
	int clientfd;
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	std::string sendBuf, recvBuf;
	ssize_t byteN = 1024, recvN = 0;

	const int maxfd = 10;
	int timeout = 10000;
	int retval = 0;
	int epfd = -1;
	struct epoll_event ev;
	struct epoll_event conn[maxfd];
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;
	
	epfd = epoll_create(1);
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0) {
		perror("epoll_ctl");
		exit(1);
	}
	while(true) {
		retval = epoll_wait(epfd, &conn[0], maxfd, timeout);
		if (retval < 0) {
			perror("poll()");
			exit(1);
		} else if (retval == 0) {
			std::cout << "epoll timeout..." << std::endl;
			continue;
		} else {
			for (int i = 0; i < retval; ++i) {
				struct epoll_event &tmp = conn[i];
				if ((tmp.events & EPOLLIN) == 0) continue;
				if (tmp.data.fd == listenfd) {
					if ((clientfd = accept(tmp.data.fd, (struct sockaddr*)&clientAddr, &len)) < 0) {
						perror("accept()");
						continue;
					} else {
						ev.events = EPOLLIN;
						ev.data.fd = clientfd;
						if (epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev) < 0) {
							perror("epoll_ctl");
							exit(1);
						}
						++totalClientNum, ++activeClientNum;
						std::cout << "connected client[" << activeClientNum << "/" << totalClientNum << "] ";
						std::cout << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
					}
				} else {
					recvBuf.resize(byteN);
					if ((recvN = recv(tmp.data.fd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
						if (recvN < 0) {
							perror("recv");
						} else {
							std::cout << "colsed fd(" << tmp.data.fd << ")" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
						}
						--activeClientNum;
						close(tmp.data.fd);
					} else {
						recvBuf.resize(recvN);
						std::cout << "receive from fd(" << tmp.data.fd << ")";
						std::cout <<" [" << recvBuf << ']' << std::endl;
						sendBuf = std::string("OK");
						if (send(tmp.data.fd, sendBuf.data(), sendBuf.size(), 0) < 0) {
							perror("send()");
							--activeClientNum;
							close(tmp.data.fd);
						}
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
