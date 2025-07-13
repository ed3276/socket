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

extern int h_errno;

class TCPServer {
public:
	TCPServer();
	~TCPServer();
    bool Start(int port);
	std::pair<pid_t, int> Accept();
	bool Read(int fd, std::string &msg);
	bool Write(int fd, const std::string &msg);
	bool Read(int fd, char *msg, size_t len);
	bool Write(int fd, const char *msg, size_t len);
	void Close(int fd);
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

	struct ClientInfo {
		ClientInfo() : pid(-1), id(-1), fd(-1), port(-1), addr{0}, len(sizeof(addr)) {}
		pid_t pid;
		size_t id;
		int fd;
		std::string ip;
		int port;
		struct sockaddr_in addr;
		socklen_t len;
	};

	std::unique_ptr<ClientInfo> GetClientInfo(int fd) {
		std::unique_ptr<ClientInfo> res;
		if (clients.count(fd)) {
			res = std::make_unique<ClientInfo>(clients[fd]);
		}
		return res;
	}

private:
	int listenfd;
	std::unordered_map<int, ClientInfo> clients;
    struct sockaddr_in serverAddr;
	int serverPort;
	size_t totalReadByte;
	size_t totalWriteByte;
	size_t totalClientNum;
	size_t activeClientNum;
	size_t cid;
};

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

bool TCPServer::Start(int port) {
    bool res = false;
	serverPort = port;

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
    res = true;
	return res;
}

std::pair<int, int> TCPServer::Accept() {
	int clientfd = -1;
	int pid = -1;
	ClientInfo client;
	if ((clientfd = accept(listenfd, (struct sockaddr*)&client.addr, &client.len)) < 0) {
		perror("accept");
	} else {
		client.id = cid++;
		client.fd = clientfd;
		client.ip = inet_ntoa(client.addr.sin_addr);
		client.port = ntohs(client.addr.sin_port); 

		clients[clientfd] = client;
	    ++totalClientNum;
		++activeClientNum;
		if ((pid = fork()) < 0) {
			perror("fork");
            Close(clientfd);
		} else if (pid == 0){
			close(listenfd);
		} else {
			client.pid = pid;
			close(clientfd);
		}
	}
	std::pair<int, int> res = std::make_pair(pid, clientfd);
    return res;
}



bool TCPServer::Read(int fd, std::string &msg) {
	bool res = false;
	ssize_t byteN = 1024, recvN = 0;
	if (fd < 0) return res;
	msg.resize(byteN);
	if ((recvN = recv(fd, &msg[0], msg.size(), 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
		} else {
			msg.clear();
		}
		Close(fd);
	} else {
		msg.resize(recvN);
		res = true;
		totalReadByte += recvN;
	}
	return res;
}

bool TCPServer::Write(int fd, const std::string &msg) {
	bool res = false;
	if (fd >= 0) {
		if (send(fd, msg.data(), msg.size(), 0) < 0) {
			perror("send");
			Close(fd);
		} else {
			res = true;
			totalWriteByte += msg.size();
		}
	}
	return res;
}


bool TCPServer::Read(int fd, char *msg, size_t len) {
	bool res = false;
	ssize_t byteN = 1024, recvN = 0;
	if (fd < 0) return res;
	if ((recvN = recv(fd, msg, len, 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
		} else {
			len = 0;
		}
		Close(fd);
	} else {
		msg[recvN] = '\0';
		res = true;
		totalReadByte += recvN;
	}
	return res;
}

bool TCPServer::Write(int fd, const char *msg, size_t len) {
	bool res = false;
	if (fd >= 0) {
		if (send(fd, msg, len, 0) < 0) {
			perror("send");
			Close(fd);
		} else {
			res = true;
			totalWriteByte += len;
		}
	}
	return res;

}

void TCPServer::Close(int fd) {
	if (fd >= 0) {
		close(fd);
		auto &client = clients[fd];
		std::cout << "client colsed " << client.ip;
		std::cout << ":" << client.port << std::endl;
		clients.erase(fd);
		--activeClientNum;
	}
}

void TCPServer::Stop() {
	if (listenfd >= 0) {
		for (auto &c : clients) {
			close(c.second.fd);
			waitpid(c.second.pid, NULL, 0);
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
	if (server.Start(std::stoi(argv[1])) == false) {
		std::cerr << "server start failed" << std::endl;
		exit(0);
	}
	std::cout << "listen on socket fd: " << server.ListenFd() << std::endl;

	int clientfd;
	std::string sendBuf, recvBuf;
	
	while(true) {
		auto client = server.Accept();
        pid_t pid = client.first;
		if ((client.first == 0)) {
            int clientfd = client.second;
			auto pClient = server.GetClientInfo(clientfd);
			auto &client = *pClient;
			std::cout << "connected client[" << server.GetActiveClientNum();
			std::cout << "/" << server.GetTotalClientNum() << "] " << client.ip << ":" << client.port << std::endl;

			while(true) {
				if (server.Read(clientfd, recvBuf)) {
					std::cout << "receive from " << client.ip << ":" << client.port;
					std::cout << " [" << recvBuf << ']' << std::endl;
					sendBuf = std::string("OK");
					if (!server.Write(clientfd, sendBuf.data(), sendBuf.size())) {
						break;
					}
				} else {
					break;
				}
			}
			exit(0);
		} else if (client.first > 0) {
			std::cout << "Child pid:" << pid << ", ppid:" << std::this_thread::get_id() << ", handle client " << client.second << std::endl;
		}
	}

	std::cout << "colsed" << std::endl;
	exit(0);
}
