#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <string.h>       //
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

extern int h_errno;

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./client 192.168.74.142 8279" << std::endl;
		exit(0);
	}
	int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(0);
	}
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(std::stoi(argv[2]));
	struct hostent *host = gethostbyname(argv[1]);
	if (!host) {
		std::cerr << "gethostbyname error for host" << argv[1] << hstrerror(h_errno) << std::endl;
		exit(0);
	}
    serverAddr.sin_addr = *(struct in_addr*)(host->h_addr);

	if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(sockaddr_in)) < 0) {
		perror("connect");
		exit(0);
	}
	std::cout << "connect to server " << inet_ntoa(serverAddr.sin_addr) << ":" << ntohs(serverAddr.sin_port) << std::endl;

	ssize_t byteN = 1024, recvN = 0;
    uint32_t len;
    int num = 100000;
	std::string sendBuf, recvBuf;
	recvBuf.resize(byteN);

    printf("开始时间: %d\n", time(0));

	for (int i = 1; i <= num; ++i) {
		sendBuf = std::string("这是第") + std::to_string(i) + "个消息";
        len = sendBuf.size();
        sendBuf = std::string(sizeof(len), ' ') + sendBuf; 
        memcpy(&sendBuf[0], &len, sizeof(len));
		if (send(sockfd, sendBuf.data(), sendBuf.size(), 0) < 0) {
			perror("send");
		    exit(0);
		}
        //printf("send to %s:%d [%s]\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), sendBuf.data()+sizeof(len));

        recvN = recv(sockfd, (char*)&len, sizeof(len), 0);
		if ((recvN = recv(sockfd, &recvBuf[0], len, 0)) <= 0) {
			if (recvN < 0) {
				perror("recv");
				exit(0);
			}
			break;
		}
        //printf("receive from  %s:%d [%s]\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvBuf.c_str());
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}

    printf("结束时间: %d\n", time(0));

	close(sockfd);
	std::cout << "colsed" << std::endl;
	exit(0);
}
