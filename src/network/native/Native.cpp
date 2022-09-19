/*
 * Copyright (C) 2015-2019 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Native.h"

int ::Native::getaddrinfo(const char *__name, const char *__service, const struct addrinfo *__req,
                                 struct addrinfo **__pai) {
    return ::getaddrinfo(__name, __service, __req, __pai);
}

void ::Native::freeaddrinfo(struct addrinfo *__ai) {
    return ::freeaddrinfo(__ai);
}

int ::Native::socket(int __domain, int __type, int __protocol) {
    return ::socket(__domain, __type, __protocol);
}

int ::Native::shutdown(int __fd, int how) {
    return ::shutdown(__fd, how);
}

int ::Native::close(int __fd) {
#if defined(_WIN32)
    return ::closesocket(__fd);
#else
    return ::close(__fd);
#endif
}

int ::Native::getsockopt(int sockfd, int level, int optname, char *optval, socklen_t *optlen) {
    return ::getsockopt(sockfd, level, optname, optval, optlen);
}

int ::Native::connect(int __fd, const sockaddr *__addr, socklen_t __len) {
    return ::connect(__fd, __addr, __len);
}

int ::Native::select(int ndfs, fd_set *_read, fd_set *_write, fd_set *_except, timeval *timeout) {
    return ::select(ndfs, _read, _write, _except, timeout);
}

ssize_t (::Native::recv(int socket, void *buffer, size_t length)) {
    return ::recv(socket, static_cast<char*>(buffer), length, 0);
}

ssize_t (::Native::send(int socket, const void *buffer, size_t length)) {
    return ::send(socket, static_cast<const char*>(buffer), length, 0);
}

