#include "EventLoop.hpp"

EventLoop::EventLoop() {
    ep_ = new Epoll;
}

EventLoop::~EventLoop() {
    if (ep_) delete ep_;
}

void EventLoop::Run() {
	while(true) {
        std::vector<Channel*> channels = ep_->Loop();
		for (auto &ch : channels) {
            ch->HandleEvent();
		}
	}
}

void EventLoop::UpdateChannel(Channel *ch) {
    ep_->UpdateChannel(ch);
}
