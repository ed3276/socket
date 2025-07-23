#include "Connection.hpp"

Connection::Connection(EventLoop *loop, Socket *clientSock) :
    loop_(loop), clientSock_(clientSock) {
	clientChannel_ = new Channel(loop, clientSock_->Fd());
    clientChannel_->SetReadCallback(std::bind(&Channel::OnMessage, clientChannel_));
	clientChannel_->UseET();
	clientChannel_->EnableReading();

}

Connection::~Connection() {
    delete clientSock_; 
    delete clientChannel_;
}
