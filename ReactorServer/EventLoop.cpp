#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "EventLoop.hpp"

EventLoop::EventLoop() : wakeupFd_(eventfd(0, EFD_NONBLOCK)),
    wakeupChannel_(new Channel(this, wakeupFd_)) {
    wakeupChannel_->SetReadCallback(std::bind(&EventLoop::HandleWakeUp, this));
    wakeupChannel_->EnableReading();
}

EventLoop::~EventLoop() {

}

void EventLoop::Run() {
//  printf("EventLoop::Run() thread is %ld.\n", syscall(SYS_gettid));
    threadId_ = syscall(SYS_gettid);

    while(true) {
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
