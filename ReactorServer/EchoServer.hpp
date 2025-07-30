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
    void HandleNewConnection(spConnection conn);
    void HandleMessage(spConnection conn, std::string &message);
    void HandleClose(spConnection conn);
    void HandleError(spConnection conn);
    void HandleSendComplete(spConnection conn);
    void HandleTimeOut(EventLoop *loop);

    void OnMessage(spConnection conn, std::string message);

private:
     TcpServer tcpserver_;
     ThreadPool threadpool_;
};
