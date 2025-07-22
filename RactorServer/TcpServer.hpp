#pragma once

#include "EventLoop.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "Acceptor.hpp"

class TcpServer {
 public:
    TcpServer(const std::string ip, const uint16_t port);
    ~TcpServer();

    void Start();
 private:
    EventLoop loop_;  //一个TcpServer可以有多个事件循环
    Acceptor *acceptor_; //一个TcpServer只有一个Acceptor对象
};
