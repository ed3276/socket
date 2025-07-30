#include "TcpServer.hpp"

TcpServer::TcpServer(const std::string ip, const uint16_t port, size_t threadNum): threadNum_(threadNum) {
    mainloop_ = new EventLoop;
    mainloop_->SetEpollTimeOutCallback_(std::bind(&TcpServer::EpollTimeOut, this, std::placeholders::_1));

    acceptor_ = new Acceptor(mainloop_, ip, port);
    acceptor_->SetNewConnectionCb(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));

    threadpool_ = new ThreadPool(threadNum_, "IO");

    for (size_t i = 0; i < threadNum_; ++i) {
        subloops_.push_back(new EventLoop);
        subloops_[i]->SetEpollTimeOutCallback_(std::bind(&TcpServer::EpollTimeOut, this, std::placeholders::_1));
        threadpool_->enqueue(std::bind(&EventLoop::Run, subloops_[i]));
    }
}

TcpServer::~TcpServer() {
    delete acceptor_;
    delete mainloop_;

    for (auto loop : subloops_) {
        delete loop;
    }

    delete threadpool_;
}

void TcpServer::Start() {
    mainloop_->Run();
}

void TcpServer::NewConnection(Socket *clientSock) {
    //Connection *conn = new Connection(mainloop_, clientSock);
    //把新建的conn分配给从事件循环
    size_t eloopIdx = clientSock->Fd() % threadNum_;
    spConnection conn(new Connection(subloops_.at(eloopIdx), clientSock));
    conn->SetCloseCallback(std::bind(&TcpServer::CloseConnection, this, std::placeholders::_1));
    conn->SetErrorCallback(std::bind(&TcpServer::ErrorConnection, this, std::placeholders::_1));
    conn->SetOnMessageCallback(std::bind(&TcpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    conn->SetSendCompleteCallback(std::bind(&TcpServer::SendComplete, this, std::placeholders::_1));
    conns_[conn->Fd()] = conn;
    //printf("new connection fd(%d) %s:%d ok\n", conn->Fd(), conn->Ip().c_str(), conn->Port());
    if (newConnectionCb_) newConnectionCb_(conn);
}

void TcpServer::OnMessage(spConnection conn, std::string &message) {
    if (onMessageCb_) onMessageCb_(conn, message);
}

void TcpServer::CloseConnection(spConnection conn) {
    if (closeConnectionCb_) closeConnectionCb_(conn);
    //printf("client fd(%d) disconnected\n", conn->Fd());
    conns_.erase(conn->Fd());
}

void TcpServer::ErrorConnection(spConnection conn) {
    if (errorConnectionCb_) errorConnectionCb_(conn);
    //printf("client fd(%d) error\n", conn->Fd());
    conns_.erase(conn->Fd());
}

void TcpServer::SendComplete(spConnection conn) {
    if (sendCompleteCb_) sendCompleteCb_(conn);
}

void TcpServer::EpollTimeOut(EventLoop *loop) {
    if (timeoutCb_) timeoutCb_(loop);
}

void TcpServer::SetNewConnectionCb(std::function<void(spConnection)> fn) {
    newConnectionCb_ = fn;
}

void TcpServer::SetCloseConnectionCb(std::function<void(spConnection)> fn) {
    closeConnectionCb_ = fn;
}

void TcpServer::SetErrorConnectionCb(std::function<void(spConnection)> fn) {
    errorConnectionCb_ = fn;
}

void TcpServer::SetOnMessageCb(std::function<void(spConnection, std::string&)> fn) {
    onMessageCb_ = fn;
}

void TcpServer::SetSendCompleteCb(std::function<void(spConnection)> fn) {
    sendCompleteCb_ = fn;
}

void TcpServer::SetTimeOutCb(std::function<void(EventLoop*)> fn) {
    timeoutCb_ = fn;
}

