#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <sys/select.h>
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

	fd_set rfds, rfds_res;
	struct timeval tv, tv_res;
	int maxfd, maxfd_res;
	int retval;

	tv.tv_sec = 10;
	tv.tv_usec = 0;
	FD_ZERO(&rfds);
    FD_SET(listenfd, &rfds);
	maxfd = std::max(maxfd, listenfd);
	
	while(true) {
        rfds_res = rfds;
		maxfd_res = maxfd;
		tv_res = tv;
		retval = select(maxfd+1, &rfds_res, NULL, NULL, &tv_res);
		if (retval < 0) {
			perror("select()");
		} else if (retval == 0) {
			std::cout << "select timeout..." << std::endl;
			continue;
		} else {
			for (int fd = 0; fd <= maxfd_res; ++fd) {
				if (!FD_ISSET(fd, &rfds_res)) continue;
				if (fd == listenfd) {
					if ((clientfd = accept(listenfd, (struct sockaddr*)&clientAddr, &len)) < 0) {
						perror("accept()");
						continue;
					} else {
						FD_SET(clientfd, &rfds);
						maxfd = std::max(maxfd, clientfd);
						++totalClientNum, ++activeClientNum;
						std::cout << "connected client[" << activeClientNum << "/" << totalClientNum << "] ";
						std::cout << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
					}
				} else {
					recvBuf.resize(byteN);
					if ((recvN = recv(fd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
						if (recvN < 0) {
							perror("recv");
						} else {
							std::cout << "colsed fd(" << fd << ")" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
						}
						--activeClientNum;
						close(fd);
						FD_CLR(fd, &rfds);
						for (int j=maxfd; j>=0; --j) {
							if (FD_ISSET(j, &rfds)) {
								maxfd = j;
								break;
							}
						}
					} else {
						recvBuf.resize(recvN);
						std::cout << "receive from fd(" << fd << ")";
						std::cout <<" [" << recvBuf << ']' << std::endl;
						sendBuf = std::string("OK");
						if (send(fd, sendBuf.data(), sendBuf.size(), 0) < 0) {
							perror("send()");
							--activeClientNum;
							close(fd);
							FD_CLR(fd, &rfds);
							for (int j=maxfd; j>=0; --j) {
								if (FD_ISSET(j, &rfds)) {
									maxfd = j;
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	close(listenfd);
	std::cout << "colsed" << std::endl;
	exit(0);
}
