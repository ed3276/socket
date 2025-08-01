#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <memory>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "Epoll.hpp"

class Channel;
class Epoll;

class EventLoop {
 public:
    EventLoop(bool mainLoop = false);
    ~EventLoop();

    void Run();
    void UpdateChannel(Channel *ch);
    void RemoveChannel(Channel *ch);
    void SetEpollTimeOutCallback_(std::function<void(EventLoop *)>);
    bool InThreadLoop() const;
    void QueueInLoop(std::function<void()> fn);

    void WakeUp();
    void HandleWakeUp();
    void HandleTimer();
 private:
    Epoll ep_;   //每个事件循环只有一个Epoll
    std::function<void(EventLoop *)> epollTimeOutCallback_;
    pid_t threadId_; //事件循环所在线程的id
    std::mutex mtx_;
    std::queue<std::function<void()>> task_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    int timerFd_;
    std::unique_ptr<Channel> timerChannel_;

    bool mainLoop_;
};
