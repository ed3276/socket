#pragma once

#include <string>
#include <iostream>

class Buffer {
 public:
    Buffer();
    ~Buffer();

    void Append(const char *data, size_t size);
	void AppendWithHead(const char *data, size_t size);
    size_t Size() const;
    const char *Data() const;
    void Clear();
    bool Empty() const;
	void Erase(size_t pos, size_t len); //从buf的pos开始, 删除n个字节, pos从0开始
 private:
    std::string buf_;
};
