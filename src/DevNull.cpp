#include <libCom/DevNull.h>
#include <libCom/Range.h>
#include <libCom/BufferRange.h>


BufferRangeConst DevNull::append(const void *data, uint32_t len) {
    return Buffer::append(data, len);
}

BufferRangeConst DevNull::append(const char *data, uint32_t len) {
    return Buffer::append(static_cast<const void *>(data), len);
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

BufferRangeConst DevNull::write(const void *data, uint32_t len, uint32_t offset) {
    increase(offset+len);
    return BufferRangeConst(*this, len, offset);
}

BufferRangeConst DevNull::write(const Buffer &other, uint32_t offset) {
    return Buffer::write(other, offset);
}

BufferRangeConst DevNull::write(const BufferRangeConst &other, uint32_t offset) {
    return Buffer::write(other, offset);
}
