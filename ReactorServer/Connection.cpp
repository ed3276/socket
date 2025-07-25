#include <string.h>
#include "Connection.hpp"

Connection::Connection(EventLoop *loop, Socket *clientSock) :
    loop_(loop), clientSock_(clientSock) {
	clientChannel_ = new Channel(loop, clientSock_->Fd());
    clientChannel_->SetReadCallback(std::bind(&Connection::OnMessage, this));
    clientChannel_->SetCloseCallback(std::bind(&Connection::CloseCallback, this));
    clientChannel_->SetErrorCallback(std::bind(&Connection::ErrorCallback, this));
	clientChannel_->UseET();
	clientChannel_->EnableReading();

}

Connection::~Connection() {
    delete clientSock_; 
    delete clientChannel_;
}


int Connection::Fd() const {
    return clientSock_->Fd();
}

std::string Connection::Ip() const {
    return clientSock_->Ip();
}

uint16_t Connection::Port() const {
    return clientSock_->Port();
}

void Connection::OnMessage() {
	size_t byteN = 1024;
	ssize_t recvN = 0;
	std::string buffer(byteN, 0);
	std::string message;
    int len;
	while (true) {
		recvN = recv(Fd(), &buffer[0], buffer.size(), 0);
		if (recvN > 0) {
			inputBuffer_.Append(buffer.data(), recvN);
		} else if (recvN < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			while (true) {
				if (inputBuffer_.Size() < sizeof(len)) break;
				memcpy(&len, inputBuffer_.Data(), sizeof(len));
				if (inputBuffer_.Size() < len + sizeof(len)) break;
				message.assign(inputBuffer_.Data()+sizeof(len), len);
				inputBuffer_.Erase(0, len+sizeof(len));
                printf("recv from fd(%d) [%s]\n", Fd(), message.c_str());

				message = std::string("reply:") + message;
                onMessageCallback_(this, message);
            }
		} else if (recvN < 0 && errno == EINTR) {
			continue;
		} else if (recvN == 0) {
            CloseCallback();
			break;
		} else {
			perror("recv");
            ErrorCallback();
			break;
		}
	}
}

void Connection::CloseCallback() {
    closeCallback_(this);
}

void Connection::ErrorCallback() {
    errorCallback_(this);
}

void Connection::SetCloseCallback(std::function<void(Connection*)> fn) {
    closeCallback_ = fn;
}

void Connection::SetErrorCallback(std::function<void(Connection*)> fn) {
    errorCallback_ = fn;
}

void Connection::SetOnMessageCallback(std::function<void(Connection*, std::string)> fn) {
    onMessageCallback_ = fn;
}
