#include <iostream>
#include <unistd.h>

#include <libCom/Buffer.h>
#include <libCom/Range.h>
#include <libCom/openssl_hook.h>
#include <libCom/Request.h>

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/conf.h>

const char STICKY_PUBKEY[] = "-----BEGIN PUBLIC KEY-----\n"
                            "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA4tmmlX6LxHFfkUr+L3Tz\n"
                            "Mfyw2RrkPvIgtSgtwHEIIQq5By3zsT0m8pNfpspascIQjtJ47A+HkbAgzn0tQvuI\n"
                            "D9sQPbrdtHrHll5zH4jOPPuibx0dczmmXN3cBnMZZZaUMmYclwvSZ8zu3nJC8iG5\n"
                            "t1ITRlnCvnNzjqHF2v2vGvfth7KcmVrb8q4wlI9kfdiuL0ypm9A/OWA0wjgQOAUq\n"
                            "RXe7z1aqDU7fqyM72Vynkw7aWzg/gitWA1t7NT6Ph8aVRcTAffRBdcOA+B6kTVKc\n"
                            "DonHvD0qk758ieFEED8bWdEsP+gwAVdbWD4pRfZzYYJmqCpsfFfDZ7b5DKRs0Cbr\n"
                            "GRdIwZC/yXzLKKlG1MrkeXbmEfK05SyjF6R8swULDK29oms6LjxSSZPdJEB+kUf6\n"
                            "aWKT0+hoRdvXJh+dMjj+WBTAoqSu5SP7yfxZePDO41MtP42MQaELFy0e0QqV2z8r\n"
                            "WlMAbYweVPMtsvTplB1dCj2YB9ud1G1l2H0xtb+iapeF4gKm9hXs+JPPx++9WVGK\n"
                            "gttgruOsj3ukjtVlB2gKUV5jj3B9EwQsuiGNJIsC/v9nKom3fTiLRZYkCYCCfZ/H\n"
                            "tVOlJjPbHELFA5sFhVaPlX3BDQCL+w5hYVZPsZ64JZI7bkIDIS0Q8R9zsAoK+98A\n"
                            "CZTMil/tviZShNIisKNfPpUCAwEAAQ==\n"
                            "-----END PUBLIC KEY-----";
int STICKY_PUBKEY_SIZE = sizeof(STICKY_PUBKEY)/sizeof(STICKY_PUBKEY[0]);

using namespace std;

Request::Request(std::string host, u_short port, bool IPv6, bool stickyPubKey) {
	this->host = host;
	this->port = port;
	this->ipv6 = IPv6;
    this->stickyPubKey = stickyPubKey;

	global_initOpenSSL();
}

Request::~Request() {
    close();
}

void printError() {
    std::cerr << ERR_func_error_string(ERR_get_error()) << std::endl;
}

int Request::initSsl(int fd) {
	const SSL_METHOD *method;

	method = TLS_client_method();
	if (method == nullptr) {
		return -3;
	}

	ctx = SSL_CTX_new(method);
	if (ctx == nullptr) {
        printError();
        return -4;
    }
    // options
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    // verify
    if (stickyPubKey)
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, [] (int preVerify, X509_STORE_CTX *ctx) -> int {
            if (X509_STORE_CTX_get_error_depth(ctx) > 0)
                return preVerify;

            X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
            if (cert) {
                using RSA_ref = std::unique_ptr<RSA, decltype(&RSA_free)>;
                using BIO_ref = std::unique_ptr<BIO, decltype(&BIO_free)>;
                using EVP_PKEY_ref = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

                EVP_PKEY *pubKey = X509_get_pubkey(cert);

                // create memory bio
                BIO_ref pubKeyStoredBio(BIO_new(BIO_s_mem()), &BIO_free);
                if (!pubKeyStoredBio)
                    return 0;

                // write PEM data into bio
                if (BIO_write(pubKeyStoredBio.get(), STICKY_PUBKEY, STICKY_PUBKEY_SIZE) != STICKY_PUBKEY_SIZE)
                    return 0;

                // parse the PEM private key data
                RSA_ref pubKeyStoredRSA(PEM_read_bio_RSA_PUBKEY(pubKeyStoredBio.get(), nullptr, nullptr, nullptr), &RSA_free);
                if (!pubKeyStoredRSA)
                    return 0;
                EVP_PKEY_ref pubKeyStored(EVP_PKEY_new(), &EVP_PKEY_free);
                EVP_PKEY_set1_RSA(pubKeyStored.get(), pubKeyStoredRSA.get());

                // now compare the keys
                int cmpRes = EVP_PKEY_cmp(pubKey, pubKeyStored.get());
                if (cmpRes == 1)
                    return 1;
            }
            return 0;
        });
    else
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    SSL_CTX_set_verify_depth(ctx, 8);
    // load default certificate path for verification
    SSL_CTX_set_default_verify_paths(ctx);      // TODO issue #6

	ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        printError();
        return -5;
    }
	if (SSL_set_fd(ssl, fd) == 0) {
        printError();
        return -6;
    }
    //SSL_set_tlsext_host_name(ssl, "host.name");       // TODO hostname lookup
    if (SSL_connect(ssl) != 1) {
        printError();
        return -7;
    }

    initDone = true;
    
    return 0;
}

int Request::init() {
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd == (SOCKET)SOCKET_ERROR) {
        return -1;
    }
/* TODO unused rcv and snd timeout code
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        std::cerr<<"Failed to setsockopt. Code: "<<strerror(errno);
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
        std::cerr<<"Failed to setsockopt. Code: "<<strerror(errno);
*/
    sockaddr_in service;
    if (this->ipv6) {
        service.sin_port = AF_INET6;
    } else {
        service.sin_family = AF_INET;
    }
    service.sin_port = htons(this->port);
    service.sin_addr.s_addr = inet_addr(this->host.c_str());

    // TODO hostname lookup
    int result = connect(fd, reinterpret_cast<sockaddr *>(&service), sizeof(service));
    if (result == -1) {
        return -2;
    }

    int ret = initSsl(fd);
    // close connection if SSL init fails
    if (ret != 0)
        ::close(fd);

    return ret;
}

bool Request::read(Buffer &buffer, const uint32_t min) {
    if (!initDone)
        return false;

    uint32_t read = 0;
    int res;
    uint8_t iters = 0;
    buffer.increase(buffer.size() + 512 * 4);     // must be big enough to hold at least 512 bytes (*4 for 4 iterations)

    // TODO read timeout, non-blocking?
    while ((res = SSL_read(ssl, buffer.data(buffer.size()), 512)) > 0) {
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

int32_t Request::readMax(Buffer &buffer, const uint32_t size) {
    if (!initDone)
        return -1;

    int res;
    buffer.increase(size, true);     // must be big enough to hold at least size bytes

    // TODO read timeout, non-blocking?
    res = SSL_read(ssl, buffer.data(buffer.size()), size);
    if (res > 0)
        buffer.use(static_cast<uint32_t>(res));

    return res;
}

bool Request::readExactly(Buffer &buffer, const uint32_t size) {
    if (!initDone)
        return false;

    uint32_t read = 0;
    int res;
    buffer.increase(size, true);        // must be big enough to hold at least size bytes

    // TODO read timeout
    while (read != size && (res = SSL_read(ssl, buffer.data(buffer.size()), size-read)) > 0) {
        read += res;
        buffer.use(static_cast<uint32_t>(res));
    }

    return read == size;
}

bool Request::write(const Buffer &buffer) {
    if (!initDone)
        return false;

    // TODO: writeExactly
    int res = SSL_write(ssl, buffer.const_data(), buffer.size());
    if (res <= 0) return false;

    uint32_t writtenbytes = static_cast<uint32_t>(res);
    return writtenbytes == buffer.size();
}

void Request::close() {
    if (initDone) {
        SSL_shutdown(ssl);
        ::close(fd);        // global namespace socket close method, not the member method!
        SSL_CTX_free(ctx);
        SSL_free(ssl);
    }
}
