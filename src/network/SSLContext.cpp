/*
 * Copyright (C) 2015-2018 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "network/SSLContext.h"

thread_local SSLContext SSLContext::mInstance;

void SSLContext::saveSession(const Connection &connection, SSL_SESSION *session) {
    mSessions.emplace(ConnectionInfo(connection), SSL_SESSION_ref(session, &SSL_SESSION_free));
}

SSL_SESSION *SSLContext::getSession(const Connection &connection) {
    auto elem = mSessions.find(ConnectionInfo(connection));

    return elem == mSessions.end() ? nullptr : (*elem).second.get();
}
