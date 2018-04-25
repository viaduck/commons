#ifndef COMMONS_NATIVEINIT_H
#define COMMONS_NATIVEINIT_H

#include <commons/util/Except.h>

/* network includes */
#ifdef WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <wincrypt.h>
    #define NW__SHUT_RDWR SD_BOTH
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <fcntl.h>
    #include <cerrno>
    #define INVALID_SOCKET  (SOCKET)(~0)
    #define SOCKET_ERROR            (-1)
    #define NW__SHUT_RDWR SHUT_RDWR
#endif

#include <memory>
#include <openssl/ssl.h>
#include <openssl/x509.h>

DEFINE_ERROR(native_init, base_error);

/**
 * Platform one-time init
 */
class NativeInit {
    using X509_ref = std::unique_ptr<X509, decltype(&X509_free)>;
public:
#ifdef WIN32
    NativeInit() {
        // windows socket startup
        WSADATA w;
        L_assert(WSAStartup(MAKEWORD(2,2), &w) == 0, native_init_error);

        // populate openssl certificate store from windows cert store
        mRootStore = X509_STORE_new();
        populateStore("ROOT", mRootStore);
    }

    ~NativeInit() {
        L_expect(WSACleanup());
    }

    // adapted from https://stackoverflow.com/a/40046425/207861
    void populateStore(const char *winStore, X509_STORE *targetStore) {
        // open windows CA store
        HCERTSTORE store = CertOpenSystemStore(0, winStore);
        L_assert(store, native_init_error);

        // due to the structure of the loop, each cert context will be automatically freed by the next iteration
        for (PCCERT_CONTEXT winCert = nullptr; (winCert = CertEnumCertificatesInStore(store, winCert)); ) {
            // warning: always pass the ptr as local variable, never directly (d2i_X509 modifies the ptr)
            const unsigned char *encodedCert = winCert->pbCertEncoded;
            // warning: do NOT pass a reuse parameter (d2i_X509 expects it to be valid X509 and preserves its data)
            X509_ref target(d2i_X509(nullptr, &encodedCert, winCert->cbCertEncoded), &X509_free);

            if (target)
                L_expect(X509_STORE_add_cert(targetStore, target.get()) != 0);
        }

        // close store
        L_assert(CertCloseStore(store, 0), native_init_error);
    }

#else
    NativeInit() = default;
    ~NativeInit() = default;
#endif

    void setStore(SSL_CTX *ctx) {
        if (mRootStore) {
            // ensure our store does not get freed on ctx store replace
            X509_STORE_up_ref(mRootStore);
            SSL_CTX_set_cert_store(ctx, mRootStore);
        }
    }

protected:
    X509_STORE *mRootStore = nullptr;
};

#endif //COMMONS_NATIVEINIT_H
