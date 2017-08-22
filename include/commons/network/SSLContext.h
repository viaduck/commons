#ifndef VDCOMMONS_SSLCONTEXT_H
#define VDCOMMONS_SSLCONTEXT_H

#include <openssl/ssl.h>

#include <commons/network/ConnectionInfo.h>
#include <unordered_map>

/**
 * Wrapper class for OpenSSL's SSL_CTX. Each thread has one static singleton SSLContext assigned to save the
 * sessions on a per-thread basis.
 */
class SSLContext {
public:
    /**
     * @return SSLContext singleton associated with this thread
     */
    static SSLContext &getInstance() {
        return mInstance;
    }

    /**
     * Maximum number of stored sessions (= session cache size)
     */
    const static uint8_t CACHE_SIZE = 20;

    /**
     * Frees OpenSSL's native SSL context
     */
    ~SSLContext() {
        SSL_CTX_free(mCtx);
    }

    /**
     * Custom conversion operator to return native OpenSSL context
     * @return Native SSL_CTX
     */
    operator SSL_CTX* () {
        return mCtx;
    }

    /**
     * Saves a session associated with Connection in global session cache
     * @param connection Associated connection
     * @param session Native OpenSSL SSL context
     */
    void saveSession(const Connection &connection, SSL_SESSION *session);

    /**
     * Returns a session from global session cache
     * @param connection
     * @return
     */
    SSL_SESSION *getSession(const Connection &connection);

    /**
     * @return Number of sessions resumed in this SSLContext
     */
    uint16_t sessionsResumed() const {
        return mSessionsResumed;
    }

protected:
    /**
     * Constructor used internally to create singleton instance.
     */
    SSLContext() : mCtx(SSL_CTX_new(TLS_client_method())), mSessions(CACHE_SIZE) {
        // simplify application logic by removing need for manually handling SSL state
        SSL_CTX_set_mode(SSLContext::getInstance(), SSL_MODE_AUTO_RETRY);
    }

    /**
     * Thread-specific singleton instance
     */
    static thread_local SSLContext mInstance;

    SSL_CTX *mCtx;
    std::unordered_map<ConnectionInfo, SSL_SESSION*> mSessions;
    uint16_t mSessionsResumed = 0;

    friend class Connection;
};


#endif //VDCOMMONS_SSLCONTEXT_H
