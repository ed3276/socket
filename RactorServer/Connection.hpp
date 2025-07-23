#pragma once
#include <functional>
#include "Socket.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"

class Socket;
class Channel;
class EventLoop;

class Connection {
 public:
    Connection(EventLoop *loop, Socket *clientSock);
    ~Connection();
 private:
    EventLoop *loop_; //Connection对应的事件循环, 在构造函数中传入
    Socket *clientSock_;
    Channel *clientChannel_;
};
