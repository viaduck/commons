/*
 * Copyright (C) 2015-2023 The ViaDuck Project
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

#ifndef COMMONS_SSLCONTEXT_H
#define COMMONS_SSLCONTEXT_H

#include <unordered_map>
#include <network/ConnectionInfo.h>

/**
 * Wrapper class for OpenSSL's SSL_CTX. Each thread has one static singleton SSLContext assigned to save the
 * sessions on a per-thread basis.
 */
class SSLContext {
    using SSL_CTX_ref = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>;
    using SSL_SESSION_ref = std::unique_ptr<SSL_SESSION, decltype(&SSL_SESSION_free)>;
public:
    /**
     * @return SSLContext singleton associated with this thread
     */
    static SSLContext &getInstance() {
        return mInstance;
    }

    /**
     * @return Native SSL_CTX
     */
    SSL_CTX *get() {
        return mCtx.get();
    }

    int dataIndex() const {
        return mDataIndex;
    }

    /**
     * Loads the context for verification
     *
     * @param certPath If given, passed to SSL load_verify_locations
     */
    void load(const std::string &certPath) {
        if (certPath.empty())
            Native::gInit.defaultStore(get());
        else
            L_expect(SSL_CTX_load_verify_locations(get(), nullptr, certPath.c_str()));
    }

    /**
     * Saves a session associated in session cache
     *
     * @param info Associated connection information
     * @param session Native OpenSSL SSL context
     */
    void saveSession(const ConnectionInfo &info, SSL_SESSION *session) {
        mSessions.emplace(info, SSL_SESSION_ref(session, &SSL_SESSION_free));
    }

    /**
     * Returns a session from session cache
     *
     * @param info Associated connection information
     * @return Saved session to resume
     */
    SSL_SESSION *getSession(const ConnectionInfo &info) {
        auto elem = mSessions.find(info);
        return elem == mSessions.end() ? nullptr : elem->second.get();
    }

    /**
     * Removes a session from session cache
     *
     * @param info Associated connection information
     */
    void removeSession(const ConnectionInfo &info, SSL_SESSION *session) {
        auto range = mSessions.equal_range(info);
        auto it = std::find_if(range.first, range.second, [&] (const auto &it) { return it.second.get() == session; });

        if (it != range.second)
            mSessions.erase(it);
    }

protected:
    /**
     * Constructor used internally to create singleton instance.
     */
    explicit SSLContext() : mCtx(SSL_CTX_new(TLS_client_method()), &SSL_CTX_free) {
        // simplify application logic by removing need for manually handling SSL state
        SSL_CTX_set_mode(get(), SSL_MODE_AUTO_RETRY);
        // data index used to pass custom data to ssl verify
        mDataIndex = SSL_get_ex_new_index(0, nullptr, nullptr, nullptr, nullptr);
        // set session caching mode to cache client sessions
        SSL_CTX_set_session_cache_mode(get(), SSL_SESS_CACHE_CLIENT);
    }

    // thread specific singleton
    static thread_local SSLContext mInstance;

    // native context
    SSL_CTX_ref mCtx;
    // ssl data index for custom data
    int mDataIndex;
    // saved sessions for resumption
    std::unordered_multimap<ConnectionInfo, SSL_SESSION_ref> mSessions;
};

#endif //COMMONS_SSLCONTEXT_H
