#pragma once
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "Epoll.hpp"

class Epoll;

class Channel {
 public:
    Channel(Epoll *ep, int fd, bool isListen);
    ~Channel();
  
    int Fd() const;
    void UseET();
    void EnableReading();
    void SetInEpoll();
    void SetReEvents(uint32_t ev);
    bool InPoll() const;
    uint32_t Events() const;
    uint32_t Revents() const;
    void HandleEvent(Socket *servsock); // 事件处理函数, epoll_wait()返回的时候, 执行它
 private:
    Epoll *ep_ = nullptr; // Channel对应的红黑树, Channel与Epoll是多对一的关系, 一个Channel只对应一个Epoll.
    int fd_ = -1; // Channel 拥有fd, Channel和fd是一对一的关系.
    bool inEpoll_ = false; // Channel是否已添加到epoll树上, 如果未添加, 调用epoll_ctl()的时候EPOLL_CTL_ADD, 否则调用EPOLL_CTL_MOD
    uint32_t events_ = 0;  // fd需要监视的事件, listenfd和clientfd需要监视EPOLLIN, clientfd还可能需要监视EPOLLOUT
    uint32_t revents_ = 0;  // fd已发生的事件
    bool isListen_ = false; // listen取值为true, 客户端连上来的fd取值为false
};
