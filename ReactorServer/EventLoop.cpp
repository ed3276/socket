#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "EventLoop.hpp"

EventLoop::EventLoop() {
    ep_ = new Epoll;
}

EventLoop::~EventLoop() {
    if (ep_) delete ep_;
}

void EventLoop::Run() {
//  printf("EventLoop::Run() thread is %ld.\n", syscall(SYS_gettid));
    while(true) {
        std::vector<Channel*> channels = ep_->Loop();
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
    ep_->UpdateChannel(ch);
}

void EventLoop::RemoveChannel(Channel *ch) {
    ep_->RemoveChannel(ch);
}

void EventLoop::SetEpollTimeOutCallback_(std::function<void(EventLoop *)> fn) {
    epollTimeOutCallback_ = fn;
}
