#include "commons/network/SSLContext.h"

thread_local SSLContext SSLContext::mInstance;

void SSLContext::saveSession(const Connection &connection, SSL_SESSION *session) {
    mSessions.emplace(ConnectionInfo(connection), SSL_SESSION_ref(session, &SSL_SESSION_free));
}

SSL_SESSION *SSLContext::getSession(const Connection &connection) {
    auto elem = mSessions.find(ConnectionInfo(connection));

    return elem == mSessions.end() ? nullptr : (*elem).second.get();
}
