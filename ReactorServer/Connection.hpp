#pragma once
#include <functional>
#include "Socket.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"

class Socket;
class Channel;
class EventLoop;

class Connection {
 public:
    Connection(EventLoop *loop, Socket *clientSock);
    ~Connection();

    int Fd() const;
    std::string Ip() const;
    uint16_t Port() const;

    void OnMessage();
    void Send(const char *data, size_t size);
    void WriteCallback();
    void CloseCallback();
    void ErrorCallback();
    void SetCloseCallback(std::function<void(Connection*)>);
    void SetErrorCallback(std::function<void(Connection*)>);
    void SetOnMessageCallback(std::function<void(Connection*, std::string)>);
    void SetSendCompleteCallback(std::function<void(Connection*)>);
 private:
    EventLoop *loop_; //Connection对应的事件循环, 在构造函数中传入
    Socket *clientSock_;
    Channel *clientChannel_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    std::function<void(Connection*)> closeCallback_;
    std::function<void(Connection*)> errorCallback_;
    std::function<void(Connection*, std::string)> onMessageCallback_;
    std::function<void(Connection*)> sendCompleteCallback_;
};
