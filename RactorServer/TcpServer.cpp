#include "TcpServer.hpp"

TcpServer::TcpServer(const std::string ip, const uint16_t port) {
    acceptor_ = new Acceptor(&loop_, ip, port);
}

TcpServer::~TcpServer() {
    delete acceptor_;
}

void TcpServer::Start() {
    loop_.Run();
}


