#include "TcpServer.hpp"

TcpServer::TcpServer(const std::string ip, const uint16_t port) {
    acceptor_ = new Acceptor(&loop_, ip, port);
    acceptor_->SetNewConnectionCb(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer() {
    delete acceptor_;
    for (auto &conn : conns_) {
        delete conn.second;
    }
}

void TcpServer::Start() {
    loop_.Run();
}

void TcpServer::NewConnection(Socket *clientSock) {
    Connection *conn = new Connection(&loop_, clientSock);
    conn->SetCloseCallback(std::bind(&TcpServer::CloseConnection, this, std::placeholders::_1));
    conn->SetErrorCallback(std::bind(&TcpServer::ErrorConnection, this, std::placeholders::_1));
    conns_[conn->Fd()] = conn;
	printf("new connection fd(%d) %s:%d ok\n", conn->Fd(), conn->Ip().c_str(), conn->Port());
}


void TcpServer::CloseConnection(Connection *conn) {
	printf("client fd(%d) disconnected\n", conn->Fd());
    conns_.erase(conn->Fd());
    delete conn;
}

void TcpServer::ErrorConnection(Connection *conn) {
	printf("client fd(%d) error\n", conn->Fd());
    conns_.erase(conn->Fd());
    delete conn;
}
