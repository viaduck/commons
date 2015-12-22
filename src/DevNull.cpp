#include <libCom/DevNull.h>
#include <libCom/Range.h>
#include <libCom/BufferRange.h>


BufferRangeConst DevNull::append(const void *data, uint32_t len) {
    return append(static_cast<const char *>(data), len);
}

BufferRangeConst DevNull::append(const char *data, uint32_t len) {
    increase(len);
    return BufferRangeConst(*this, len, 0);
}

BufferRangeConst DevNull::append(const Buffer &other) {
    return Buffer::append(other);
}

BufferRangeConst DevNull::append(const BufferRangeConst &range) {
    return Buffer::append(range);
}

void DevNull::consume(uint32_t n) {

}

void DevNull::use(uint32_t used) {

}
