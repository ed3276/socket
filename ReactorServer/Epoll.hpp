#pragma once
#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <strings.h>      // bzero()
#include <errno.h>
#include <unistd.h>       // close()
#include <sys/epoll.h>
#include <vector>
#include "Channel.hpp"

class Channel;

class Epoll {
 public:
    Epoll();
    ~Epoll();

    void UpdateChannel(Channel *ch);
    void RemoveChannel(Channel *ch);
    std::vector<Channel*> Loop(int timeout = -1);

 private:
    static const int MaxEvents = 100;
    int epollfd_ = -1;
    epoll_event events_[MaxEvents];
};

