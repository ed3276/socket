#pragma once

#include "Epoll.hpp"

class EventLoop {
 public:
    EventLoop();
    ~EventLoop();

    void Run();
    Epoll *Ep() const { return ep_; }
 private:
    Epoll *ep_;   //每个事件循环只有一个Epoll
};
