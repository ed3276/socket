#include "Channel.hpp"

Channel::Channel(Epoll *ep, int fd) : ep_(ep), fd_(fd) {

}

Channel::~Channel() {

}

int Channel::Fd() const {
    return fd_;
}

void Channel::UseET() {
    events_ = events_ | EPOLLET;
}

void Channel::EnableReading() {
    events_ |= EPOLLIN;
    ep_->UpdateChannel(this);
}

void Channel::SetInEpoll() {
    inEpoll_ = true;
}

void Channel::SetReEvents(uint32_t ev) {
    revents_ = ev;
}

bool Channel::InPoll() const {
    return inEpoll_;
}

uint32_t Channel::Events() const {
    return events_;
}

uint32_t Channel::Revents() const {
    return revents_;
}
