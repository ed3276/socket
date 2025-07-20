#pragma once

#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <unistd.h>       // close()
#include <netinet/tcp.h>
#include <string>
#include "InetAddress.hpp"

int CreateNoBlocking();

class Socket {
 public:
    Socket(int fd);
    ~Socket();
    int Fd() const;
	void SetReuseAddr(bool on);
	void SetReusePort(bool on);
	void SetTcpNoDelay(bool on);
	void SetKeepAlive(bool on);

    void Bind(const InetAddress &servaddr);
    void Listen(int num = 128);
    int  Accept(InetAddress &clientaddr);

 private:
    const int fd_;
};

