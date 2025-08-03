#pragma once
#include <functional>
#include <queue>
#include <mutex>
#include <memory>
#include <map>
#include <atomic>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "Epoll.hpp"

class Connection;
class Channel;
class Epoll;

using spConnection = std::shared_ptr<Connection>;

class EventLoop {
 public:
    EventLoop(bool mainLoop = false, int timeInterval = 30, int timeout = 80);
    ~EventLoop();

    void Run();
    void Stop();
    void UpdateChannel(Channel *ch);
    void RemoveChannel(Channel *ch);
    void SetEpollTimeOutCallback_(std::function<void(EventLoop *)>);
    bool InThreadLoop() const;
    void QueueInLoop(std::function<void()> fn);

    void WakeUp();
    void HandleWakeUp();
    void HandleTimer();

    void NewConnection(spConnection);
    void SetTimerCallback_(std::function<void(int)>);
 private:
    Epoll ep_;   //每个事件循环只有一个Epoll
    std::function<void(EventLoop *)> epollTimeOutCallback_;
    pid_t threadId_; //事件循环所在线程的id
    std::mutex mtx_;
    std::queue<std::function<void()>> task_;

    int timeInterval_;
    int timeout_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    int timerFd_;
    std::unique_ptr<Channel> timerChannel_;

    bool mainLoop_;
    std::mutex connsMtx;
    std::map<int, spConnection> conns_;
    std::function<void(int)> timerCallback_;

    std::atomic_bool stop_;
};
