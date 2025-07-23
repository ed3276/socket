#include "Acceptor.hpp"
#include "Connection.hpp"

Acceptor::Acceptor(EventLoop *loop, const std::string ip, const uint16_t port) :
    loop_(loop) {
    servSock_ = new Socket(CreateNoBlocking());
    InetAddress servaddr(ip, port);
    servSock_->SetIp(ip);
    servSock_->SetPort(port);
    servSock_->SetReuseAddr(true);
    servSock_->SetReusePort(true);
    servSock_->SetTcpNoDelay(true);
    servSock_->SetKeepAlive(true);
    servSock_->Bind(servaddr);
    servSock_->Listen();

    printf("listen on socket fd: %d\n", servSock_->Fd());

    Channel *acceptChannel_ = new Channel(loop_, servSock_->Fd());
    acceptChannel_->SetReadCallback(std::bind(&Acceptor::NewConnection, this));
    acceptChannel_->EnableReading();

}

Acceptor::~Acceptor() {
    delete servSock_;
    delete acceptChannel_;
}

void Acceptor::NewConnection() {
	InetAddress clientaddr;
	Socket *pClientsock = new Socket(servSock_->Accept(clientaddr));
    pClientsock->SetIp(clientaddr.Ip());
    pClientsock->SetPort(clientaddr.Port());
    newConnectionCb_(pClientsock);
}

void Acceptor::SetNewConnectionCb(std::function<void(Socket*)> fn) {
    newConnectionCb_ = fn;
}
