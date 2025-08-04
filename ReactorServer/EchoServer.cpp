#include "EchoServer.hpp"

EchoServer::EchoServer(const std::string ip, const uint16_t port, size_t subThreadNum, size_t workThreadNum) :
    tcpserver_(ip, port, subThreadNum), threadpool_(workThreadNum, "WORKS") {
    tcpserver_.SetNewConnectionCb(std::bind(&EchoServer::HandleNewConnection, this, std::placeholders::_1));
    tcpserver_.SetOnMessageCb(std::bind(&EchoServer::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.SetSendCompleteCb(std::bind(&EchoServer::HandleSendComplete, this, std::placeholders::_1));
    tcpserver_.SetTimeOutCb(std::bind(&EchoServer::HandleTimeOut, this, std::placeholders::_1));
    tcpserver_.SetCloseConnectionCb(std::bind(&EchoServer::HandleClose, this, std::placeholders::_1));
    tcpserver_.SetErrorConnectionCb(std::bind(&EchoServer::HandleError, this, std::placeholders::_1));
}

EchoServer::~EchoServer() {
    Stop();
}

void EchoServer::Start() {
    tcpserver_.Start();
}

void EchoServer::Stop() {
    threadpool_.Stop();
    printf("工作线程池已停止\n");
    tcpserver_.Stop();
}

void EchoServer::HandleNewConnection(spConnection conn) {
    //printf("EchoServer::HandleNewConnection() thread is %ld.\n", syscall(SYS_gettid));
    printf("%s new conn(fd %d %s:%d) ok.\n", TimeStamp::Now().ToString().c_str(), conn->Fd(), conn->Ip().c_str(), conn->Port());
}

void EchoServer::HandleMessage(spConnection conn, std::string &message) {
    //printf("EchoServer::HandleMessage() thread is %ld.\n", syscall(SYS_gettid));
    if(threadpool_.size() == 0) {
        OnMessage(conn, message);
    } else {
        threadpool_.enqueue(std::bind(&EchoServer::OnMessage, this, conn, message));
    }
}

void EchoServer::HandleClose(spConnection conn) {
    printf("%s conn closed(fd %d %s:%d) ok.\n", TimeStamp::Now().ToString().c_str(), conn->Fd(), conn->Ip().c_str(), conn->Port());
    //printf("EchoServer conn close.\n");
}

void EchoServer::HandleError(spConnection conn) {
    printf("EchoServer conn error.\n");
}

void EchoServer::HandleSendComplete(spConnection conn) {
    //printf("Message send complete.\n");
}

void EchoServer::HandleTimeOut(EventLoop *loop) {
    printf("EchoServer timeout.\n");
}

void EchoServer::OnMessage(spConnection conn, std::string &message) {
    //printf("%s message fd(%d) [%s]\n", TimeStamp::Now().ToString().c_str(), conn->Fd(), message.c_str());
    message = "reply:" + message;
    conn->Send(message.data(), message.size());
}
