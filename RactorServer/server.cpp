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
#include "EventLoop.hpp"

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

    EventLoop loop;
    Channel *servchannel = new Channel(&loop, servsock.Fd());
    servchannel->SetReadCallback(std::bind(&Channel::NewConnection, servchannel, &servsock));
    servchannel->EnableReading();

    loop.Run();

	exit(0);
}
