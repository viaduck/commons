#include <libCom/DevNull.h>
#include <libCom/BufferRange.h>


BufferRange DevNull::append(const void *data, uint32_t len) {
    return append(static_cast<const char *>(data), len);
}

BufferRange DevNull::append(const char *data, uint32_t len) {
    increase(len);
    return BufferRange(*this, len, 0);
}

BufferRange DevNull::append(const Buffer &other) {
    return Buffer::append(other);
}

BufferRange DevNull::append(const BufferRange &range) {
    return Buffer::append(range);
}

void DevNull::consume(uint32_t n) {

}

void DevNull::use(uint32_t used) {

}
