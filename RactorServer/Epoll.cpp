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

void Epoll::UpdateChannel(Channel *ch) {
    epoll_event ev;
	ev.data.ptr = ch;
	ev.events = ch->Events();

    if (ch->InPoll()) {
	    if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->Fd(), &ev) < 0) {
		    perror("epoll_ctl");
		    exit(1);
        }
	} else {
	    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->Fd(), &ev) < 0) {
		    perror("epoll_ctl");
		    exit(1);
        }
    }
    ch->SetInEpoll();
}

std::vector<Channel*> Epoll::Loop(int timeout) {
    std::vector<Channel*> channels;
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
            Channel *ch = reinterpret_cast<Channel*>(events_[i].data.ptr);
            ch->SetReEvents(events_[i].events);
			channels.push_back(ch);
        }
    }
    return channels;
}
