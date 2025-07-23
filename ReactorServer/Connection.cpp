#include "Connection.hpp"

Connection::Connection(EventLoop *loop, Socket *clientSock) :
    loop_(loop), clientSock_(clientSock) {
	clientChannel_ = new Channel(loop, clientSock_->Fd());
    clientChannel_->SetReadCallback(std::bind(&Channel::OnMessage, clientChannel_));
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
