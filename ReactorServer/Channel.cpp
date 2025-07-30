#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Connection.hpp"

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd) {

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
    loop_->UpdateChannel(this);
}

void Channel::DisableReading() {
    events_ &= ~EPOLLIN;
    loop_->UpdateChannel(this);
}

void Channel::EnableWriting() {
    events_ |= EPOLLOUT;
    loop_->UpdateChannel(this);
}

void Channel::DisableWriting() {
    events_ &= ~EPOLLOUT;
    loop_->UpdateChannel(this);
}

void Channel::DisableAll() {
    events_ = 0;
    loop_->UpdateChannel(this);
}

void Channel::Remove() {
    DisableAll();
    loop_->RemoveChannel(this);
}

void Channel::SetInEpoll() {
    inEpoll_ = true;
}

void Channel::SetRevents(uint32_t ev) {
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

void Channel::HandleEvent() {
    if (revents_ & EPOLLRDHUP) {
        closeCallback_();
    } else if (revents_ & (EPOLLIN | EPOLLPRI)) {
        readCallback_();
    } else if (revents_ & EPOLLOUT) {
        writeCallback_();
    } else {
        errorCallback_();
    }
}

void Channel::SetReadCallback(std::function<void()> fn) {
    readCallback_ = fn;
}

void Channel::SetWriteCallback(std::function<void()> fn) {
    writeCallback_ = fn;
}

void Channel::SetCloseCallback(std::function<void()> fn) {
    closeCallback_ = fn;
}

void Channel::SetErrorCallback(std::function<void()> fn) {
    errorCallback_ = fn;
}
