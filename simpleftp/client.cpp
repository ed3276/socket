#include <stdio.h>        // perror()
#include <stdlib.h>       // exit()
#include <unistd.h>       // close()
#include <sys/types.h>
#include <sys/socket.h>   // socket()
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>   // struct sockaddr_in
#include <netdb.h>
#include <arpa/inet.h>    // htons()
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>
#include <map>
#include <utility>
#include <algorithm>
#include <cctype>

extern int h_errno;

class TCPClient {
public:
	TCPClient();
	~TCPClient();
    bool Connect(const std::string ip, int port);
	size_t Read(std::string &buf, size_t maxLen);
	size_t Write(const std::string &msg);
	size_t Read(char *msg, size_t len);
	size_t Write(const char *msg, size_t len);
    bool ReadN(std::string &msg, size_t len);
    bool WriteN(const std::string &msg);
    bool RecvFile(const std::string &file);
    bool SendFile(const std::string &file);
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
    size_t headerSize;
    size_t bufferSize;
};

TCPClient::TCPClient() :
    fd(-1),
    serverPort(-1),
    serverAddr{0},
	localPort(-1),
	localAddr{0},
	totalReadByte(0),
	totalWriteByte(0),
    headerSize(128),
    bufferSize(4096)
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

size_t TCPClient::Read(std::string &buf, size_t maxLen) {
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

size_t TCPClient::Write(const std::string &msg) {
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

size_t TCPClient::Read(char *msg, size_t len) {
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

size_t TCPClient::Write(const char *msg, size_t len) {
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

bool TCPClient::ReadN(std::string &msg, size_t len) {
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

bool TCPClient::WriteN(const std::string &msg) {
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

bool TCPClient::RecvFile(const std::string &file) {
	bool res = false;
	size_t block = 128;
	std::string buf;
	buf = std::string("download ") + file;
	buf.resize(block, ' ');
	if (WriteN(buf) == false) {
		return res;
	}
	std::cout << "send to " << GetServerIp() << ":" << GetServerPort();
	std::cout << " [" << trim(buf) << ']' << std::endl;

	if (ReadN(buf, block) == false) {
		return false;
	}
	std::cout << "receive [" << trim(buf) << "]" << std::endl;
	std::vector<std::string> vs = ParseWithRegex(buf);
	if (vs.size() == 0) {
		std::cout << "receive null" << std::endl;
		return false;
	}
	size_t filesize = std::stoi(vs.at(1));
	std::cout << "receive file info: " << vs.at(0) << " " << filesize << "字节" << std::endl;

	std::ofstream outfile(GetFilename(file), std::ios::binary);
	if (!outfile) {
		std::cout << "打开文件失败: " << GetFilename(file) << std::endl;
		buf = std::string("receive ") + file + " Failed";
		buf.resize(128, ' ');
		WriteN(buf);
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

	buf = std::string("receive ") + file + " OK";
	buf.resize(128, ' ');
	if (!WriteN(buf)) {
		return false;
	}
	std::cout << "send to " << GetServerIp() << ":" << GetServerPort();
	std::cout << " [" << trim(buf) << ']' << std::endl;
    
    res = true;
	return res;
}

bool TCPClient::SendFile(const std::string &file) {
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
	    buf = std::string("upload ") + file + " " + std::to_string(filesize);
		buf.resize(block, ' ');
		if (WriteN(buf) == false) {
			return false;
		}
	}

	std::cout << "send to " << GetServerIp() << ":" << GetServerPort();
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
		std::cout << "sen file: " << file << " " << needByte << "字节 inprocess " << inprocess << "%" << std::endl;
	}

	if (!ReadN(buf, 128)) {
		return false;
	} else {
		std::cout << "receive from  " << GetServerIp() << ":" << GetServerPort();
		std::cout <<" [" << trim(buf) << ']' << std::endl;
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
    std::cout << "Usage:                               " << std::endl;
    std::cout << "upload 123.txt                " << std::endl;
    std::cout << "download /123.txt             " << std::endl;
    std::cout << "ls path                       " << std::endl;
    std::cout << "rm file                       " << std::endl;
    std::cout << "quit                          " << std::endl;

	std::map<std::string, std::function<bool(std::string)>> command;
	command["quit"] = [&](std::string file) -> bool {
        return false;
    };
	command["upload"] = [&](std::string file) -> bool {
       return client.SendFile(file); 
	};

	command["download"] = [&](std::string file) -> bool {
        return client.RecvFile(file);
	};

    std::string sendBuf;
    std::string line;
    std::cout << "ftp$ ";
	while(getline(std::cin, line)) {
        auto match = ParseWithRegex(line);
		if (match.size() == 0) {
            std::cout << "ftp$ ";
            continue;
		}
		if (command.find(match[0]) == command.end()) {
            std::cout << "ftp$ ";
            continue;
		}
		
		if (match.size() == 1) {
			match.push_back("");
		}
        if (command.at(match[0])(match[1]) == false) {
            break;
        }
        std::cout << "ftp$ ";
	}

    return 0;
}
