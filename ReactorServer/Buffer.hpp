#pragma once

#include <string>
#include <iostream>

class Buffer {
 public:
    Buffer(uint16_t sep = 1);
    ~Buffer();

    void Append(const char *data, size_t size);
    void AppendWithSep(const char *data, size_t size);
    size_t Size() const;
    const char *Data() const;
    void Clear();
    bool Empty() const;
    void Erase(size_t pos, size_t len); //从buf的pos开始, 删除n个字节, pos从0开始
    bool PickMessage(std::string &ss);
 private:
    std::string buf_;
    const uint16_t sep_; //报文的分隔符：0-无分隔符; 1-四字节的报头; 2-"\r\n\r\n"分隔符(http协议)
    const std::string token;
};
