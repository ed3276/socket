#include "Acceptor.hpp"

Acceptor::Acceptor(EventLoop *loop, const std::string ip, const uint16_t port) :
    loop_(loop) {
    servSock_ = new Socket(CreateNoBlocking());
    InetAddress servaddr(ip, port);
    servSock_->SetReuseAddr(true);
    servSock_->SetReusePort(true);
    servSock_->SetTcpNoDelay(true);
    servSock_->SetKeepAlive(true);
    servSock_->Bind(servaddr);
    servSock_->Listen();

    printf("listen on socket fd: %d\n", servSock_->Fd());

    Channel *acceptChannel_ = new Channel(loop_, servSock_->Fd());
    acceptChannel_->SetReadCallback(std::bind(&Channel::NewConnection, acceptChannel_, servSock_));
    acceptChannel_->EnableReading();

}

Acceptor::~Acceptor() {
    delete servSock_;
    delete acceptChannel_;
}
