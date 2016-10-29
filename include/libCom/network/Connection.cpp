#include <unistd.h>

#include <libCom/openssl_hook.h>
#include <libCom/network/Connection.h>

Connection::Connection(std::string host, uint16_t port) : mHost(host), mPort(port) {
    // TODO detect IPv6 and set protocol accordingly
    mProtocol = Protocol::IPv4;

    global_initOpenSSL();
}

Connection::~Connection() {
    close();
    // TODO cleanup?
}

Connection::ConnectResult Connection::connect() {
    struct addrinfo addressQuery;
    memset(&addressQuery, 0, sizeof(addressQuery));


    // address query parameters
    addressQuery.ai_family = PF_UNSPEC;
    addressQuery.ai_socktype = SOCK_STREAM;
    addressQuery.ai_protocol = IPPROTO_TCP;

    // resolve hostname
    struct addrinfo *addressInfoTmp = nullptr;
    int res = getaddrinfo(mHost.c_str(), nullptr, &addressQuery, &addressInfoTmp);

    std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> addressInfo(addressInfoTmp, &freeaddrinfo);

    if (res != 0)
        return ConnectResult::ERROR_RESOLVE;

    // iterate over all available addresses
    for (struct addrinfo *it = addressInfo.get(); it; it = it->ai_next) {
        mSocket = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (mSocket == -1)
            return ConnectResult ::ERROR_INTERNAL;

        // call global connect function
        res = ::connect(mSocket, it->ai_addr, it->ai_addrlen);
        if (res == -1) {
            ::close(mSocket);
            mSocket = -1;
        } else        // if there is a successful connection, return success
            return ConnectResult::SUCCESS;
    }

    // no resolved address has been successful
    // TODO more verbose error reporting
    return ConnectResult::ERROR_CONNECT;
}

bool Connection::close() {
    if (mSocket != -1) {
        ::close(mSocket);
        return true;
    }
    return false;
}
