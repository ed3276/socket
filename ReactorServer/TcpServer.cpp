#include "TcpServer.hpp"

TcpServer::TcpServer(const std::string ip, const uint16_t port) {
    acceptor_ = new Acceptor(&loop_, ip, port);
    acceptor_->SetNewConnectionCb(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
	loop_.SetEpollTimeOutCallback_(std::bind(&TcpServer::EpollTimeOut, this, std::placeholders::_1));
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
    conn->SetOnMessageCallback(std::bind(&TcpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    conn->SetSendCompleteCallback(std::bind(&TcpServer::SendComplete, this, std::placeholders::_1));
    conns_[conn->Fd()] = conn;
	//printf("new connection fd(%d) %s:%d ok\n", conn->Fd(), conn->Ip().c_str(), conn->Port());
	if (newConnectionCb_) newConnectionCb_(conn);
}

void TcpServer::OnMessage(Connection *conn, std::string message) {
	if (onMessageCb_) onMessageCb_(conn, message);
}

void TcpServer::CloseConnection(Connection *conn) {
	if (closeConnectionCb_) closeConnectionCb_(conn);
	//printf("client fd(%d) disconnected\n", conn->Fd());
    conns_.erase(conn->Fd());
    delete conn;
}

void TcpServer::ErrorConnection(Connection *conn) {
    if (errorConnectionCb_) errorConnectionCb_(conn);
	//printf("client fd(%d) error\n", conn->Fd());
    conns_.erase(conn->Fd());
    delete conn;
}

void TcpServer::SendComplete(Connection *conn) {
	if (sendCompleteCb_) sendCompleteCb_(conn);
}

void TcpServer::EpollTimeOut(EventLoop *loop) {
	if (timeoutCb_) timeoutCb_(loop);
}

void TcpServer::SetNewConnectionCb(std::function<void(Connection*)> fn) {
	newConnectionCb_ = fn;
}

void TcpServer::SetCloseConnectionCb(std::function<void(Connection*)> fn) {
    closeConnectionCb_ = fn;
}

void TcpServer::SetErrorConnectionCb(std::function<void(Connection*)> fn) {
	errorConnectionCb_ = fn;
}

void TcpServer::SetOnMessageCb(std::function<void(Connection*, std::string&)> fn) {
    onMessageCb_ = fn;
}

void TcpServer::SetSendCompleteCb(std::function<void(Connection*)> fn) {
    sendCompleteCb_ = fn;
}

void TcpServer::SetTimeOutCb(std::function<void(EventLoop*)> fn) {
    timeoutCb_ = fn;
}

