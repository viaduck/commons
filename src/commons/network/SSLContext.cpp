#include "commons/network/SSLContext.h"

thread_local SSLContext SSLContext::mInstance;

void SSLContext::saveSession(const Connection &connection, SSL_SESSION *session) {
    SSL_SESSION *storedSession = getSession(connection);
    if (storedSession != nullptr) {
        SSL_SESSION_free(storedSession);
    }
    mSessions.emplace(ConnectionInfo(connection), session);
}

SSL_SESSION *SSLContext::getSession(const Connection &connection) {
    auto elem = mSessions.find(ConnectionInfo(connection));

    return elem == mSessions.end() ? nullptr : (*elem).second;
}
