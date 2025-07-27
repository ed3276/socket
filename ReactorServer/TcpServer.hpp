#pragma once

#include <map>
#include "EventLoop.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "Acceptor.hpp"

class TcpServer {
 public:
    TcpServer(const std::string ip, const uint16_t port);
    ~TcpServer();

    void Start();
    void NewConnection(Socket *clientSock);
	void OnMessage(Connection *conn, std::string message);
    void CloseConnection(Connection *conn);
    void ErrorConnection(Connection *conn);
    void SendComplete(Connection *conn);
    void EpollTimeOut(EventLoop *loop);

	void SetNewConnectionCb(std::function<void(Connection*)>);
	void SetCloseConnectionCb(std::function<void(Connection*)>);
	void SetErrorConnectionCb(std::function<void(Connection*)>);
	void SetOnMessageCb(std::function<void(Connection*, std::string)>);
	void SetSendCompleteCb(std::function<void(Connection*)>);
	void SetTimeOutCb(std::function<void(EventLoop*)>);
 private:
    EventLoop loop_;  //一个TcpServer可以有多个事件循环
    Acceptor *acceptor_; //一个TcpServer只有一个Acceptor对象
    std::map<int, Connection*> conns_;
	std::function<void(Connection*)> newConnectionCb_;
	std::function<void(Connection*)> closeConnectionCb_;
	std::function<void(Connection*)> errorConnectionCb_;
	std::function<void(Connection*, std::string)> onMessageCb_;
	std::function<void(Connection*)> sendCompleteCb_;
	std::function<void(EventLoop*)> timeoutCb_;
};
