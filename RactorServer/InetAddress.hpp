#pragma once

#include <sys/socket.h>   // socket()
#include <netinet/ip.h>   // struct sockaddr_in
#include <arpa/inet.h>    // htons()
#include <string>

class InetAddress {
 public:
    InetAddress() {}
    InetAddress(const std::string &ip, uint16_t port);
    InetAddress(const sockaddr_in &addr) : addr_(addr) {}
    ~InetAddress();
    const char *Ip() const;
    uint16_t Port() const;
    const sockaddr *Addr() const;
 private:
    sockaddr_in addr_;
};
