#include <iostream>
#include "TcpServer.hpp"

extern int h_errno;

int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./server 192.168.74.130 8279" << std::endl;
		exit(0);
	}

    TcpServer tcpserver(argv[1], std::stoi(argv[2]));
    tcpserver.Start();

	exit(0);
}
