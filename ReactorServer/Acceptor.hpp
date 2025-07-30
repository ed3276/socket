#pragma once
#include <functional>
#include <memory>
#include "Socket.hpp"
#include "InetAddress.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"

class Acceptor {
 public:
    Acceptor(EventLoop *loop, const std::string ip, const uint16_t port);
    ~Acceptor();

    void NewConnection();
    void SetNewConnectionCb(std::function<void(std::unique_ptr<Socket>)>);
 private:
    EventLoop *loop_; //Acceptor对应的事件循环, 在构造函数中传入
    Socket servSock_;
    Channel acceptChannel_;
    std::function<void(std::unique_ptr<Socket>)> newConnectionCb_;
};
