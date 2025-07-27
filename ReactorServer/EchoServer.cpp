#include "EchoServer.hpp"

EchoServer::EchoServer(const std::string ip, const uint16_t port) :
    tcpserver_(ip, port) {
	tcpserver_.SetNewConnectionCb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
	tcpserver_.SetCloseConnectionCb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
	tcpserver_.SetErrorConnectionCb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
	tcpserver_.SetOnMessageCb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	tcpserver_.SetSendCompleteCb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
	tcpserver_.SetTimeOutCb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
}

EchoServer::~EchoServer() {

}

void EchoServer::Start() {
    tcpserver_.Start();
}

void EchoServer::HandleNewConnection(Connection *conn) {
    printf("New Connection Come in.\n");
}

void EchoServer::HandleMessage(Connection *conn, std::string message) {
	std::string tmpBuf;
	int len;
	message = std::string("reply:") + message;

	len = message.size();
	tmpBuf = std::string((char*)&len, sizeof(len));
	tmpBuf.append(message);
	conn->Send(tmpBuf.data(), tmpBuf.size());
}

void EchoServer::HandleClose(Connection *conn) {
    printf("EchoServer conn close.\n");
}

void EchoServer::HandleError(Connection *conn) {
    printf("EchoServer conn error.\n");
}

void EchoServer::HandleSendComplete(Connection *conn) {
    printf("Message send complete.\n");
}

void EchoServer::HandleTimeOut(EventLoop *loop) {
    printf("EchoServer timeout.\n");
}

