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
#include "Epoll.hpp"

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

    Epoll ep;
    Channel *servchannel = new Channel(&ep, servsock.Fd());
    servchannel->EnableReading();

	while(true) {
        std::vector<Channel*> channels = ep.Loop();
		for (auto &ch : channels) {
			if (ch->Revents() & EPOLLRDHUP) {
				printf("client fd(%d) disconnected\n", ch->Fd());
				close(ch->Fd());
			} else if (ch->Revents() & (EPOLLIN | EPOLLPRI)) {
				if (ch == servchannel) {

					InetAddress clientaddr;
					Socket *pClientsock = new Socket(servsock.Accept(clientaddr));
                    Channel *clientchannel = new Channel(&ep, pClientsock->Fd());
					++totalClientNum, ++activeClientNum;
					printf("accept client fd(%d) [%llu/%llu] %s:%d ok\n", pClientsock->Fd(), activeClientNum, totalClientNum, clientaddr.Ip(), clientaddr.Port());
                    clientchannel->UseET();
                    clientchannel->EnableReading();
				} else {
					std::string sendBuf, recvBuf;
					ssize_t byteN = 1024, recvN = 0;
					while (true) {
						recvBuf.resize(byteN);
						recvN = recv(ch->Fd(), &recvBuf[0], recvBuf.size(), 0);
						if (recvN > 0) {
							printf("receive from fd(%d) [%s]\n", ch->Fd(), recvBuf.c_str());
							sendBuf = std::string(recvBuf, 0, recvN);
							send(ch->Fd(), sendBuf.data(), sendBuf.size(), 0);
						} else if (recvN < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
							break;
						} else if (recvN < 0 && errno == EINTR) {
							continue;
						} else if (recvN == 0) {
							printf("client fd(%d) disconnected\n", ch->Fd());
							close(ch->Fd());
							--activeClientNum;
							break;
						} else {
							perror("recv");
							close(ch->Fd());
							--activeClientNum;
							break;
						}
					}
				}
			} else if (ch->Revents() & EPOLLOUT) {

			} else {
				printf("client fd(%d) error\n", ch->Fd());
				close(ch->Fd());
				--activeClientNum;
			}
		}
	}

	std::cout << "colsed" << std::endl;
	exit(0);
}
