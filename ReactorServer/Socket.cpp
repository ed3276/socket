#include "Socket.hpp"

int CreateNoBlocking() {
	int listenfd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
	if (listenfd< 0) {
		perror("socket");
        exit(-1);
	}
    return listenfd;
}

Socket::Socket(int fd) : fd_(fd) {

}

Socket::~Socket() {
    close(fd_);
}

int Socket::Fd() const {
    return fd_;
}

void Socket::SetReuseAddr(bool on) {
    int optval = on;
	setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::SetReusePort(bool on) {
    int optval = on;
	setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::SetTcpNoDelay(bool on) {
    int optval = on;
	setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::SetKeepAlive(bool on) {
    int optval = on;
	setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void Socket::Bind(const InetAddress &servaddr) {
	if (bind(fd_, servaddr.Addr(), sizeof(sockaddr)) < 0) {
		perror("bind");
		exit(-1);
	}
    ip_   = std::string(servaddr.Ip());
    port_ = servaddr.Port();
}

void Socket::Listen(int num) {
	if (listen(fd_, num) < 0) {
		perror("listen");
		exit(-1);
	}
}

int Socket::Accept(InetAddress &clientaddr) {
	int clientfd;
	struct sockaddr_in peeraddr;
	socklen_t len = sizeof(peeraddr);
	clientfd = accept4(fd_, (struct sockaddr*)&peeraddr, &len, SOCK_NONBLOCK);
    clientaddr = InetAddress(peeraddr);

    return clientfd;
}

std::string Socket::Ip() const {
    return ip_;
}

uint16_t Socket::Port() const {
    return port_;
}


void Socket::SetIp(const std::string &ip) {
    ip_ = ip;
}

void Socket::SetPort(const uint16_t port) {
    port_ = port;
}
