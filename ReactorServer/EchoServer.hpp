#pragma once
#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "Connection.hpp"
#include "ThreadPool.hpp"

class EchoServer {
 public:
    EchoServer(
        const std::string ip,
        const uint16_t port,
        size_t subThreadNum = 3,
        size_t workThreadNum = 5);
    ~EchoServer();
    void Start();
    void HandleNewConnection(Connection *clientSock);
    void HandleMessage(Connection *conn, std::string &message);
    void HandleClose(Connection *conn);
    void HandleError(Connection *conn);
    void HandleSendComplete(Connection *conn);
    void HandleTimeOut(EventLoop *loop);

    void OnMessage(Connection* conn, std::string message);

private:
     TcpServer tcpserver_;
     ThreadPool threadpool_;
};
