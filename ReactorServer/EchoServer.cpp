#include "EchoServer.hpp"

EchoServer::EchoServer(const std::string ip, const uint16_t port, size_t subThreadNum, size_t workThreadNum) :
    tcpserver_(ip, port, subThreadNum), threadpool_(workThreadNum, "WORKS") {
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
    printf("EchoServer::HandleNewConnection() thread is %ld.\n", syscall(SYS_gettid));
}

void EchoServer::HandleMessage(Connection *conn, std::string &message) {
    printf("EchoServer::HandleMessage() thread is %ld.\n", syscall(SYS_gettid));
    //message = "reply:" + message;
    //conn->Send(message.data(), message.size());
    threadpool_.enqueue(std::bind(&EchoServer::OnMessage, this, conn, message));
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

void EchoServer::OnMessage(Connection* conn, std::string message) {
    message = "reply:" + message;
    conn->Send(message.data(), message.size());
}
