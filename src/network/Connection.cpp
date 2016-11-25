#include <unistd.h>

#include <libCom/network/Connection.h>
#include <libCom/openssl_hook.h>

#include "NativeWrapper.h"

Connection::Connection(std::string host, uint16_t port, bool ssl) : mHost(host), mPort(port), mUsesSSL(ssl) {
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
    int res = NativeWrapper::getaddrinfo(mHost.c_str(), std::to_string(mPort).c_str(), &addressQuery, &addressInfoTmp);

    std::unique_ptr<struct addrinfo, decltype(&freeaddrinfo)> addressInfo(addressInfoTmp, &freeaddrinfo);

    if (res != 0)
        return ConnectResult::ERROR_RESOLVE;

    // iterate over all available addresses
    for (struct addrinfo *it = addressInfo.get(); it; it = it->ai_next) {
        mSocket = NativeWrapper::socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (mSocket == INVALID_SOCKET)
            return ConnectResult::ERROR_INTERNAL;

        // call global connect function
        res = NativeWrapper::connect(mSocket, it->ai_addr, it->ai_addrlen);
        if (res == -1) {
            NativeWrapper::close(mSocket);
            mSocket = INVALID_SOCKET;
        } else {
            switch (it->ai_family) {
                case AF_INET:
                    mProtocol = Protocol::IPv4; break;
                case AF_INET6:
                    mProtocol = Protocol::IPv6; break;
                default:
                    mProtocol = Protocol::UNSET;
            }

            // try to establish SSL session
            if (mUsesSSL && initSsl() != SSLResult::SUCCESS) {
                // TODO more verbose error reporting
                return ConnectResult::ERROR_SSL;
            }

            mStatus = Status::CONNECTED;
            return ConnectResult::SUCCESS;
        }
    }

    // no resolved address has been successful
    // TODO more verbose error reporting
    return ConnectResult::ERROR_CONNECT;
}

bool Connection::close() {
    if (mSocket != INVALID_SOCKET) {
        NativeWrapper::close(mSocket);
        mStatus = Status::UNCONNECTED;
        return true;
    }
    return false;
}

bool Connection::read(Buffer &buffer, const uint32_t min) {
    if (status() != Status::CONNECTED)
        return false;

    uint32_t read = 0;
    int res;
    uint8_t iters = 0;
    buffer.increase(buffer.size() + 512 * 4);     // must be big enough to hold at least 512 bytes (*4 for 4 iterations)

    // TODO read timeout, non-blocking?
    while ((res = SSL_read(mSSL, buffer.data(buffer.size()), 512)) > 0) {
        buffer.use(static_cast<uint32_t>(res));
        read += res;

        if (res != 512 || read >= min)
            break;

        iters++;
        if (iters == 4) {      // buffer is not big enough for another iteration -> increase it (another 4 iterations)
            buffer.increase(buffer.size() + 512 * 4);
            iters = 0;
        }
    }
    return res > 0;
}

int32_t Connection::readMax(Buffer &buffer, const uint32_t size) {
    if (status() != Status::CONNECTED)
        return -1;

    int res;
    buffer.increase(size, true);     // must be big enough to hold at least size bytes

    // TODO read timeout, non-blocking?
    res = SSL_read(mSSL, buffer.data(buffer.size()), size);
    if (res > 0)
        buffer.use(static_cast<uint32_t>(res));

    return res;
}

bool Connection::readExactly(Buffer &buffer, const uint32_t size) {
    if (status() != Status::CONNECTED)
        return false;

    uint32_t read = 0;
    int res;
    buffer.increase(size, true);        // must be big enough to hold at least size bytes

    // TODO read timeout
    while (read != size && (res = SSL_read(mSSL, buffer.data(buffer.size()), size-read)) > 0) {
        read += res;
        buffer.use(static_cast<uint32_t>(res));
    }

    return read == size;
}

bool Connection::write(const Buffer &buffer) {
    if (status() != Status::CONNECTED)
        return false;

    // TODO: writeExactly
    int res = SSL_write(mSSL, buffer.const_data(), buffer.size());
    if (res <= 0) return false;

    uint32_t writtenbytes = static_cast<uint32_t>(res);
    return writtenbytes == buffer.size();
}

Connection::SSLResult Connection::initSsl() {
    const SSL_METHOD *method = TLS_client_method();
    if (method == nullptr) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    mSSLContext = SSL_CTX_new(method);
    if (mSSLContext == nullptr) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    // simplify application logic
    SSL_CTX_set_mode(mSSLContext, SSL_MODE_AUTO_RETRY);

    // TODO certificate verification
    mSSL = SSL_new(mSSLContext);
    if (mSSL == nullptr) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    if (SSL_set_fd(mSSL, mSocket) == 0) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_INTERNAL;
    }

    if (SSL_connect(mSSL) != 1) {
        // TODO more verbose error reporting
        return SSLResult::ERROR_CONNECT;
    }

    return SSLResult::SUCCESS;
}
