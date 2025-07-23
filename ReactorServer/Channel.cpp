#include "Channel.hpp"

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

void Channel::HandleEvent() {
	if (revents_ & EPOLLRDHUP) {
        closeCallback_();
	} else if (revents_ & (EPOLLIN | EPOLLPRI)) {
        readCallback_();
	} else if (revents_ & EPOLLOUT) {

	} else {
        errorCallback_();
	}
}

void Channel::OnMessage() {
	std::string sendBuf, recvBuf;
	ssize_t byteN = 1024, recvN = 0;
	while (true) {
		recvBuf.resize(byteN);
		recvN = recv(fd_, &recvBuf[0], recvBuf.size(), 0);
		if (recvN > 0) {
			printf("receive from fd(%d) [%s]\n", fd_, recvBuf.c_str());
			sendBuf = std::string(recvBuf, 0, recvN);
			send(fd_, sendBuf.data(), sendBuf.size(), 0);
		} else if (recvN < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			break;
		} else if (recvN < 0 && errno == EINTR) {
			continue;
		} else if (recvN == 0) {
            closeCallback_();
			break;
		} else {
			perror("recv");
            errorCallback_();
			break;
		}
	}
}

void Channel::SetReadCallback(std::function<void()> fn) {
    readCallback_ = fn;
}

void Channel::SetCloseCallback(std::function<void()> fn) {
    closeCallback_ = fn;
}

void Channel::SetErrorCallback(std::function<void()> fn) {
    errorCallback_ = fn;
}
