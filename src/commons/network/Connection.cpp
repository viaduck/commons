#include <unistd.h>
#include <fcntl.h>

#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/x509_vfy.h>

#include <commons/network/Connection.h>
#include <commons/network/ConnectionInfo.h>
#include <commons/network/SSLContext.h>

#include "NativeWrapper.h"

Connection::Connection(std::string host, uint16_t port, bool ssl, std::string certPath, CertificateStorage &certStore,
                       uint16_t timeout) :
        mHost(host), mPort(port), mTimeout(timeout), mUsesSSL(ssl), mCertPath(certPath), mCertStore(certStore) { }

Connection::~Connection() {
    close();
}

void socket_io_timeout(SOCKET s, uint16_t t) {
    if (t == 0)
        return;

#if WIN32
    DWORD tv = t;
#else
    struct timeval tv;
    tv.tv_sec = t;
    tv.tv_usec = 0;
#endif

    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, static_cast<const char*>(static_cast<void*>(&tv)), sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, static_cast<const char*>(static_cast<void*>(&tv)), sizeof(tv));
}

void socket_io_nonblock(SOCKET s, bool value) {
#if WIN32
    ulong mode = value ? 1 : 0;  // 1 to enable non-blocking socket
    ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, NULL);
    value ? (flags |= O_NONBLOCK) : (flags &= ~O_NONBLOCK);
    fcntl(s, F_SETFL, flags);
#endif
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
        if (!handleSpecialDNS(it))
            return ConnectResult::ERROR_RESOLVE;

        mSocket = NativeWrapper::socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (mSocket == INVALID_SOCKET)
            return ConnectResult::ERROR_INTERNAL;

        // set read / write timeout
        socket_io_timeout(mSocket, mTimeout);
        // make socket non blocking
        socket_io_nonblock(mSocket, true);

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
            ConnectResult res;
            if (mUsesSSL &&  (res = initSsl()) != ConnectResult::SUCCESS)
                return res;

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
        if (mUsesSSL && status() == Status::CONNECTED) {
            // save current SSL session
            SSLContext::getInstance().saveSession(*this, SSL_get1_session(mSSL));

            // disconnect & cleanup SSL
            SSL_shutdown(mSSL);
            SSL_free(mSSL);
        }

        NativeWrapper::close(mSocket);
        mStatus = Status::UNCONNECTED;
        return true;
    }
    return false;
}

bool Connection::read(Buffer &buffer, const uint32_t size) {
    if (status() != Status::CONNECTED)
        return false;

    uint32_t read = 0;
    ssize_t res;
    buffer.increase(size, true);        // must be big enough to hold at least size bytes

    if (mUsesSSL)
        res = SSL_read(mSSL, buffer.data(buffer.size()), size-read);
    else
        res = NativeWrapper::recv(mSocket, buffer.data(buffer.size()), size-read);

    while (read != size && (res > 0)) {
        read += res;
        buffer.use(static_cast<uint32_t>(res));
    }

    return read == size;
}

ssize_t Connection::readMax(Buffer &buffer, const uint32_t size) {
    if (status() != Status::CONNECTED)
        return -1;

    ssize_t res;
    buffer.increase(size, true);     // must be big enough to hold at least size bytes

    if (mUsesSSL)
        res = SSL_read(mSSL, buffer.data(buffer.size()), size);
    else
        res = NativeWrapper::recv(mSocket, buffer.data(buffer.size()), size);

    if (res > 0)
        buffer.use(static_cast<uint32_t>(res));

    return res;
}

bool Connection::write(const Buffer &buffer) {
    if (status() != Status::CONNECTED)
        return false;

    ssize_t res;
    if (mUsesSSL)
        res = SSL_write(mSSL, buffer.const_data(), buffer.size());
    else
        res = NativeWrapper::send(mSocket, buffer.const_data(), buffer.size());

    if (res <= 0)
        return false;

    uint32_t writtenbytes = static_cast<uint32_t>(res);
    return writtenbytes == buffer.size();
}

Connection::ConnectResult Connection::initSsl() {
    if (!initVerification())  {
        // TODO more verbose error reporting
        return ConnectResult::ERROR_INVALID_CERTPATH;
    }

    mSSL = SSL_new(SSLContext::getInstance());
    if (mSSL == nullptr) {
        // TODO more verbose error reporting
        return ConnectResult::ERROR_INTERNAL;
    }

    SSL_set_tlsext_host_name(mSSL, mHost.c_str());
    // storing this to access CertificateStorage member
    SSL_set_ex_data(mSSL, CertificateStorage::getOpenSSLDataIndex(), this);

    if (SSL_set_fd(mSSL, mSocket) == 0) {
        // TODO more verbose error reporting
        return ConnectResult::ERROR_INTERNAL;
    }

    // set stored session
    SSL_SESSION *session = SSLContext::getInstance().getSession(*this);
    if (session != nullptr)
        SSL_set_session(mSSL, session);

    int ret;
    if ((ret = SSL_connect(mSSL)) != 1) {
        switch (SSL_get_error(mSSL, ret)) {
            case SSL_ERROR_SSL: {
                unsigned long err = ERR_peek_last_error();
                int lib = ERR_GET_LIB(err);
                int reason = ERR_GET_REASON(err);

                if (lib == ERR_LIB_SSL && reason == SSL_R_CERTIFICATE_VERIFY_FAILED)
                    return ConnectResult::ERROR_SSL_VERIFY;
                else
                    return ConnectResult::ERROR_SSL_GENERAL;
            }
            case SSL_ERROR_NONE:
                return ConnectResult::ERROR_CONNECT;
            default:
                return ConnectResult::ERROR_SSL_GENERAL;
        }
    }

    // count how often we saved the user's traffic
    if (SSL_session_reused(mSSL))
        SSLContext::getInstance().mSessionsResumed++;

    return ConnectResult::SUCCESS;
}

bool Connection::initVerification() {
    if (mCertPath != "")
        if (SSL_CTX_load_verify_locations(SSLContext::getInstance(), nullptr, mCertPath.c_str()) == 0)
            return false;

    SSL_CTX_set_verify(SSLContext::getInstance(), SSL_VERIFY_PEER, [] (int preVerify, X509_STORE_CTX *ctx) -> int {
        // preVerify indicates success of last level. 1 is success, 0 is fail
        // return value: 1 is success, 0 is fail

        X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
        if (cert) {
            EVP_PKEY *pubKey = X509_get_pubkey(cert);

            // get context user data
            SSL *ssl = static_cast<SSL*>(X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
            Connection *thiz = static_cast<Connection*>(SSL_get_ex_data(ssl, CertificateStorage::getOpenSSLDataIndex()));

            switch (thiz->mCertStore.check(pubKey, CertificateStorage::Mode::UNDECIDED)) {
                case CertificateStorage::Mode::DENY:
                    return 0;
                case CertificateStorage::Mode::ALLOW:
                    return 1;
                case CertificateStorage::Mode::UNDECIDED:
                    return preVerify;
            }
        }
        return 0;
    });
    return true;
}

bool Connection::handleSpecialDNS(const struct addrinfo *address) {
    const static uint32_t DNS_NAME_COLLISION = ntoh(static_cast<uint32_t>(0x7f003535));       // 127.0.53.53

    // IPv4
    if (address->ai_family == AF_INET) {
        // see https://www.icann.org/news/announcement-2-2014-08-01-en
        if (reinterpret_cast<sockaddr_in *>(address->ai_addr)->sin_addr.s_addr == DNS_NAME_COLLISION)
            return false;
    }
    return true;
}
