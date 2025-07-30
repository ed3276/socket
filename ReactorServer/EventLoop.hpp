#pragma once
#include <functional>
#include "Epoll.hpp"

class Channel;
class Epoll;

class EventLoop {
 public:
    EventLoop();
    ~EventLoop();

    void Run();
    void UpdateChannel(Channel *ch);
    void RemoveChannel(Channel *ch);
    void SetEpollTimeOutCallback_(std::function<void(EventLoop *)>);
 private:
    Epoll *ep_;   //每个事件循环只有一个Epoll
    std::function<void(EventLoop *)> epollTimeOutCallback_;
};
