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

#ifndef COMMONS_NETWORKRESULT_H
#define COMMONS_NETWORKRESULT_H

#include <network/component/NetworkError.h>
#include <commons/util/Result.h>

#include <memory>

/// possible result value types for the NetworkResult
enum class NetworkResultType {
    SUCCESS,
    WAIT_EVENT,
    RETRY
};

/// holds polymorphic error types for the NetworkResult
class NetworkErrorType {
public:
    template<typename T, std::enable_if_t<std::is_base_of_v<INetworkError, T>>* = nullptr>
    NetworkErrorType(T &&error) // NOLINT(*-explicit-constructor)
            : mError(std::make_unique<T>(error)) {

    }

    template<typename T, std::enable_if_t<std::is_base_of_v<INetworkError, T>>* = nullptr>
    [[nodiscard]] bool isErrorType() const {
        // check if error "instance of" T
        return dynamic_cast<const T*>(mError.get());
    }

protected:
    // compare errors, required to be a Result E type
    friend bool operator==(const NetworkErrorType &, const NetworkErrorType &);

    std::shared_ptr<INetworkError> mError;
};

/**
 * Custom result type for networking, contains either a success/deferred result or an error class
 */
class NetworkResult : public result::Result<NetworkResultType, NetworkErrorType> {
public:
    NetworkResult(NetworkResultType value) : Result(value) { } // NOLINT(*-explicit-constructor)

    template<typename T, std::enable_if_t<std::is_base_of_v<INetworkError, T>>* = nullptr>
    NetworkResult(T &&error) : Result(NetworkErrorType(std::forward<T>(error))) { } // NOLINT(*-explicit-constructor)

    /// @returns True iff the result contains a value of deferred success, like wait or retry
    [[nodiscard]] bool isDeferred() const {
        return has_value() && value() != NetworkResultType::SUCCESS;
    }

    /// @returns True iff the result contains an error of type T or derived from T
    template<typename T, std::enable_if_t<std::is_base_of_v<INetworkError, T>>* = nullptr>
    [[nodiscard]] bool isErrorType() const {
        return !has_value() && error().isErrorType<T>();
    }

    /// if this result represents a timeout error, convert it to a wait result
    NetworkResult timeoutToWait();

protected:
    // friend compare NetworkResult to value type
    friend bool operator==(const NetworkResult &, const NetworkResultType &);
    friend bool operator==(const NetworkResultType &, const NetworkResult &);
};

// compare NetworkResult to error type
template<typename T, std::enable_if_t<std::is_base_of_v<INetworkError, T>>* = nullptr>
bool operator==(const NetworkResult &lhs, const T &) {
    return lhs.isErrorType<T>();
}
template<typename T, std::enable_if_t<std::is_base_of_v<INetworkError, T>>* = nullptr>
bool operator==(const T &, const NetworkResult &rhs) {
    return rhs.isErrorType<T>();
}

#endif //COMMONS_NETWORKRESULT_H
