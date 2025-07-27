#include "EventLoop.hpp"

EventLoop::EventLoop() {
    ep_ = new Epoll;
}

EventLoop::~EventLoop() {
    if (ep_) delete ep_;
}

void EventLoop::Run() {
	while(true) {
		std::vector<Channel*> channels = ep_->Loop(10*1000);
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

void EventLoop::SetEpollTimeOutCallback_(std::function<void(EventLoop *)> fn) {
	epollTimeOutCallback_ = fn;
}
