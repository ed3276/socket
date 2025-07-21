#include "Epoll.hpp"

Epoll::Epoll() {
	epollfd_ = epoll_create(1);
    if (epollfd_ < 0) {
        perror("epoll_create");
        exit(-1);
    }
}

Epoll::~Epoll() {
    if (epollfd_ >= 0) {
        close(epollfd_);
    }
}

void Epoll::AddFd(int fd, uint32_t op) {
    epoll_event ev;
	ev.data.fd = fd;
	ev.events = op;

	if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
		perror("epoll_ctl");
		exit(1);
	}
}

std::vector<epoll_event> Epoll::Loop(int timeout) {
    std::vector<epoll_event> evs;
    bzero(events_, sizeof(events_));
    int retval;
	retval = epoll_wait(epollfd_, &events_[0], MaxEvents, timeout);
	if (retval < 0) {
		perror("poll()");
		exit(-1);
	} else if (retval == 0) {
	    printf("epoll timeout...\n");
    } else {
		for (int i = 0; i < retval; ++i) {
			evs.push_back(events_[i]);
        }
    }
    return evs;
}
