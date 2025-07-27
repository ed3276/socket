#include "Buffer.hpp"

Buffer::Buffer() {  }
Buffer::~Buffer() {  }

void Buffer::Append(const char *data, size_t size) {
    buf_.append(data, size);
}

void Buffer::AppendWithHead(const char *data, size_t size) {
    uint32_t len = size;
    buf_.append((char*)&len, sizeof(len));
    buf_.append(data, size);
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
