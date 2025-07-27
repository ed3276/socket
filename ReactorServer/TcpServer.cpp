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
	newConnectionCb_(conn);
}

void TcpServer::OnMessage(Connection *conn, std::string message) {
	onMessageCb_(conn, message);
}

void TcpServer::CloseConnection(Connection *conn) {
	closeConnectionCb_(conn);
	//printf("client fd(%d) disconnected\n", conn->Fd());
    conns_.erase(conn->Fd());
    delete conn;
}

void TcpServer::ErrorConnection(Connection *conn) {
	errorConnectionCb_(conn);
	//printf("client fd(%d) error\n", conn->Fd());
    conns_.erase(conn->Fd());
    delete conn;
}

void TcpServer::SendComplete(Connection *conn) {
	sendCompleteCb_(conn);
}

void TcpServer::EpollTimeOut(EventLoop *loop) {
	timeoutCb_(loop);
}

void TcpServer::SetNewConnectionCb(std::function<void(Connection*)> fn) {
	if (newConnectionCb_)
	    newConnectionCb_ = fn;
}

void TcpServer::SetCloseConnectionCb(std::function<void(Connection*)> fn) {
	if (closeConnectionCb_)
        closeConnectionCb_ = fn;
}

void TcpServer::SetErrorConnectionCb(std::function<void(Connection*)> fn) {
    if (errorConnectionCb_)
		errorConnectionCb_ = fn;
}

void TcpServer::SetOnMessageCb(std::function<void(Connection*, std::string)> fn) {
	if (onMessageCb_)
        onMessageCb_ = fn;
}

void TcpServer::SetSendCompleteCb(std::function<void(Connection*)> fn) {
	if (sendCompleteCb_)
        sendCompleteCb_ = fn;
}

void TcpServer::SetTimeOutCb(std::function<void(EventLoop*)> fn) {
	if (timeoutCb_)
        timeoutCb_ = fn;
}

