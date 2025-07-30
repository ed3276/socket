#include "Acceptor.hpp"
#include "Connection.hpp"

Acceptor::Acceptor(EventLoop *loop, const std::string ip, const uint16_t port) :
    loop_(loop), servSock_(CreateNoBlocking()),
    acceptChannel_(loop_, servSock_.Fd()) {
    InetAddress servaddr(ip, port);
    servSock_.SetIp(ip);
    servSock_.SetPort(port);
    servSock_.SetReuseAddr(true);
    servSock_.SetReusePort(true);
    servSock_.SetTcpNoDelay(true);
    servSock_.SetKeepAlive(true);
    servSock_.Bind(servaddr);
    servSock_.Listen();

    printf("listen on socket fd: %d\n", servSock_.Fd());

    acceptChannel_.SetReadCallback(std::bind(&Acceptor::NewConnection, this));
    acceptChannel_.EnableReading();

}

Acceptor::~Acceptor() {

}

void Acceptor::NewConnection() {
    InetAddress clientaddr;
    std::unique_ptr<Socket> pClientsock(new Socket(servSock_.Accept(clientaddr)));
    pClientsock->SetIp(clientaddr.Ip());
    pClientsock->SetPort(clientaddr.Port());
    newConnectionCb_(std::move(pClientsock));
}

void Acceptor::SetNewConnectionCb(std::function<void(std::unique_ptr<Socket>)> fn) {
    newConnectionCb_ = fn;
}
