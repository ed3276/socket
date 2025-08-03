#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "EventLoop.hpp"
#include "Connection.hpp"

int CreateTimerFd(int sec = 30) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
    struct itimerspec timerout;
    bzero(&timerout, sizeof(timerout));
    timerout.it_value.tv_sec = sec;
    timerout.it_value.tv_nsec = 0;
    timerfd_settime(tfd, 0, &timerout, 0);
    return tfd;
}

EventLoop::EventLoop(bool mainLoop, int timeInterval, int timeout) :
    timeInterval_(timeInterval),
    timeout_(timeout),
    wakeupFd_(eventfd(0, EFD_NONBLOCK)),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    timerFd_(CreateTimerFd(timeInterval_)),
    timerChannel_(new Channel(this, timerFd_)),
    mainLoop_(mainLoop),
    stop_(false) {

    wakeupChannel_->SetReadCallback(std::bind(&EventLoop::HandleWakeUp, this));
    wakeupChannel_->EnableReading();

    timerChannel_->SetReadCallback(std::bind(&EventLoop::HandleTimer, this));
    timerChannel_->EnableReading();
}

EventLoop::~EventLoop() {

}

void EventLoop::Run() {
//  printf("EventLoop::Run() thread is %ld.\n", syscall(SYS_gettid));
    threadId_ = syscall(SYS_gettid);

    while(stop_ == false) {
        std::vector<Channel*> channels = ep_.Loop();
        if (channels.empty()) {
            epollTimeOutCallback_(this);
        } else {
            for (auto &ch : channels) {
                ch->HandleEvent();
            }
        }
    }
}

void EventLoop::Stop() {
    stop_ = true;
    WakeUp();
}

void EventLoop::UpdateChannel(Channel *ch) {
    ep_.UpdateChannel(ch);
}

void EventLoop::RemoveChannel(Channel *ch) {
    ep_.RemoveChannel(ch);
}

void EventLoop::SetEpollTimeOutCallback_(std::function<void(EventLoop *)> fn) {
    epollTimeOutCallback_ = fn;
}


bool EventLoop::InThreadLoop() const {
    return threadId_ == syscall(SYS_gettid);
}

void EventLoop::QueueInLoop(std::function<void()> fn) {
    {
        std::lock_guard<std::mutex> lk(mtx_);
        task_.push(fn);
    }

    //唤醒事件循环
    WakeUp();
}

void EventLoop::WakeUp() {
    uint64_t val = 1;
    write(wakeupFd_, &val, sizeof(val));
}

void EventLoop::HandleWakeUp() {
    printf("HandleWakeUp() thread id is %ld\n", syscall(SYS_gettid));
    uint64_t val = 1;
    read(wakeupFd_, &val, sizeof(val));

    std::function<void()> fn;
    std::lock_guard<std::mutex> lk(mtx_);
    while (!task_.empty()) {
        fn = std::move(task_.front());
        task_.pop();
        fn();
    }
}

void EventLoop::HandleTimer() {
    struct itimerspec timerout;
    bzero(&timerout, sizeof(timerout));
    timerout.it_value.tv_sec = timeInterval_;
    timerout.it_value.tv_nsec = 0;
    timerfd_settime(timerFd_, 0, &timerout, 0);

    if (mainLoop_) {
        //printf("主事件循环闹钟时间到了\n");
    } else {
        //printf("从事件循环闹钟时间到了\n");
        printf("EventLoop::HandleTimer thread is %d, fd", syscall(SYS_gettid));
        time_t now = time(0);
        for (auto &c : conns_) {
            printf(" %d", c.first);
            if (c.second->Timeout(now, timeout_)) {
                {
                    std::lock_guard<std::mutex> lk(connsMtx);
                    conns_.erase(c.first);
                }
                timerCallback_(c.first);
            }
        }
        printf("\n");
    }
}

void EventLoop::NewConnection(spConnection conn) {
    std::lock_guard<std::mutex> lk(connsMtx);
    conns_[conn->Fd()] = conn;
}

void EventLoop::SetTimerCallback_(std::function<void(int)> fn) {
    timerCallback_ = fn;
}
