#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/wait.h>     // waitpid()
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <utility>
#include <thread>
#include <functional>
#include <mutex>

extern int h_errno;

class TCPServer;
class Connection : public std::enable_shared_from_this<Connection> {
public:
	explicit Connection(TCPServer &server) :
		srv(server),
		id(-1), fd(-1), port(-1), addr{0}, len(sizeof(addr)) {}
	~Connection() {
		if (th.joinable()) {
			th.join();
		}
	}
	void SetHandle(std::function<void()> ClientCallback) {
		handle = std::move(ClientCallback);
	}
	void Process() {
		auto self = shared_from_this();
		if (fd >= 0) {
			handle();
		}
	}
	bool Read();
	bool Write(const std::string &msg);
	bool Read(char *msg, size_t len);
	bool Write(const char *msg, size_t len);
	void Close();

	int GetFd() const { return fd; }
	TCPServer &GetServer() { return srv; }
	std::string GetMsg() { return recvBuf; }
//private:
	TCPServer &srv;
	std::function<void()> handle;
	size_t id;
	int fd;
	std::string ip;
	int port;
	struct sockaddr_in addr;
	socklen_t len;
	std::string recvBuf;
	size_t totalReadByte;
	size_t totalWriteByte;
    std::thread th;
};

bool Connection::Read() {
	bool res = false;
	ssize_t byteN = 1024, recvN = 0;
	if (fd < 0) return res;
	recvBuf.resize(byteN);
	if ((recvN = recv(fd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
		} else {
			recvBuf.clear();
		}
		Close();
	} else {
		recvBuf.resize(recvN);
		res = true;
		totalReadByte += recvN;
	}
	return res;
}

bool Connection::Write(const std::string &msg) {
	bool res = false;
	if (fd >= 0) {
		if (send(fd, msg.data(), msg.size(), 0) < 0) {
			perror("send");
			Close();
		} else {
			res = true;
			totalWriteByte += msg.size();
		}
	}
	return res;
}


bool Connection::Read(char *msg, size_t len) {
	bool res = false;
	ssize_t byteN = 1024, recvN = 0;
	if (fd < 0) return res;
	if ((recvN = recv(fd, msg, len, 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
		} else {
			len = 0;
		}
		Close();
	} else {
		msg[recvN] = '\0';
		res = true;
		totalReadByte += recvN;
	}
	return res;
}

bool Connection::Write(const char *msg, size_t len) {
	bool res = false;
	if (fd >= 0) {
		if (send(fd, msg, len, 0) < 0) {
			perror("send");
			Close();
		} else {
			res = true;
			totalWriteByte += len;
		}
	}
	return res;
}

class TCPServer {
	friend class Connection;
public:
	TCPServer();
	~TCPServer();
    bool Start(int port, std::function<void(Connection &conn)> acceptCallback);

private:
	std::shared_ptr<Connection> Accept();
	void Stop();
	int ListenFd() const {
		return listenfd;
	}
	size_t GetTotalClientNum() const {
		return totalClientNum;
	}

    size_t GetActiveClientNum() const {
		return activeClientNum;
	}

	int GetServerPort() const {
		return serverPort;
	}


	std::shared_ptr<Connection> GetConnection(int fd) {
		std::shared_ptr<Connection> res;
		if (clients.count(fd)) {
			res = clients[fd];
		}
		return res;
	}

	int listenfd;
	std::unordered_map<int, std::shared_ptr<Connection>> clients;
    struct sockaddr_in serverAddr;
	int serverPort;
	size_t totalReadByte;
	size_t totalWriteByte;
	size_t totalClientNum;
	size_t activeClientNum;
	size_t cid;
	std::function<void(Connection&)> acceptHandle;
	std::mutex mtx;
};

void Connection::Close() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
		{
			std::lock_guard<std::mutex> lk(srv.mtx);
			srv.clients.erase(fd);
			--srv.activeClientNum;
			srv.totalReadByte += totalReadByte;
			srv.totalWriteByte += totalWriteByte;
		}
		std::cout << "client colsed " << ip << ":" << port << std::endl;
	}
}

TCPServer::TCPServer() :
    listenfd(-1),
    serverAddr{0},
	serverPort(-1),
	totalReadByte(0),
	totalWriteByte(0),
	totalClientNum(0),
	activeClientNum(0),
	cid(0)
{

}

TCPServer::~TCPServer() {
	Stop();
}

bool TCPServer::Start(int port, std::function<void(Connection &conn)> acceptCallback) {
    bool res = false;
	serverPort = port;
    acceptHandle = std::move(acceptCallback);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		perror("socket");
		return res;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind");
		return res;
	}

	if (listen(listenfd, 5) < 0) {
		perror("listen");
		return res;
	}

	std::cout << "listen on socket fd: " << ListenFd() << std::endl;

	while(true) {
		auto pClient = Accept();
		if (pClient) {
			std::cout << "connected client[" << GetActiveClientNum();
			std::cout << "/" << GetTotalClientNum() << "] " << pClient->ip << ":" << pClient->port << std::endl;
            
			pClient->th = std::move(std::thread([=]() { pClient->Process(); }));
			//client.Process();
		} 
	}

	std::cout << "colsed" << std::endl;


    res = true;
	return res;
}

std::shared_ptr<Connection> TCPServer::Accept() {
    std::shared_ptr<Connection>	client = std::make_shared<Connection>(*this);
	int clientfd = -1;
	if ((clientfd = accept(listenfd, (struct sockaddr*)&client->addr, &client->len)) < 0) {
		perror("accept");
	} else {
		acceptHandle(*client);
		client->id = cid++;
		client->fd = clientfd;
		client->ip = inet_ntoa(client->addr.sin_addr);
		client->port = ntohs(client->addr.sin_port); 

		clients[clientfd] = client;
	    ++totalClientNum;
		++activeClientNum;
	}
    return client;
}





void TCPServer::Stop() {
	if (listenfd >= 0) {
		for (auto &pClient : clients) {
			auto &c = *pClient.second;
			close(c.fd);
			if (c.th.joinable()) {
				c.th.join();
			}
		}
		clients.clear();
		close(listenfd);
		std::cout << "server closed" << std::endl;
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: ./server 8279" << std::endl;
		exit(0);
	}
	
    TCPServer server;
	int port = std::stoi(argv[1]);
	bool res = server.Start(port, [](Connection &conn) {
			conn.SetHandle([&conn]() {
				while(true) {
					if (conn.Read()) {
						std::cout << "receive from " << conn.ip << ":" << conn.port;
						std::cout << " [" << conn.GetMsg() << ']' << std::endl;
						if (!conn.Write("OK")) {
							break;
						}
					} else {
						break;
					}
				}
			});
		});

	if (res == false) {
		std::cerr << "server start failed" << std::endl;
	}

	exit(0);
}
