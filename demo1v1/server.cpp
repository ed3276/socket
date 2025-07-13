#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <iostream>
#include <string>

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
	
	while(true) {
		if ((clientfd = accept(listenfd, (struct sockaddr*)&clientAddr, &len)) < 0) {
			perror("accept");
			continue;
		}
		++totalClientNum, ++activeClientNum;
		std::cout << "connected client[" << activeClientNum << "/" << totalClientNum << "] " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
		while(true) {
			recvBuf.resize(byteN);
			if ((recvN = recv(clientfd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
				if (recvN < 0) {
					perror("recv");
				} else {
				    std::cout << "client colsed " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
				}
			    --activeClientNum;
				close(clientfd);
				break;
			} else {
				recvBuf.resize(recvN);
				std::cout << "receive from " << inet_ntoa(serverAddr.sin_addr) << ":" << ntohs(serverAddr.sin_port);
				std::cout <<" [" << recvBuf << ']' << std::endl;
				sendBuf = std::string("OK");
				if (send(clientfd, sendBuf.data(), sendBuf.size(), 0) < 0) {
					perror("send");
			        --activeClientNum;
					close(clientfd);
					break;
				}

			}
		}
	}

	close(listenfd);
	std::cout << "colsed" << std::endl;
	exit(0);
}
