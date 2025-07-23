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
    std::string Ip() const;
    uint16_t Port() const;
	void SetReuseAddr(bool on);
	void SetReusePort(bool on);
	void SetTcpNoDelay(bool on);
	void SetKeepAlive(bool on);

    void Bind(const InetAddress &servaddr);
    void Listen(int num = 128);
    int  Accept(InetAddress &clientaddr);
    void SetIp(const std::string&);
    void SetPort(const uint16_t);

 private:
    const int fd_;
    std::string ip_;
    uint16_t port_;
};

