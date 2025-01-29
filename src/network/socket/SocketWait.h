/*
 * Copyright (C) 2023-2025 The ViaDuck Project
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

#ifndef COMMONS_SOCKETWAIT_H
#define COMMONS_SOCKETWAIT_H

#include <network/component/NetworkResult.h>
#include <network/socket/ISocket.h>

#include <optional>

class SocketWait {
public:
    /**
     * Flags of socket events, derived from select API.
     */
    enum Events : uint8_t {
        UNSET = 0,
        READABLE = 1,
        WRITEABLE = 2,
        EXCEPT = 4,
    };

    /**
     * SocketWait Entry describes a socket fd and the requested events to listen for
     * as well as the resulting events. Derived from poll API.
     */
    class Entry {
    public:
        explicit Entry(SOCKET _fd = INVALID_SOCKET, Events _request = Events::UNSET)
                : fd(_fd), request(_request), response(Events::UNSET) {

        }

        /// @returns True if this socket is readable
        bool readable() const { return (response & Events::READABLE) == Events::READABLE; }
        /// @returns True if this socket is writeable
        bool writeable() const { return (response & Events::WRITEABLE) == Events::WRITEABLE; }
        /// @returns True if this socket is in a state of exception
        bool except() const { return (response & Events::EXCEPT) == Events::EXCEPT; }

    protected:
        friend class SocketWait;

        SOCKET fd;
        Events request;
        Events response;
    };

    /**
     * Wait for all given entries until any entry receives one of the requested events or the specified
     * timeout is reached. Derived from poll API.
     *
     * @param entries List of entries to listen for.
     * If an entry fd is INVALID_SOCKET, it will be skipped entirely.
     * @param timeoutMs Optional number of milliseconds after which to stop waiting
     * if no requested event was raised for any valid entry. Defaults to indefinite.
     * @return NetworkResult containing SUCCESS or a network error
     * @throws std::out_of_range if any entry fd does not fit in a FD_SET
     */
    static NetworkResult wait(std::vector<Entry> &entries, const std::optional<int32_t> &timeoutMs = std::nullopt) {
        // create a set of sockets for select
        fd_set setR, setW, setE;
        FD_ZERO(&setR);
        FD_ZERO(&setW);
        FD_ZERO(&setE);

#ifdef _WIN32
        // on Windows, FD_SETSIZE may be redefined before including winsock.h
        // and specifies the number of complete SOCKETs in the "fd_set"

        // on Windows, FD_SETSIZE is the maximum number of allowed FDs
        if (entries.size() > FD_SETSIZE)
            throw std::out_of_range("too many fd for win32 select: " + std::to_string(entries.size())
                                    + " vs " + std::to_string(FD_SETSIZE));
#else
        // on BSD, FD_SETSIZE may also be redefined before including sys/types.h
        // on Linux, FD_SETSIZE is fixed

        // on these platforms, FD_SETSIZE is the largest allowed FD
        for (const auto &entry : entries)
            if (entry.fd > FD_SETSIZE)
                throw std::out_of_range("fd too large for select: " + std::to_string(entry.fd)
                                        + " vs " + std::to_string(FD_SETSIZE));
#endif

        // fill sets with the fds from the entries
        SOCKET maxfd = -1;
        for (const auto &entry : entries) {
            if (entry.fd == INVALID_SOCKET)
                continue;

            if ((entry.request & Events::READABLE) == Events::READABLE)
                FD_SET(entry.fd, &setR);
            if ((entry.request & Events::WRITEABLE) == Events::WRITEABLE)
                FD_SET(entry.fd, &setW);
            if ((entry.request & Events::EXCEPT) == Events::EXCEPT)
                FD_SET(entry.fd, &setE);

            maxfd = std::max(maxfd, entry.fd);
        }

        // timeout in microseconds
        auto t = timeoutMs.value_or(0);
        timeval tv = {.tv_sec = t / 1000, .tv_usec = 1000 * (t % 1000)};

        // waits for any socket to become readable/writeable/except
        auto rv = Native::select(maxfd + 1, &setR, &setW, &setE, timeoutMs.has_value() ? &tv : nullptr);
        if (rv > 0) {
            for (auto &entry : entries) {
                if (entry.fd == INVALID_SOCKET)
                    continue;

                if (FD_ISSET(entry.fd, &setR))
                    entry.response = static_cast<Events>(entry.response | Events::READABLE);
                if (FD_ISSET(entry.fd, &setW))
                    entry.response = static_cast<Events>(entry.response | Events::WRITEABLE);
                if (FD_ISSET(entry.fd, &setE))
                    entry.response = static_cast<Events>(entry.response | Events::EXCEPT);
            }

            // success
            return NetworkResultType::SUCCESS;
        }

        // rv is 0 for timeout or SOCKET_ERROR on error
        if (rv == 0)
            return NetworkTimeoutError();
        return NetworkOSError();
    }

    /**
     * Convenience: like wait but for one entry only.
     *
     * @see SocketWait::wait
     */
    static NetworkResult waitOne(Entry &entry, const std::optional<int32_t> &timeoutMs = std::nullopt) {
        std::vector<Entry> entries = { entry };

        auto rv = wait(entries, timeoutMs);
        if (rv == NetworkResultType::SUCCESS)
            entry = entries[0];

        return rv;
    }
};

// flags operators
inline SocketWait::Events operator|(SocketWait::Events lhs, SocketWait::Events rhs) {
    return static_cast<SocketWait::Events>(static_cast<std::underlying_type<SocketWait::Events>::type>(lhs)
            | static_cast<std::underlying_type<SocketWait::Events>::type>(rhs));
}
inline SocketWait::Events operator&(SocketWait::Events lhs, SocketWait::Events rhs) {
    return static_cast<SocketWait::Events>(static_cast<std::underlying_type<SocketWait::Events>::type>(lhs)
            & static_cast<std::underlying_type<SocketWait::Events>::type>(rhs));
}

#endif //COMMONS_SOCKETWAIT_H
