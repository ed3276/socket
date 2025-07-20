#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <fcntl.h>
#include <poll.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

extern int h_errno;

int setnonblocking(int fd) {
	int flags;

	if ((flags=fcntl(fd, F_GETFL, 0)) == -1) {
		flags = 0;
	}
	return fcntl(fd, F_SETFL, flags|O_NONBLOCK);
}

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./client 192.168.74.130 8279" << std::endl;
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
	std::string sendBuf, recvBuf;
	std::string line;

	std::cout << "please input: ";
	while (std::getline(std::cin, line)) {  // 读取整行（包括空格）
        if (line.empty()) {
            std::cout << "please input: ";
            continue;
        }
		sendBuf = line;
		if (send(sockfd, sendBuf.data(), sendBuf.size(), 0) < 0) {
			perror("send");
			exit(0);
		}

		recvBuf.resize(byteN);
		if ((recvN = recv(sockfd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
			if (recvN < 0) {
				perror("recv");
				exit(0);
			}
			break;
		}
		recvBuf.resize(recvN);
		std::cout << "receive from  " << inet_ntoa(serverAddr.sin_addr) << ":" << ntohs(serverAddr.sin_port);
		std::cout <<" [" << recvBuf << ']' << std::endl;
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "please input: ";
	}
	close(sockfd);
	std::cout << "colsed" << std::endl;
	exit(0);
}
