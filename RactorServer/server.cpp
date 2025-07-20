#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "InetAddress.hpp"
#include "Socket.hpp"

extern int h_errno;

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./server 192.168.74.130 8279" << std::endl;
		exit(0);
	}

    Socket servsock(CreateNoBlocking());
    InetAddress servaddr(argv[1], std::stoi(argv[2]));
    servsock.SetReuseAddr(true);
    servsock.SetReusePort(true);
    servsock.SetTcpNoDelay(true);
    servsock.SetKeepAlive(true);
    servsock.Bind(servaddr);
    servsock.Listen();

    printf("listen on socket fd: %d\n", servsock.Fd());

	unsigned long long totalClientNum = 0, activeClientNum = 0;

	int timeout = 10000;
	int retval = 0;
	int epfd = -1;
	struct epoll_event ev;
	const int maxfd = 10;
	struct epoll_event evs[maxfd];
	ev.data.fd = servsock.Fd();
	ev.events = EPOLLIN;
	
	epfd = epoll_create(1);
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, servsock.Fd(), &ev) < 0) {
		perror("epoll_ctl");
		exit(1);
	}
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
				if (evs[i].events & EPOLLRDHUP) {
					printf("client fd(%d) disconnected\n", tmp.data.fd);
					close(tmp.data.fd);
				} else if (evs[i].events & (EPOLLIN | EPOLLPRI)) {
					if (tmp.data.fd == servsock.Fd()) {

						InetAddress clientaddr;
						Socket *pClientsock = new Socket(servsock.Accept(clientaddr));
						ev.data.fd = pClientsock->Fd();
						ev.events = EPOLLIN|EPOLLET;
						if (epoll_ctl(epfd, EPOLL_CTL_ADD, pClientsock->Fd(), &ev) < 0) {
							perror("epoll_ctl");
							continue;
						}
						++totalClientNum, ++activeClientNum;
						printf("accept client fd(%d) [%llu/%llu] %s:%d ok\n", pClientsock->Fd(), activeClientNum, totalClientNum, clientaddr.Ip(), clientaddr.Port());
					} else {
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
								printf("client fd(%d) disconnected\n", tmp.data.fd);
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
					}
				} else if (evs[i].events & EPOLLOUT) {

				} else {
					printf("client fd(%d) error\n", tmp.data.fd);
					close(tmp.data.fd);
					--activeClientNum;
				}
			}
		}
	}

	close(servsock.Fd());
	std::cout << "colsed" << std::endl;
	close(epfd);
	exit(0);
}
