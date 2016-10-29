#include <netdb.h>
#include <unordered_map>

#include <libCom/network/Connection.h>
#include "custom_assert.h"
#include "ConnectionTest.h"

inline const char *currentTestName() {
    return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

static std::unordered_map<std::string, std::unordered_map<std::string, void*>> mocks;

#define callMockFunction(Function,...) (*(decltype(&Function)) (mocks[currentTestName()][#Function]))(__VA_ARGS__)

// let's mock the socket functions to emulate network behavior
extern "C" {
int getaddrinfo (const char *__restrict __name,
                 const char *__restrict __service,
                 const struct addrinfo *__restrict __req,
                 struct addrinfo **__restrict __pai) {
    return callMockFunction(getaddrinfo, __name, __service, __req, __pai);
}

int socket(int __domain, int __type, int __protocol) {
    return callMockFunction(socket, __domain, __type, __protocol);
}

int connect(int __fd, const sockaddr *__addr, socklen_t __len) {
    return callMockFunction(connect, __fd, __addr, __len);
}

int close(int __fd) {
    return callMockFunction(close, __fd);
}
}

void ConnectionTest::SetUpTestCase() {
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

    Connection conn("localhost", 1337);
    ASSERT_EQ(Connection::ConnectResult::ERROR_RESOLVE, conn.connect());
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

    Connection conn("localhost", 1337);
    ASSERT_EQ(Connection::ConnectResult::ERROR_CONNECT, conn.connect());
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

    Connection conn("localhost", 1337);
    ASSERT_EQ(Connection::ConnectResult::ERROR_INTERNAL, conn.connect());
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


    Connection conn("localhost", 1337);
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    ASSERT_EQ(Connection::Protocol::IPv4, conn.protocol());
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

    Connection conn("localhost", 1337);
    ASSERT_EQ(Connection::ConnectResult::SUCCESS, conn.connect());
    // do not execute the assert in close if connect was already successful because a successful socket will be closed
    // there too
    checkClose = false;

    ASSERT_EQ(Connection::Protocol::IPv4, conn.protocol());
}
