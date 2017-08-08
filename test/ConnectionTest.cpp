#include <unordered_map>

#if defined(__WIN32)
#undef WINVER
#define WINVER 0x0600
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <libCom/network/Connection.h>
#include <libCom/String.h>
#include <libCom/network/SSLContext.h>
#include "custom_assert.h"
#include "ConnectionTest.h"

// private include
#include "../src/libCom/network/NativeWrapper.h"

/**
 * One-Time initialization for winsock (taken from Native-Wrapper)
 */
class OneTimeInit {
public:
    OneTimeInit() {
#ifdef __WIN32
        WSADATA w;
        if (WSAStartup(MAKEWORD(2,2), &w) != 0) {
            // TODO proper fatal shutdown
            return;
        }
#endif
    }

    ~OneTimeInit() {
#ifdef __WIN32
        WSACleanup();
#endif
    }
};
OneTimeInit go;

// let's mock the socket functions to emulate network behavior
inline const char *currentTestName() {
    return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

static std::unordered_map<std::string, std::unordered_map<std::string, void*>> mocks;

#define callMockFunction(Function,...) (*(decltype(&Function)) (mocks[currentTestName()][#Function]))(__VA_ARGS__)

int ::NativeWrapper::getaddrinfo(const char *__name, const char *__service, const struct addrinfo *__req,
                                 struct addrinfo **__pai) {
    return callMockFunction(getaddrinfo, __name, __service, __req, __pai);
}

int ::NativeWrapper::socket(int __domain, int __type, int __protocol) {
    return callMockFunction(socket, __domain, __type, __protocol);
}

int ::NativeWrapper::connect(int __fd, const sockaddr *__addr, socklen_t __len) {
    return callMockFunction(connect, __fd, __addr, __len);
}

int ::NativeWrapper::close(int __fd) {
    return callMockFunction(close, __fd);
}

ssize_t (::NativeWrapper::recv(int /*socket*/, void */*buffer*/, size_t /*length*/)) {
    return 0;           // unused for now
}

ssize_t (::NativeWrapper::send(int /*socket*/, const void */*buffer*/, size_t /*length*/)) {
    return 0;           // unused for now
}

void ::NativeWrapper::freeaddrinfo(struct addrinfo *__ai) {
    return callMockFunction(freeaddrinfo, __ai);
}

TEST_F(ConnectionTest, noHost) {
    mocks[currentTestName()]["getaddrinfo"] =   (void*)+([] (const char *__restrict, const char *__restrict,
                                                             const struct addrinfo *__restrict,
                                                             struct addrinfo **__restrict) {
                                                    return EAI_NONAME;
                                                });

    mocks[currentTestName()]["close"] =
            (void*)+([] (int ) {
                // noop
            });

    mocks[currentTestName()]["freeaddrinfo"] = (void*)+([] (struct addrinfo *) {
                // noop
            });

    Connection conn("localhost", 1337, false);
    ASSERT_EQ(Connection::ConnectResult::ERROR_RESOLVE, conn.connect());
    ASSERT_EQ(Connection::Status::UNKNOWN, conn.status());
}

TEST_F(ConnectionTest, hostButNoAddresses) {
    // host can be resolved, but there are no addresses associated. This shouldn't happen under normal operation but
    // testing it just in case
    mocks[currentTestName()]["getaddrinfo"] =   (void*)+([] (const char *__restrict, const char *__restrict,
                                                             const struct addrinfo *__restrict,
                                                             struct addrinfo **__restrict outAddr) {
                                                    *outAddr = nullptr;
                                                    return 0;
                                                });

    mocks[currentTestName()]["close"] =
            (void*)+([] (int ) {
                // noop
            });

    mocks[currentTestName()]["freeaddrinfo"] = (void*)+([] (struct addrinfo *) {
                // noop
            });

    Connection conn("localhost", 1337, false);
    ASSERT_EQ(Connection::ConnectResult::ERROR_CONNECT, conn.connect());
    ASSERT_EQ(Connection::Status::UNKNOWN, conn.status());
}

TEST_F(ConnectionTest, dnsCollision) {
    mocks[currentTestName()]["getaddrinfo"] =   (void*)+([] (const char *__restrict, const char *__restrict,
                                                             const struct addrinfo *__restrict,
                                                             struct addrinfo **__restrict outAddr) {
                // define addrinfo used for resolve mock
                struct addrinfo *addr = new addrinfo;
                memset(addr, 0, sizeof(addrinfo));

                addr->ai_family = AF_INET;
                addr->ai_socktype = SOCK_STREAM;
                addr->ai_protocol = IPPROTO_TCP;
                struct sockaddr_in *saddr = new sockaddr_in;
                ::inet_pton(AF_INET, "127.0.53.53", &(saddr->sin_addr));
                addr->ai_addr = (sockaddr*)saddr;
                // --

                *outAddr = addr;
                return 0;
            });

    mocks[currentTestName()]["close"] =
            (void*)+([] (int ) {
                // noop
            });

    mocks[currentTestName()]["freeaddrinfo"] = (void*)+([] (struct addrinfo *__ai) {
                free(__ai);
            });

    Connection conn("localhost", 1337, false);
    ASSERT_EQ(Connection::ConnectResult::ERROR_RESOLVE, conn.connect());
    ASSERT_EQ(Connection::Status::UNKNOWN, conn.status());
}

TEST_F(ConnectionTest, invalidSocket) {
    mocks[currentTestName()]["getaddrinfo"] =
            (void*)+([] (const char *__restrict, const char *__restrict, const struct addrinfo *__restrict,
                         struct addrinfo **__restrict outAddr) {
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
            });
    mocks[currentTestName()]["socket"] =
            (void*)+([] (int , int , int ) {
                return -1;      // invalid socket
            });

    mocks[currentTestName()]["connect"] =
            (void*)+([] (int __fd, const sockaddr *, socklen_t ) {
                EXPECT_NE(-1, __fd) << "Missing check for invalid socket identifier!";
                return -1;
            });

    mocks[currentTestName()]["close"] =
            (void*)+([] (int ) {
                // noop
            });
    mocks[currentTestName()]["freeaddrinfo"] = (void*)+([] (struct addrinfo *__ai) {
                free(__ai);
            });

    Connection conn("localhost", 1337, false);
    ASSERT_EQ(Connection::ConnectResult::ERROR_INTERNAL, conn.connect());
    ASSERT_EQ(Connection::Status::UNKNOWN, conn.status());
}

TEST_F(ConnectionTest, successConnect1stAddressIPv4) {
    mocks[currentTestName()]["getaddrinfo"] =
        (void*)+([] (const char *__restrict, const char *__restrict, const struct addrinfo *__restrict,
                     struct addrinfo **__restrict outAddr) {
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
        });
    mocks[currentTestName()]["socket"] =
            (void*)+([] (int , int , int ) {
            return 42;      // just pick some socket identifier number, is used in tests only
        });

    mocks[currentTestName()]["connect"] =
        (void*)+([] (int __fd, const sockaddr *__addr, socklen_t ) {
            EXPECT_EQ(42, __fd) << "Internal socket descriptor should not change";
            // check that internal values stay same
            EXPECT_EQ(AF_INET, __addr->sa_family);
            EXPECT_ARRAY_EQ(const char, "01234567890123", __addr->sa_data, 14);

            // emulate connect -> successful connect
            return 0;
        });

    mocks[currentTestName()]["close"] =
            (void*)+([] (int ) {
                // noop
            });

    mocks[currentTestName()]["freeaddrinfo"] = (void*)+([] (struct addrinfo *__ai) {
                free(__ai);
            });


    Connection conn("localhost", 1337, false);
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    ASSERT_EQ(Connection::Protocol::IPv4, conn.protocol());
    ASSERT_EQ(Connection::Status::CONNECTED, conn.status());
}


TEST_F(ConnectionTest, successConnect2ndAddressIPv4) {
    mocks[currentTestName()]["getaddrinfo"] =
            (void*)+([] (const char *__restrict, const char *__restrict, const struct addrinfo *__restrict,
                         struct addrinfo **__restrict outAddr) {
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
            });
    mocks[currentTestName()]["socket"] =
            (void*)+([] (int , int , int ) {
                return 42;      // just pick some socket identifier number, is used in tests only
            });

    static int i;
    i = 0;
    mocks[currentTestName()]["connect"] =
            (void*)+([] (int __fd, const sockaddr *__addr, socklen_t ) {
                EXPECT_EQ(42, __fd) << "Internal socket descriptor should not change!";
                // check that internal values stay same
                EXPECT_EQ(AF_INET, __addr->sa_family);

                if (i == 0) {                // 1st address
                    EXPECT_ARRAY_EQ(const char, "00000000000000", __addr->sa_data, 14);
                    i++;
                    return -1;      // connect is unsuccessful
                } else if (i == 1) {         // 2nd address
                    EXPECT_ARRAY_EQ(const char, "01234567890123", __addr->sa_data, 14);
                    i++;
                    return 0;
                }
                ADD_FAILURE() << "Should not be reached!";
                return -1;
            });

    static bool checkClose = true;
    mocks[currentTestName()]["close"] =
            (void*)+([] (int __fd) {
                EXPECT_EQ(42, __fd) << "Internal socket descriptor should not change!";
                if (checkClose)
                    ASSERT_EQ(1, i) << "Only close failed sockets!";
            });

    mocks[currentTestName()]["freeaddrinfo"] = (void*)+([] (struct addrinfo *__ai) {
                free(__ai);
            });

    Connection conn("localhost", 1337, false);
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    // do not execute the assert in close if connect was already successful because a successful socket will be closed
    // there too
    checkClose = false;

    ASSERT_EQ(Connection::Protocol::IPv4, conn.protocol());
    ASSERT_EQ(Connection::Status::CONNECTED, conn.status());
}

TEST_F(ConnectionTest, realSSL) {
    mocks[currentTestName()]["getaddrinfo"] = (void*)&::getaddrinfo;
    mocks[currentTestName()]["socket"] = (void*)&::socket;
    mocks[currentTestName()]["connect"] = (void*)&::connect;
#ifdef __WIN32
    mocks[currentTestName()]["close"] = (void*)&::closesocket;
#else
    mocks[currentTestName()]["close"] = (void*)&::close;
    mocks[currentTestName()]["freeaddrinfo"] = (void*)&::freeaddrinfo;
#endif

    // tries to establish a connection to viaduck servers
    Connection conn("viaduck.org", 443, true, "/etc/ssl/certs");
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    ASSERT_EQ(Connection::Status::CONNECTED, conn.status());
    ASSERT_TRUE(conn.isSSL());
    conn.close();
}

TEST_F(ConnectionTest, realNoSSL) {
    mocks[currentTestName()]["getaddrinfo"] = (void*)&::getaddrinfo;
    mocks[currentTestName()]["socket"] = (void*)&::socket;
    mocks[currentTestName()]["connect"] = (void*)&::connect;
#ifdef __WIN32
    mocks[currentTestName()]["close"] = (void*)&::closesocket;
#else
    mocks[currentTestName()]["close"] = (void*)&::close;
    mocks[currentTestName()]["freeaddrinfo"] = (void*)&::freeaddrinfo;
#endif

    // tries to establish a connection to viaduck servers
    Connection conn("viaduck.org", 80, false);
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    ASSERT_EQ(Connection::Status::CONNECTED, conn.status());
    ASSERT_FALSE(conn.isSSL());
    conn.close();
}

TEST_F(ConnectionTest, sessionResumption) {
    mocks[currentTestName()]["getaddrinfo"] = (void*)&::getaddrinfo;
    mocks[currentTestName()]["socket"] = (void*)&::socket;
    mocks[currentTestName()]["connect"] = (void*)&::connect;
#ifdef __WIN32
    mocks[currentTestName()]["close"] = (void*)&::closesocket;
#else
    mocks[currentTestName()]["close"] = (void*)&::close;
    mocks[currentTestName()]["freeaddrinfo"] = (void*)&::freeaddrinfo;
#endif

    // tries to establish a connection to viaduck servers
    Connection conn("viaduck.org", 443, true, "/etc/ssl/certs");
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    ASSERT_EQ(Connection::Status::CONNECTED, conn.status());
    ASSERT_TRUE(conn.isSSL());
    conn.close();       // this saves SSL session

    uint16_t resumptionCount = SSLContext::getInstance().sessionsResumed();

    // following connections should use stored ssl sessions
    Connection conn2("viaduck.org", 443, true, "/etc/ssl/certs");
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn2.connect());
    ASSERT_EQ(Connection::Status::CONNECTED, conn2.status());
    ASSERT_EQ(resumptionCount+1, SSLContext::getInstance().sessionsResumed());
    ASSERT_TRUE(conn2.isSSL());
    conn2.close();

    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn2.connect());
    ASSERT_EQ(Connection::Status::CONNECTED, conn2.status());
    ASSERT_EQ(resumptionCount+2, SSLContext::getInstance().sessionsResumed());
    ASSERT_TRUE(conn2.isSSL());
}
