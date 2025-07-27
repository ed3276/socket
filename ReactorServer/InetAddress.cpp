#include "InetAddress.hpp"

InetAddress::InetAddress(const std::string &ip, uint16_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

InetAddress::~InetAddress() {}

const char *InetAddress::Ip() const {
    return inet_ntoa(addr_.sin_addr);
}

uint16_t InetAddress::Port() const {
    return ntohs(addr_.sin_port);
}

const sockaddr *InetAddress::Addr() const {
    return reinterpret_cast<const sockaddr*>(&addr_);
}
