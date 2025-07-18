#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <poll.h>
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

	int nfds = 0;
	int retval = 0;
	int timeout = 10000;
	std::vector<struct pollfd> fds;
	struct pollfd pfd;
	pfd.fd = listenfd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);
	
	while(true) {
	    nfds = fds.size();
		retval = poll(&fds[0], nfds, timeout);
		if (retval < 0) {
			perror("poll()");
		} else if (retval == 0) {
			std::cout << "poll timeout..." << std::endl;
			continue;
		} else {
			for (int i = 0; i < nfds; ++i) {
				struct pollfd &tmp = fds.at(i);
				if ((tmp.revents & POLLIN) == 0) continue;
				if (tmp.fd == listenfd) {
					if ((clientfd = accept(tmp.fd, (struct sockaddr*)&clientAddr, &len)) < 0) {
						perror("accept()");
						continue;
					} else {
						pfd.fd = clientfd;
						pfd.events = POLLIN;
						pfd.revents = 0;
						fds.push_back(pfd);
						++totalClientNum, ++activeClientNum;
						std::cout << "connected client[" << activeClientNum << "/" << totalClientNum << "] ";
						std::cout << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
					}
				} else {
					recvBuf.resize(byteN);
					if ((recvN = recv(tmp.fd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
						if (recvN < 0) {
							perror("recv");
						} else {
							std::cout << "colsed fd(" << tmp.fd << ")" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
						}
						--activeClientNum;
						close(tmp.fd);
						tmp.fd = -1;
					} else {
						recvBuf.resize(recvN);
						std::cout << "receive from fd(" << tmp.fd << ")";
						std::cout <<" [" << recvBuf << ']' << std::endl;
						sendBuf = std::string("OK");
						if (send(tmp.fd, sendBuf.data(), sendBuf.size(), 0) < 0) {
							perror("send()");
							--activeClientNum;
							close(tmp.fd);
						    tmp.fd = -1;
						}
					}
				}
			}
		    fds.erase(std::remove_if(fds.begin(), fds.end(), [](struct pollfd &e)->bool { return e.fd == -1; }), fds.end());
		}
	}

	close(listenfd);
	std::cout << "colsed" << std::endl;
	exit(0);
}
