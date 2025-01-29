/*
 * Copyright (C) 2025 The ViaDuck Project
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

#include <network/component/NetworkResult.h>

#include "../native/Native.h"

NetworkOSError::NetworkOSError()
#ifdef _WIN32
        : NetworkOSError(WSAGetLastError()) {
#else
        : NetworkOSError(errno) {
#endif
}

NetworkResult NetworkResult::timeoutToWait() {
    if (*this == NetworkTimeoutError())
        return NetworkResultType::WAIT_EVENT;

    return *this;
}

bool operator==(const NetworkErrorType &lhs, const NetworkErrorType &rhs) {
    auto &lhr = *lhs.mError;
    auto &rhr = *rhs.mError;
    return typeid(lhr) == typeid(rhr);
}

bool operator==(const NetworkResult &lhs, const NetworkResultType &rhs) {
    // result is equal iff result has a value and the value is equal to the specified value
    return lhs.has_value() && lhs.value() == rhs;
}
bool operator==(const NetworkResultType &lhs, const NetworkResult &rhs) {
    return rhs == lhs;
}
