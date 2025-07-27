#include <iostream>
#include "EchoServer.hpp"

extern int h_errno;

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./server 192.168.74.130 8279" << std::endl;
        exit(0);
    }

    EchoServer echoserver(argv[1], std::stoi(argv[2]));
    echoserver.Start();

    exit(0);
}
