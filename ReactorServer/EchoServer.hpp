#pragma once
#include "TcpServer.hpp"
#include "EventLoop.hpp"
#include "Connection.hpp"

class EchoServer {
 public:
	EchoServer(const std::string ip, const uint16_t port, size_t threadNum = 3);
	~EchoServer();
	void Start();
    void HandleNewConnection(Connection *clientSock);
	void HandleMessage(Connection *conn, std::string &message);
    void HandleClose(Connection *conn);
    void HandleError(Connection *conn);
    void HandleSendComplete(Connection *conn);
    void HandleTimeOut(EventLoop *loop);

private:
	 TcpServer tcpserver_;
};
