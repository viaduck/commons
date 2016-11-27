#include "libCom/network/SSLContext.h"

thread_local SSLContext SSLContext::mInstance;

void SSLContext::saveSession(const Connection &connection, SSL_SESSION *session) {
    SSL_SESSION *storedSession = getSession(connection);
    if (storedSession != nullptr) {
        SSL_SESSION_free(storedSession);
    }
    mSessions.write(ConnectionInfo(connection), session);
}

SSL_SESSION *SSLContext::getSession(const Connection &connection) {
    return mSessions.read(ConnectionInfo(connection));
}
