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
		printf("client fd(%d) disconnected\n", fd_);
		close(fd_);
	} else if (revents_ & (EPOLLIN | EPOLLPRI)) {
        readCallback_();
	} else if (revents_ & EPOLLOUT) {

	} else {
		printf("client fd(%d) error\n", fd_);
		close(fd_);
	}
}


void Channel::NewConnection(Socket *servsock) {
	InetAddress clientaddr;
	Socket *pClientsock = new Socket(servsock->Accept(clientaddr));
    Connection *conn = new Connection(loop_, pClientsock);
	printf("accept client fd(%d) %s:%d ok\n", pClientsock->Fd(), clientaddr.Ip(), clientaddr.Port());
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
			printf("client fd(%d) disconnected\n", fd_);
			close(fd_);
			break;
		} else {
			perror("recv");
			close(fd_);
			break;
		}
	}
}


void Channel::SetReadCallback(std::function<void()> fn) {
    readCallback_ = fn;
}
