#include "NativeWrapper.h"

int ::NativeWrapper::getaddrinfo(const char *__name, const char *__service, const struct addrinfo *__req,
                                 struct addrinfo **__pai) {
    return ::getaddrinfo(__name, __service, __req, __pai);
}

int ::NativeWrapper::socket(int __domain, int __type, int __protocol) {
    return ::socket(__domain, __type, __protocol);
}

int ::NativeWrapper::connect(int __fd, const sockaddr *__addr, socklen_t __len) {
    return ::connect(__fd, __addr, __len);
}

int ::NativeWrapper::close(int __fd) {
    return ::close(__fd);
}
