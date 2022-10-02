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

#include <unordered_map>

#if defined(__WIN32)
#undef WINVER
#define WINVER 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <network/PushConnection.h>
#include <secure_memory/String.h>
#include "custom_assert.h"
#include "ConnectionTest.h"

// private include
#include "../src/network/native/Native.h"
#include "../../src/network/Resolve.h"
#include "../../src/network/socket/SSLSocket.h"
#include "../../src/network/socket/NotifySocket.h"

// mock the socket functions to emulate network behavior
inline const char *currentTestName() {
    return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

#define MAKE_MOCK_FUNCTION(fun, ret, ...) std::function<ret(__VA_ARGS__)> fun = [] (__VA_ARGS__)

// defaults
struct NativeMock {
    MAKE_MOCK_FUNCTION(getaddrinfo, int, const char*, const char*, const addrinfo*, addrinfo**) { return 0; };
    MAKE_MOCK_FUNCTION(socket, int, int, int, int) { return 0; };
    MAKE_MOCK_FUNCTION(connect, int, int, const sockaddr*, socklen_t) { return 0; };
    MAKE_MOCK_FUNCTION(shutdown, int, int, int) { return 0; };
    MAKE_MOCK_FUNCTION(close, int, int) { return 0; };
    MAKE_MOCK_FUNCTION(recv, int64_t, int, void*, size_t) { return 0; };
    MAKE_MOCK_FUNCTION(send, int64_t, int, const void*, size_t) { return 0; };
    MAKE_MOCK_FUNCTION(freeaddrinfo, void, addrinfo*) { return 0; };
    MAKE_MOCK_FUNCTION(getsockopt, int, int, int, int, char*, socklen_t*) { return 0; };
    MAKE_MOCK_FUNCTION(select, int, int, fd_set*, fd_set*, fd_set*, timeval*) { return 0; };
};

static std::unordered_map<std::string, NativeMock> mocks;

int ::Native::getaddrinfo(const char *__name, const char *__service, const struct addrinfo *__req,
                                 struct addrinfo **__pai) {
    return mocks[currentTestName()].getaddrinfo(__name, __service, __req, __pai);
}

int ::Native::socket(int __domain, int __type, int __protocol) {
    return mocks[currentTestName()].socket(__domain, __type, __protocol);
}

int ::Native::connect(int __fd, const sockaddr *__addr, socklen_t __len) {
    return mocks[currentTestName()].connect(__fd, __addr, __len);
}

int ::Native::shutdown(int __fd, int how) {
    return mocks[currentTestName()].shutdown(__fd, how);
}

int ::Native::close(int __fd) {
    return mocks[currentTestName()].close(__fd);
}

int64_t (::Native::recv(int __fd, void *buffer, size_t length)) {
    return mocks[currentTestName()].recv(__fd, buffer, length);
}

int64_t (::Native::send(int __fd, const void *buffer, size_t length)) {
    return mocks[currentTestName()].send(__fd, buffer, length);
}

void ::Native::freeaddrinfo(struct addrinfo *__ai) {
    return mocks[currentTestName()].freeaddrinfo(__ai);
}

int ::Native::getsockopt(int sockfd, int level, int optname, char *optval, socklen_t *optlen) {
    return mocks[currentTestName()].getsockopt(sockfd, level, optname, optval, optlen);
}

int ::Native::select(int ndfs, fd_set *_read, fd_set *_write, fd_set *_except, timeval *timeout) {
    return mocks[currentTestName()].select(ndfs, _read, _write, _except, timeout);
}

TEST_F(ConnectionTest, noHost) {
    // host does not exist
    mocks[currentTestName()].getaddrinfo = [] (const char *, const char *, const addrinfo *, addrinfo **) {
        return EAI_NONAME;
    };

    Connection conn("localhost", 1337, false);
    ASSERT_THROW(conn.connect(), resolve_error);
    ASSERT_FALSE(conn.connected());
}

TEST_F(ConnectionTest, hostButNoAddresses) {
    // host can be resolved, but there are no addresses associated. This shouldn't happen under normal operation but
    // testing it just in case
    mocks[currentTestName()].getaddrinfo = [] (const char *, const char *, const addrinfo *, addrinfo **outAddr) {
        *outAddr = nullptr;
        return 0;
    };

    Connection conn("localhost", 1337, false);
    ASSERT_THROW(conn.connect(), resolve_error);
    ASSERT_FALSE(conn.connected());
}

TEST_F(ConnectionTest, invalidSocket) {
    mocks[currentTestName()].getaddrinfo = [] (const char *, const char *, const addrinfo *, addrinfo **outAddr) {
        // define addrinfo used for resolve mock
        struct addrinfo *addr = new addrinfo;
        memset(addr, 0, sizeof(addrinfo));

        addr->ai_family = AF_INET;
        addr->ai_socktype = SOCK_STREAM;
        addr->ai_protocol = IPPROTO_TCP;
        struct sockaddr *saddr = new sockaddr;
        memcpy(saddr->sa_data, "01234567890123", 14);
        saddr->sa_family = AF_INET;
        addr->ai_addr = saddr;

        *outAddr = addr;
        return 0;
    };
    mocks[currentTestName()].socket = [] (int, int, int) {
        // invalid socket
        return -1;
    };
    mocks[currentTestName()].connect = [] (int __fd, const sockaddr *, socklen_t) {
        EXPECT_NE(-1, __fd) << "Missing check for invalid socket identifier!";
        return -1;
    };
    mocks[currentTestName()].freeaddrinfo = [] (struct addrinfo *__ai) {
        delete __ai->ai_addr;
        delete __ai;
    };

    Connection conn("localhost", 1337, false);
    ASSERT_THROW(conn.connect(), socket_error);
    ASSERT_FALSE(conn.connected());
}

TEST_F(ConnectionTest, successConnect1stAddressIPv4) {
    mocks[currentTestName()].getaddrinfo = [] (const char *, const char *, const addrinfo *, addrinfo **outAddr) {
        // define addrinfo used for resolve mock
        struct addrinfo *addr = new addrinfo;
        memset(addr, 0, sizeof(addrinfo));

        addr->ai_family = AF_INET;
        addr->ai_socktype = SOCK_STREAM;
        addr->ai_protocol = IPPROTO_TCP;
        struct sockaddr *saddr = new sockaddr;
        memcpy(saddr->sa_data, "01234567890123", 14);
        saddr->sa_family = AF_INET;
        addr->ai_addr = saddr;
        // --

        *outAddr = addr;
        return 0;
    };
    mocks[currentTestName()].socket = [] (int, int, int) {
        return 42;      // just pick some socket identifier number, is used in tests only
    };
    mocks[currentTestName()].connect = [] (int __fd, const sockaddr *__addr, socklen_t) {
        EXPECT_EQ(42, __fd) << "Internal socket descriptor should not change";
        // check that internal values stay same
        EXPECT_EQ(AF_INET, __addr->sa_family);
        EXPECT_ARRAY_EQ(const char, "01234567890123", __addr->sa_data, 14);

        // emulate connect -> return -1 and set errno / WSAError
#ifdef WIN32
        WSASetLastError(WSAEWOULDBLOCK);
#else
        errno = EINPROGRESS;
#endif
        return -1;
    };
    mocks[currentTestName()].select = [] (int , fd_set *, fd_set *, fd_set *, timeval *) {
        return 1;
    };
    mocks[currentTestName()].getsockopt = [] (int , int , int , char *optval, socklen_t *) {
        int *ptr = (int*)optval;
        *ptr = 0;
        return 0;
    };
    mocks[currentTestName()].freeaddrinfo = [] (struct addrinfo *__ai) {
        delete __ai->ai_addr;
        delete __ai;
    };

    Connection conn("localhost", 1337, false);
    ASSERT_NO_THROW(conn.connect());
    ASSERT_TRUE(conn.connected());
    ASSERT_EQ(IPProtocol::IPv4, conn.protocol());
}

TEST_F(ConnectionTest, successConnect2ndAddressIPv4) {
    mocks[currentTestName()].getaddrinfo = [] (const char *, const char *, const struct addrinfo *, struct addrinfo ** outAddr) {
        // define addrinfo used for resolve mock
        // 1st address is invalid, connection it cannot be established
        struct addrinfo *addr = new addrinfo;
        memset(addr, 0, sizeof(addrinfo));

        addr->ai_family = AF_INET;
        addr->ai_socktype = SOCK_STREAM;
        addr->ai_protocol = IPPROTO_TCP;
        struct sockaddr *saddr1 = new sockaddr;
        memcpy(saddr1->sa_data, "00000000000000", 14);
        saddr1->sa_family = AF_INET;
        addr->ai_addr = saddr1;

        // 2nd address (this one is valid)
        struct addrinfo *addr2 = new addrinfo;
        memset(addr2, 0, sizeof(addrinfo));

        addr2->ai_family = AF_INET;
        addr2->ai_socktype = SOCK_STREAM;
        addr2->ai_protocol = IPPROTO_TCP;
        struct sockaddr *saddr2 = new sockaddr;
        memcpy(saddr2->sa_data, "01234567890123", 14);
        saddr2->sa_family = AF_INET;
        addr2->ai_addr = saddr2;
        // --

        // chain them
        addr->ai_next = addr2;

        *outAddr = addr;
        return 0;
    };
    mocks[currentTestName()].socket = [] (int , int , int ) {
        return 42;      // just pick some socket identifier number, is used in tests only
    };
    static int i;
    i = 0;
    mocks[currentTestName()].connect = [] (int __fd, const sockaddr *__addr, socklen_t ) {
        EXPECT_EQ(42, __fd) << "Internal socket descriptor should not change!";
        // check that internal values stay same
        EXPECT_EQ(AF_INET, __addr->sa_family);

        if (i == 0) {                // 1st address
            EXPECT_ARRAY_EQ(const char, "00000000000000", __addr->sa_data, 14);
            i++;

            // emulate connect -> return -1 and set errno / WSAError
#ifdef WIN32
            WSASetLastError(WSAEWOULDBLOCK);
#else
            errno = ECONNREFUSED;
#endif
            return -1;      // connect is unsuccessful
        } else if (i == 1) {         // 2nd address
            EXPECT_ARRAY_EQ(const char, "01234567890123", __addr->sa_data, 14);
            i++;

            // emulate connect -> return -1 and set errno / WSAError
#ifdef WIN32
            WSASetLastError(WSAECONNREFUSED);
#else
            errno = EINPROGRESS;
#endif
            return -1;
        }
        ADD_FAILURE() << "Should not be reached!";
        return -1;
    };
    mocks[currentTestName()].select = [] (int , fd_set *, fd_set *, fd_set *, timeval *) {
        return 1;
    };
    mocks[currentTestName()].getsockopt = [] (int , int , int , char *optval, socklen_t *) {
        int *ptr = (int*)optval;
        *ptr = 0;
        return 0;
    };
    static bool checkClose = true;
    mocks[currentTestName()].close = [] (int __fd) {
        EXPECT_EQ(42, __fd) << "Internal socket descriptor should not change!";
        if (checkClose) {
            EXPECT_EQ(1, i) << "Only close failed sockets!";
            return 1;
        }
        return 0;
    };
    mocks[currentTestName()].freeaddrinfo = [] (struct addrinfo *__ai) {
        delete __ai->ai_next->ai_addr;
        delete __ai->ai_next;
        delete __ai->ai_addr;
        delete __ai;
    };

    Connection conn("localhost", 1337, false);
    ASSERT_NO_THROW(conn.connect());
    // do not execute the assert in close if connect was already successful because a successful socket will be closed
    // there too
    checkClose = false;

    ASSERT_TRUE(conn.connected());
    ASSERT_EQ(IPProtocol::IPv4, conn.protocol());
}

void mockReal() {
    mocks[currentTestName()].getaddrinfo = &::getaddrinfo;
    mocks[currentTestName()].socket = &::socket;
    mocks[currentTestName()].connect = &::connect;
    mocks[currentTestName()].recv = [] (int _fd, void*b, size_t l) { return ::recv(_fd, static_cast<char*>(b), l, 0); };
    mocks[currentTestName()].send = [] (int _fd, const void*b, size_t l) { return ::send(_fd, static_cast<const char*>(b), l, 0); };
    mocks[currentTestName()].freeaddrinfo = &::freeaddrinfo;
    mocks[currentTestName()].select = &::select;
    mocks[currentTestName()].getsockopt = &::getsockopt;
    mocks[currentTestName()].shutdown = &::shutdown;

#ifdef __WIN32
    mocks[currentTestName()].close = &::closesocket;
#else
    mocks[currentTestName()].close = &::close;
#endif
}

TEST_F(ConnectionTest, realSSL) {
    // switch to real native calls
    mockReal();

    // tries to establish a connection to viaduck servers
    Connection conn("viaduck.org", 443);
    ASSERT_NO_THROW(conn.connect());
    ASSERT_TRUE(conn.connected());
    ASSERT_TRUE(conn.info().ssl());
}

TEST_F(ConnectionTest, realNoSSL) {
    // switch to real native calls
    mockReal();

    // tries to establish a connection to viaduck servers
    Connection conn("viaduck.org", 80, false);
    ASSERT_NO_THROW(conn.connect());
    ASSERT_TRUE(conn.connected());
    ASSERT_FALSE(conn.info().ssl());
}

TEST_F(ConnectionTest, sessionResumption) {
    // switch to real native calls
    mockReal();

    // tries to establish a connection to viaduck servers
    Connection conn("viaduck.org", 443);
    ASSERT_NO_THROW(conn.connect());
    ASSERT_TRUE(conn.connected());
    ASSERT_TRUE(conn.info().ssl());
    // this saves SSL session in TLS 1.3
    conn.disconnect();

    // following connections should use stored ssl sessions
    Connection conn2("viaduck.org", 443);
    ASSERT_NO_THROW(conn2.connect());
    ASSERT_TRUE(conn2.connected());
    ASSERT_TRUE(conn2.info().ssl());
    ASSERT_TRUE(((SSLSocket*)conn2.socket())->isReused());
    conn2.disconnect();

    ASSERT_NO_THROW(conn2.connect());
    ASSERT_TRUE(conn2.connected());
    ASSERT_TRUE(conn2.info().ssl());
    ASSERT_TRUE(((SSLSocket*)conn2.socket())->isReused());
}

TEST_F(ConnectionTest, notify) {
    // switch to real native calls
    mockReal();

    NotifySocket notify(ConnectionInfo("", 0));
    ASSERT_TRUE(notify.connect(nullptr));
    ASSERT_NO_THROW(notify.notify());
    ASSERT_NO_THROW(notify.clear());
}

TEST_F(ConnectionTest, pushConnection) {
    // switch to real native calls
    mockReal();

    PushConnection conn(ConnectionInfo("viaduck.org", 443));
    ASSERT_NO_THROW(conn.connect());
    ASSERT_TRUE(conn.connected());
    ASSERT_TRUE(conn.info().ssl());

    // send one notify
    conn.notify();
    // should not wait since we have a notify
    bool readable, notify;
    ASSERT_TRUE(conn.wait(readable, notify));
    ASSERT_TRUE(notify);
    ASSERT_NO_THROW(conn.clearNotify());
}