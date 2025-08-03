#pragma once
#include <memory>
#include <map>
#include <mutex>
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Connection.hpp"
#include "Acceptor.hpp"
#include "ThreadPool.hpp"

class TcpServer {
 public:
    TcpServer(const std::string ip, const uint16_t port, size_t threadNum = 3);
    ~TcpServer();

    void Start();
    void Stop();
    void NewConnection(std::unique_ptr<Socket> clientSock);
    void OnMessage(spConnection conn, std::string &message);
    void CloseConnection(spConnection conn);
    void ErrorConnection(spConnection conn);
    void SendComplete(spConnection conn);
    void EpollTimeOut(EventLoop *loop);

    void SetNewConnectionCb(std::function<void(spConnection)>);
    void SetCloseConnectionCb(std::function<void(spConnection)>);
    void SetErrorConnectionCb(std::function<void(spConnection)>);
    void SetOnMessageCb(std::function<void(spConnection, std::string&)>);
    void SetSendCompleteCb(std::function<void(spConnection)>);
    void SetTimeOutCb(std::function<void(EventLoop*)>);
 private:
    std::unique_ptr<EventLoop> mainloop_;  //主事件循环
    std::vector<std::unique_ptr<EventLoop>> subloops_; //存放从事件循环
    Acceptor acceptor_; //一个TcpServer只有一个Acceptor对象
    size_t threadNum_;    //线程池大小, 即从事件循环的个数
    ThreadPool threadpool_;
    std::mutex connsMtx_;
    std::map<int, spConnection> conns_;
    std::function<void(spConnection)> newConnectionCb_;
    std::function<void(spConnection)> closeConnectionCb_;
    std::function<void(spConnection)> errorConnectionCb_;
    std::function<void(spConnection, std::string&)> onMessageCb_;
    std::function<void(spConnection)> sendCompleteCb_;
    std::function<void(EventLoop*)> timeoutCb_;

    void RemoveConnection(int fd);
};
