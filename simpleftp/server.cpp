#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>     // waitpid()
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <utility>
#include <thread>
#include <functional>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "threadpool.hpp"

extern int h_errno;

class TCPServer;
class Connection : public std::enable_shared_from_this<Connection> {
public:
	explicit Connection(TCPServer &server) :
		srv(server),
		id(-1), fd(-1), port(-1), addr{0}, len(sizeof(addr)) {}

	~Connection() {  }

	void SetHandle(std::function<void()> ClientCallback) {
		handle = std::move(ClientCallback);
	}

	void Process() {
		auto self = shared_from_this();
		if (fd >= 0) {
			handle();
		}
	}

	size_t Read(std::string &buf, size_t maxLen);
	size_t Write(const std::string &msg);
	size_t Read(char *msg, size_t len);
	size_t Write(const char *msg, size_t len);
    bool ReadN(std::string &msg, size_t len);
    bool WriteN(const std::string &msg);
    bool RecvFile(const std::string &file, size_t filetype);
    bool SendFile(const std::string &file);
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
};

size_t Connection::Read(std::string &buf, size_t maxLen) {
	ssize_t recvN = 0;
	buf.resize(maxLen);
	if ((recvN = recv(fd, &buf[0], buf.size(), 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
		} else {
			buf.clear();
		}
		Close();
	} else {
		buf.resize(recvN);
		totalReadByte += recvN;
	}
	return recvN;
}

size_t Connection::Write(const std::string &msg) {
	bool sendN = 0;
	size_t len = msg.length();
	if ((sendN = send(fd, &msg[0], len, 0)) < 0) {
		perror("send");
		Close();
		return false;
	} else {
	    totalWriteByte += len; 
	}
	return sendN;
}

size_t Connection::Read(char *msg, size_t len) {
	ssize_t recvN = 0;

	if ((recvN = recv(fd, msg, len, 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
		}
		Close();
	} else {
		totalReadByte += recvN;
	}
	return recvN;
}

size_t Connection::Write(const char *msg, size_t len) {
	bool sendN = 0;
	if ((sendN = send(fd, &msg[0], len, 0)) < 0) {
		perror("send");
		Close();
		return false;
	} else {
		totalWriteByte += len; 
	}
	return sendN;
}

bool Connection::ReadN(std::string &msg, size_t len) {
	bool res = false;
	size_t block = 128;
	size_t n = 0;
	size_t rlen = 0;
	msg.resize(len);
	while (n < len) {
		block = (n + block) < len ? block : (len - n);
		if ((rlen = recv(fd, &msg[n], block, 0)) <= 0) {
			if (rlen < 0) {
				perror("recv");
			}
			Close();
			break;
		} else {
			n += rlen;
			totalReadByte += rlen;
		}
	}
	res = true;
	return res;
}

bool Connection::WriteN(const std::string &msg) {
	bool res = false;
    size_t len = msg.size();
	size_t block = 128;
	size_t n = 0;
	size_t wlen = 0;
	while(n < len) {
		block = (n + block) < len ? block : (len - n);
		if ((wlen = send(fd, &msg[n], block, 0)) < 0) {
			perror("send");
			Close();
			return false;
		} else {
			n += wlen;
	        totalWriteByte += wlen; 
		}
	}
	res = true;
	return res;
}

std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](int c) {
        return std::isspace(c);
    });
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](int c) {
        return std::isspace(c);
    }).base();
    return (start < end) ? std::string(start, end) : "";
}

std::vector<std::string> ParseWithRegex(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string GetFilename(const std::string& path) {
    // 查找最后一个 '/' 或 '\'
    size_t last_slash = path.find_last_of("/\\");
    return (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);
}

bool Connection::RecvFile(const std::string &file, size_t filesize) {
	size_t block = 128;
	std::string buf;

	std::cout << "receive file info: " << GetFilename(file) << " " << filesize << "字节" << std::endl;

	std::ofstream outfile(GetFilename(file), std::ios::binary);
	if (!outfile) {
		std::cout << "打开文件失败: " << GetFilename(file) << std::endl;
        return false;
	}

	size_t n = 0;
	size_t needByte = 0;
    double inprocess = 0;
	block = 512;
	while ( n < filesize) {
		needByte = (n + block) < filesize ? block : (filesize - n);     
		buf.resize(needByte);
		if (ReadN(buf, needByte) == false) {
			return false;
		}
		outfile.write(buf.data(), buf.size());
        n += needByte;
        inprocess = double(n*100)/filesize;
		std::cout << "receive file: " << file << " " << needByte << "字节 inprocess " << inprocess << "%" << std::endl;
	}
    outfile.close();

	return true;
}

bool Connection::SendFile(const std::string &file) {
	bool res = true;
	std::string buf;
    size_t block = 128;
	struct stat st;
	if (stat(file.c_str(), &st) < 0) {
		perror("stat");
		return res;
	}
	size_t filesize = st.st_size;
	std::ifstream infile(file, std::ios::binary);
	if (!infile) {
		std::cout << "打开文件失败: " << file << std::endl;
        return false;
	} else {
		buf = file + " " + std::to_string(filesize);
	}
	buf.resize(block, ' ');
	if (WriteN(buf) == false) {
		return false;
	}
	std::cout << "send to " << ip << ":" << port;
	std::cout << " [" << trim(buf) << ']' << std::endl;

    size_t n = 0;
    size_t needByte = 0;
    double inprocess = 0;
    block = 512;
	while ( n < filesize) {
        needByte = (n + block) < filesize ? block : (filesize - n);     
        buf.resize(needByte);
        infile.read(&buf[0], needByte);
		if (WriteN(buf) == false) {
			return false;
		}
        n += needByte;
        inprocess = double(n*100)/filesize;
		std::cout << "send file: " << file << " " << needByte << "字节 inprocess " << inprocess << "%" << std::endl;
	}

	if (!ReadN(buf, 128)) {
		return false;
	} else {
		std::cout << "receive from  " << ip << ":" << port;
		std::cout <<" [" << trim(buf) << ']' << std::endl;
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
	ThreadPool pool;
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
	cid(0),
	pool(6)
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
            
			pool.enqueue([=]() { pClient->Process(); });
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
		}
		clients.clear();
		close(listenfd);
		std::cout << "server closed" << std::endl;
	}
}

bool DoFtp(Connection &conn) {
	std::string buf;
	size_t len = 0;
    size_t block = 128;
	if (!conn.ReadN(buf, 128)) {
        return false;
	}
    auto match = ParseWithRegex(std::string(buf.begin(), buf.begin()+128));
    if (match.size() == 0) return false;
    if (match[0] == "upload") {
        std::string file = std::string("/tmp/ftp/") + match[1]; 
        size_t filesize = std::stoi(match[2]); 
        std::string buf = std::string("receive ") + file;
		if(!conn.RecvFile(file, filesize)) {
			buf += " Failed";
		} else {
			buf += " Ok";
		}

        std::cout << trim(buf) << std::endl;
        buf.resize(128, ' ');
		if (!conn.WriteN(buf)) {
            return false;
		}
	} else if (match[0] == "download") {
        std::string file = std::string("/tmp/ftp/") + GetFilename(match[1]);
        conn.SendFile(file);

        std::string buf;
		if (!conn.ReadN(buf, 128)) {
            return false;
		}
        std::cout << trim(buf) << std::endl;
	}

    return true;
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
					if (!DoFtp(conn)) {
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
