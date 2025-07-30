#include <string.h>
#include "Connection.hpp"

Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientSock) :
    loop_(loop), clientSock_(std::move(clientSock)),
    clientChannel_(loop, clientSock_->Fd()), disconnect_(false) {
    clientChannel_.SetReadCallback(std::bind(&Connection::OnMessage, this));
    clientChannel_.SetWriteCallback(std::bind(&Connection::WriteCallback, this));
    clientChannel_.SetCloseCallback(std::bind(&Connection::CloseCallback, this));
    clientChannel_.SetErrorCallback(std::bind(&Connection::ErrorCallback, this));
    clientChannel_.UseET();
    clientChannel_.EnableReading();

}

Connection::~Connection() {

}


int Connection::Fd() const {
    return clientSock_->Fd();
}

std::string Connection::Ip() const {
    return clientSock_->Ip();
}

uint16_t Connection::Port() const {
    return clientSock_->Port();
}

void Connection::OnMessage() {
    size_t byteN = 1024;
    ssize_t recvN = 0;
    std::string buffer(byteN, 0);
    std::string message;
    uint32_t len;
    while (true) {
        recvN = recv(Fd(), &buffer[0], buffer.size(), 0);
        if (recvN > 0) {
            inputBuffer_.Append(buffer.data(), recvN);
        } else if (recvN < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            while (true) {
                if (inputBuffer_.Size() < sizeof(len)) break;
                memcpy(&len, inputBuffer_.Data(), sizeof(len));
                if (inputBuffer_.Size() < len + sizeof(len)) break;
                message.assign(inputBuffer_.Data()+sizeof(len), len);
                inputBuffer_.Erase(0, len+sizeof(len));
                printf("recv from fd(%d) [%s]\n", Fd(), message.c_str());

                onMessageCallback_(shared_from_this(), message);
            }
            break;
        } else if (recvN < 0 && errno == EINTR) {
            continue;
        } else if (recvN == 0) {
            CloseCallback();
            break;
        } else {
            perror("recv");
            ErrorCallback();
            break;
        }
    }
}

void Connection::WriteCallback() {
    int write = send(Fd(), outputBuffer_.Data(), outputBuffer_.Size(), 0);
    if (write > 0) {
        outputBuffer_.Erase(0, write);
    }
    if (outputBuffer_.Empty()) {
        clientChannel_.DisableWriting();
        sendCompleteCallback_(shared_from_this());
    }
}

void Connection::Send(const char *data, size_t size) {
    if (disconnect_ == true) {
        return;
    }
    outputBuffer_.AppendWithHead(data, size);
    clientChannel_.EnableWriting();
}

void Connection::CloseCallback() {
    disconnect_ = true;
    clientChannel_.Remove();
    closeCallback_(shared_from_this());
}

void Connection::ErrorCallback() {
    disconnect_ = true;
    clientChannel_.Remove();
    errorCallback_(shared_from_this());
}

void Connection::SetCloseCallback(std::function<void(spConnection)> fn) {
    closeCallback_ = fn;
}

void Connection::SetErrorCallback(std::function<void(spConnection)> fn) {
    errorCallback_ = fn;
}

void Connection::SetOnMessageCallback(std::function<void(spConnection, std::string&)> fn) {
    onMessageCallback_ = fn;
}

void Connection::SetSendCompleteCallback(std::function<void(spConnection)> fn) {
    sendCompleteCallback_ = fn;
}
