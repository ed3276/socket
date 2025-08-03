#include <string.h>
#include "Buffer.hpp"

Buffer::Buffer(uint16_t sep) : sep_(sep), token("\r\n\r\n") {  }
Buffer::~Buffer() {  }

void Buffer::Append(const char *data, size_t size) {
    buf_.append(data, size);
}

void Buffer::AppendWithSep(const char *data, size_t size) {
    if (sep_ == 0) {
        buf_.append(data, size);
    } else if (sep_ == 1) {
        uint32_t len = size;
        buf_.append((char*)&len, sizeof(len));
        buf_.append(data, size);
    } else {
        buf_.append(data, size);
        buf_.append(token.data(), token.size());
    }
}

size_t Buffer::Size() const {
    return buf_.size();
}

const char *Buffer::Data() const {
    return buf_.data();
}

void Buffer::Clear() {
    buf_.clear();
}

bool Buffer::Empty() const {
    return buf_.empty();
}

void Buffer::Erase(size_t pos, size_t len) {
    buf_.erase(pos, len);
}

bool Buffer::PickMessage(std::string &ss) {
    if (buf_.empty()) {
        return false;
    }
    if (sep_ == 0) {
        ss = buf_;
        buf_.clear();
    } else if (sep_ == 1) {
        uint32_t len;
        if (buf_.size() < sizeof(len)) return false;
        memcpy(&len, buf_.data(), sizeof(len));
        if (buf_.size() < len + sizeof(len)) return false;
        ss.assign(buf_.data() + sizeof(len), len);
        buf_.erase(0, len + sizeof(len));
    } else {
        std::size_t found = buf_.find(token);
        if (found != std::string::npos) {
            return false;
        } else {
            ss.assign(buf_.data(), found);
            buf_.erase(0, found + token.size());
        }
    }
    return true;
}
