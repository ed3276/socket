#pragma once
#include <functional>
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "EventLoop.hpp"
#include "Connection.hpp"

class EventLoop;
class Connection;

class Channel {
 public:
    Channel(EventLoop *loop, int fd);
    ~Channel();
  
    int Fd() const;
    void UseET();
    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWriting();
    void SetInEpoll();
    void SetRevents(uint32_t ev);
    bool InPoll() const;
    uint32_t Events() const;
    uint32_t Revents() const;
    void HandleEvent(); // 事件处理函数, epoll_wait()返回的时候, 执行它

    void SetReadCallback(std::function<void()>);
    void SetWriteCallback(std::function<void()>);
    void SetCloseCallback(std::function<void()>);
    void SetErrorCallback(std::function<void()>);
 private:
    EventLoop *loop_ = nullptr; // Channel对应的红黑树, Channel与EventLoop是多对一的关系, 一个Channel只对应一个EventLoop.
    int fd_ = -1; // Channel 拥有fd, Channel和fd是一对一的关系.
    bool inEpoll_ = false; // Channel是否已添加到epoll树上, 如果未添加, 调用epoll_ctl()的时候EPOLL_CTL_ADD, 否则调用EPOLL_CTL_MOD
    uint32_t events_ = 0;  // fd需要监视的事件, listenfd和clientfd需要监视EPOLLIN, clientfd还可能需要监视EPOLLOUT
    uint32_t revents_ = 0;  // fd已发生的事件
    std::function<void()> readCallback_;
    std::function<void()> writeCallback_;
    std::function<void()> closeCallback_;
    std::function<void()> errorCallback_;
};
