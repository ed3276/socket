#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

extern int h_errno;

class TCPClient {
public:
	TCPClient();
	~TCPClient();
    bool Connect(const std::string ip, int port);
	bool Read();
	bool Write(const std::string &msg);
	bool Read(char *msg, size_t len);
	bool Write(const char *msg, size_t len);
	void Close();

	std::string GetServerIp() const {
		return serverIp;
	}

	int GetServerPort() const {
		return serverPort;
	}

	std::string GetMsg() const {
		return recvBuf;
	}
private:
	int fd;
	std::string serverIp;
	int serverPort;
    struct sockaddr_in serverAddr;
	std::string localIp;
	int localPort;
    struct sockaddr_in localAddr;
	size_t totalReadByte;
	size_t totalWriteByte;
    std::string recvBuf;
};

TCPClient::TCPClient() :
    fd(-1),
    serverPort(-1),
    serverAddr{0},
	localPort(-1),
	localAddr{0},
	totalReadByte(0),
	totalWriteByte(0)
{

}

TCPClient::~TCPClient() {
	Close();
}

bool TCPClient::Connect(const std::string ip, int port) {
    bool res = false;
	serverIp = ip;
	serverPort = port;
    fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
	} else {
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(serverPort);
		struct hostent *host = gethostbyname(serverIp.c_str());
		if (!host) {
			std::cerr << "gethostbyname error for host" << serverIp << hstrerror(h_errno) << std::endl;
		} else {
			serverAddr.sin_addr = *(struct in_addr*)(host->h_addr);

			if (connect(fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
				perror("connect");
			} else {
				res = true;
			}
		}
	}
	return res;
}

bool TCPClient::Read() {
	bool res = false;
	ssize_t byteN = 1024, recvN = 0;
	if (fd < 0) return res;
	recvBuf.resize(byteN);
	if ((recvN = recv(fd, &recvBuf[0], recvBuf.size(), 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
			Close();
		} else {
			recvBuf.clear();
			Close();
		}
	} else {
		recvBuf.resize(recvN);
		res = true;
		totalReadByte += recvN;
	}
	return res;
}

bool TCPClient::Write(const std::string &msg) {
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


bool TCPClient::Read(char *msg, size_t len) {
	bool res = false;
	ssize_t byteN = 1024, recvN = 0;
	if (fd < 0) return res;
	if ((recvN = recv(fd, msg, len, 0)) <= 0) {
		if (recvN < 0) {
			perror("recv");
			Close();
		} else {
			len = 0;
			Close();
		}
	} else {
		msg[recvN] = '\0';
		res = true;
		totalReadByte += recvN;
	}
	return res;
}
bool TCPClient::Write(const char *msg, size_t len) {
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

void TCPClient::Close() {
	if (fd >= 0) {
		close(fd);
		fd = -1;
		std::cout << "closed" << std::endl;
	}
}


int main(int argc, char **argv) {
	if (argc < 3) {
		std::cerr << "Usage: ./client 192.168.74.142 8279" << std::endl;
		exit(0);
	}
	TCPClient client;
	if (client.Connect(argv[1], std::stoi(argv[2])) == false) {
		exit(0);
	}

    std::cout << "connect to server " << client.GetServerIp() << ":" << client.GetServerPort() << std::endl;
    std::string sendBuf, recvBuf;
	for (int i = 1; i <= 10; ++i) {
		sendBuf = std::string("这是第") + std::to_string(i) + "个消息";
		if (client.Write(sendBuf) == false) {
		    exit(0);
		}
		std::cout << "send to " << client.GetServerIp() << ":" << client.GetServerPort();
		std::cout << " [" << sendBuf << ']' << std::endl;

		if (!client.Read()) {
			if (client.GetMsg().size() == 0) {
			    std::cout << "server " << client.GetServerIp() << ":" << client.GetServerPort() << "closed" << std::endl;
			}
			exit(0);
		} else {
			std::cout << "receive from  " << client.GetServerIp() << ":" << client.GetServerPort();
			std::cout <<" [" << client.GetMsg() << ']' << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}

    return 0;
}
