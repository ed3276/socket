#pragma once
#include <memory>
#include <functional>
#include <atomic>
#include "Socket.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Buffer.hpp"

class Socket;
class Channel;
class EventLoop;
class Connection;

using spConnection = std::shared_ptr<Connection>;

class Connection : public std::enable_shared_from_this<Connection> {
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
    void SetCloseCallback(std::function<void(spConnection)>);
    void SetErrorCallback(std::function<void(spConnection)>);
    void SetOnMessageCallback(std::function<void(spConnection, std::string&)>);
    void SetSendCompleteCallback(std::function<void(spConnection)>);
 private:
    EventLoop *loop_; //Connection对应的事件循环, 在构造函数中传入
    Socket *clientSock_;
    Channel *clientChannel_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    std::atomic_bool disconnect_;

    std::function<void(spConnection)> closeCallback_;
    std::function<void(spConnection)> errorCallback_;
    std::function<void(spConnection, std::string&)> onMessageCallback_;
    std::function<void(spConnection)> sendCompleteCallback_;
};
