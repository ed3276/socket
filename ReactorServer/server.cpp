#include <iostream>
#include <signal.h>
#include "EchoServer.hpp"

extern int h_errno;

//1. 设置2和15的信号
//2. 在信号处理函数中停止主从事件循环的工作线程。
//3. 服务程序主动退出。

EchoServer *echoserver;

void Stop(int sig) {
    printf("sig %d\n", sig);
    delete echoserver;
    printf("echoserver已停止\n");
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./server 192.168.74.130 8279" << std::endl;
        exit(0);
    }

    signal(SIGTERM, Stop);
    signal(SIGINT, Stop);

    echoserver = new EchoServer(argv[1], std::stoi(argv[2]), 3, 3);
    echoserver->Start();

    exit(0);
}
