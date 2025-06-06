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
 *
 *
 * Adapted from: https://github.com/oktal/result/commit/d3598ece51e89054706222a480aa6cabfae2b93e
 *
 * Copyright {2016} {Mathieu Stefani}
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

#ifndef COMMONS_RESULT_H
#define COMMONS_RESULT_H

/*
   Mathieu Stefani, 03 mai 2016

   This header provides a Result type that can be used to replace exceptions in code
   that has to handle error.

   Result<T, E> can be used to return and propagate an error to the caller. Result<T, E> is an algebraic
   data type that can either Ok(T) to represent success or Err(E) to represent an error.
*/

#include <iostream>
#include <functional>
#include <type_traits>
#include <variant>

#include "Except.h"

DEFINE_ERROR(result, base_error);

// clang-format off
// @formatter:off

#define RESULT_MAKE_MAPPING_INTERNAL(name, target)                              \
    template<typename Ret, typename... Args>                                    \
    struct name<Ret (*)(Args...)> : public target<Ret (Args...)> { };           \
                                                                                \
    template<typename Ret, typename Cls, typename... Args>                      \
    struct name<Ret (Cls::*)(Args...)> : public target<Ret (Args...)> { };      \
                                                                                \
    template<typename Ret, typename Cls, typename... Args>                      \
    struct name<Ret (Cls::*)(Args...) const> : public target<Ret (Args...)> { };
#define RESULT_MAKE_MAPPING(name)                                               \
    template<typename Func>                                                     \
    struct name;                                                                \
                                                                                \
    RESULT_MAKE_MAPPING_INTERNAL(name, name)
#define RESULT_MAP_TO_IMPL(name)                                                \
    template<typename Func>                                                     \
    struct name : public impl::name<decltype(&Func::operator())> { };           \
                                                                                \
    RESULT_MAKE_MAPPING_INTERNAL(name, impl::name)

namespace result {
namespace types {
    template<typename T>
    struct Ok {
        /// only for default constructible T
        template<std::enable_if_t<std::is_default_constructible_v<T>>* = nullptr>
        Ok() { }
        /// only if T can be constructed from Args
        template<typename... Args, std::enable_if_t<std::is_constructible_v<T, Args &&...>>* = nullptr>
        Ok(std::in_place_t, Args &&...args) : val(args...) { } // NOLINT(*-explicit-constructor)

        Ok(const T& val) : val(val) { } // NOLINT(*-explicit-constructor)
        Ok(T&& val) : val(std::move(val)) { } // NOLINT(*-explicit-constructor)

        T val;
    };

    template<>
    struct Ok<void> { };

    template<typename E>
    struct Err {
        Err(const E& val) : val(val) { } // NOLINT(*-explicit-constructor)
        Err(E&& val) : val(std::move(val)) { } // NOLINT(*-explicit-constructor)

        E val;
    };
}

template<typename T, typename E>
class Result;

namespace details {
    template<typename ...>
    struct void_t {
        using type = void;
    };

    template<typename R>
    struct result_ok_type {
        using type = typename std::decay<R>::type;
    };

    template<typename T, typename E>
    struct result_ok_type<Result<T, E>> {
        using type = T;
    };

    template<typename R>
    struct result_error_type {
        using type = R;
    };

    template<typename T, typename E>
    struct result_error_type<Result<T, E>> {
        using type = typename std::remove_reference<E>::type;
    };

    template<typename R>
    struct is_result : public std::false_type { };
    template<typename T, typename E>
    struct is_result<Result<T, E>> : public std::true_type { };

    namespace Ok {
        namespace impl {
            RESULT_MAKE_MAPPING(Transform)

            // non-void callback returning non-void
            template<typename Ret, typename Arg>
            struct Transform<Ret (Arg)> {
                static_assert(!is_result<Ret>::value,
                        "Can not transform using a callback returning a Result, use and_then instead");

                // any result
                template<typename T, typename E, typename Func>
                static Result<Ret, E> transform(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<T, Arg>::value ||
                            std::is_convertible<T, Arg>::value,
                            "Incompatible parameter type detected in transform callback");

                    if (result.has_value()) {
                        auto res = func(result.value());
                        return types::Ok<Ret>(std::move(res));
                    }

                    return types::Err<E>(result.error());
                }
            };

            // callback returning void
            template<typename Arg>
            struct Transform<void (Arg)> {
                // any result
                template<typename T, typename E, typename Func>
                static Result<void, E> transform(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<T, Arg>::value ||
                            std::is_convertible<T, Arg>::value,
                            "Incompatible parameter type detected in transform callback");

                    if (result.has_value()) {
                        func(result.value());
                        return types::Ok<void>();
                    }

                    return types::Err<E>(result.error());
                }
            };

            // void callback
            template<typename Ret>
            struct Transform<Ret (void)> {
                static_assert(!is_result<Ret>::value,
                              "Can not transform using a callback returning a Result, use and_then instead");

                // void result or error
                template<typename T, typename E, typename Func>
                static Result<Ret, E> transform(const Result<T, E>& result, Func func) {
                    static_assert(std::is_same<T, void>::value,
                            "Can not transform using a void callback on a non-void Result");

                    if (result.has_value()) {
                        auto ret = func();
                        return types::Ok<Ret>(std::move(ret));
                    }

                    return types::Err<E>(result.error());
                }
            };

            // void callback returning void
            template<>
            struct Transform<void (void)> {
                // void result or error
                template<typename T, typename E, typename Func>
                static Result<void, E> transform(const Result<T, E>& result, Func func) {
                    static_assert(std::is_same<T, void>::value,
                            "Can not transform using a void callback on a non-void Result");

                    if (result.has_value()) {
                        func();
                        return types::Ok<void>();
                    }

                    return types::Err<E>(result.error());
                }
            };
        } // namespace impl
        RESULT_MAP_TO_IMPL(Transform)

    } // namespace Ok

    namespace Err {
        namespace impl {
            RESULT_MAKE_MAPPING(Transform)

            // any callback
            template<typename Ret, typename Arg>
            struct Transform<Ret (Arg)> {
                static_assert(!is_result<Ret>::value,
                        "Can not transform_error using a callback returning a Result, use or_else instead");

                // non-void result
                template<typename T, typename E, typename Func>
                static Result<T, Ret> transform(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<E, Arg>::value ||
                            std::is_convertible<E, Arg>::value,
                            "Incompatible parameter type detected in transform_error callback");

                    if (!result.has_value()) {
                        auto res = func(result.error());
                        return types::Err<Ret>(res);
                    }

                    return types::Ok<T>(result.value());
                }

                // void result
                template<typename E, typename Func>
                static Result<void, Ret> transform(const Result<void, E>& result, Func func) {
                    static_assert(
                            std::is_same<E, Arg>::value ||
                            std::is_convertible<E, Arg>::value,
                            "Incompatible parameter type detected in transform_error callback");

                    if (!result.has_value()) {
                        auto res = func(result.error());
                        return types::Err<Ret>(res);
                    }

                    return types::Ok<void>();
                }
            };
        } // namespace impl
        RESULT_MAP_TO_IMPL(Transform)

    } // namespace Err

    namespace If {
        namespace impl {
            RESULT_MAKE_MAPPING(Then)

            // any callback
            template<typename Ret, typename Arg>
            struct Then<Ret (Arg)> {
                static_assert(std::is_same<Ret, void>::value,
                        "Callback for if_then should not return anything, use transform instead");

                // any result
                template<typename T, typename E, typename Func>
                static Result<T, E> if_then(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<T, Arg>::value ||
                            std::is_convertible<T, Arg>::value,
                            "Incompatible parameter type detected in if_then callback");

                    if (result.has_value())
                        func(result.value());

                    return result;
                }
            };

            // void callback
            template<typename Ret>
            struct Then<Ret (void)> {
                static_assert(std::is_same<Ret, void>::value,
                        "Callback for if_then should not return anything, use transform instead");

                // void result or error
                template<typename T, typename E, typename Func>
                static Result<T, E> if_then(const Result<T, E>& result, Func func) {
                    static_assert(std::is_same<T, void>::value,
                            "Can not if_then using a void-callback on a non-void Result");

                    if (result.has_value())
                        func();

                    return result;
                }
            };
        } // namespace impl
        RESULT_MAP_TO_IMPL(Then)

    } // namespace If

    namespace Else {
        namespace impl {
            RESULT_MAKE_MAPPING(Then)

            // any callback
            template<typename Ret, typename Arg>
            struct Then<Ret (Arg)> {
                static_assert(std::is_same<Ret, void>::value,
                              "Callback for else_then should not return anything, use transform_error for that");

                // any result
                template<typename T, typename E, typename Func>
                static Result<T, E> else_then(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<E, Arg>::value ||
                            std::is_convertible<E, Arg>::value,
                            "Incompatible parameter type detected in else_then callback");

                    if (!result.has_value())
                        func(result.error());

                    return result;
                }
            };
        } // namespace impl
        RESULT_MAP_TO_IMPL(Then)

    } // namespace Else

    namespace And {
        namespace impl {
            RESULT_MAKE_MAPPING(Then)

            // non-void callback
            template<typename U, typename E, typename Arg>
            struct Then<Result<U, E> (Arg)> {
                // any result
                template<typename T, typename Func>
                static Result<U, E> and_then(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<T, Arg>::value ||
                            std::is_convertible<T, Arg>::value,
                            "Incompatible parameter type detected in and_then callback");

                    if (result.has_value())
                        return func(result.value());

                    return types::Err<E>(result.error());
                }
            };

            // void callback
            template<typename U, typename E>
            struct Then<Result<U, E> (void)> {
                // void result or error
                template<typename T, typename Func>
                static Result<U, E> and_then(const Result<T, E>& result, Func func) {
                    static_assert(std::is_same<T, void>::value,
                                  "Can not and_then using a void-callback on a non-void Result");

                    if (result.has_value())
                        return func();

                    return types::Err<E>(result.error());
                }
            };
        } // namespace impl
        RESULT_MAP_TO_IMPL(Then)

    } // namespace If

    namespace Or {
        namespace impl {
            RESULT_MAKE_MAPPING(Else)

            // any callback
            template<typename T, typename F, typename Arg>
            struct Else<Result<T, F> (Arg)> {
                // non-void result
                template<typename E, typename Func>
                static Result<T, F> or_else(const Result<T, E>& result, Func func) {
                    static_assert(
                            std::is_same<E, Arg>::value ||
                            std::is_convertible<E, Arg>::value,
                            "Incompatible parameter type detected in or_else callback");

                    if (!result.has_value())
                        return func(result.error());

                    return types::Ok<T>(result.value());
                }

                // void result
                template<typename E, typename Func>
                static Result<void, F> or_else(const Result<void, E>& result, Func func) {
                    static_assert(
                            std::is_same<E, Arg>::value ||
                            std::is_convertible<E, Arg>::value,
                            "Incompatible parameter type detected in or_else callback");

                    if (!result.has_value())
                        return func(result.error());

                    return types::Ok<void>();
                }
            };

            // void callback
            template<typename T, typename F>
            struct Else<Result<T, F> (void)> {
                // non-void result
                template<typename E, typename Func>
                static Result<T, F> or_else(const Result<T, E>& result, Func func) {
                    static_assert(std::is_same<T, void>::value,
                            "Can not or_else using a void-callback on a non-void Result");

                    if (!result.has_value())
                        return func();

                    return types::Ok<T>(result.value());
                }

                // void result
                template<typename E, typename Func>
                static Result<void, F> or_else(const Result<void, E>& result, Func func) {
                    if (!result.has_value())
                        return func();

                    return types::Ok<void>();
                }
            };
        } // namespace impl
        RESULT_MAP_TO_IMPL(Else)

    } // namespace Or

    template<typename T, typename E, typename Func,
            typename Ret = Result<
                    typename details::result_ok_type<
                            typename std::invoke_result_t<Func, T>
                    >::type,
                    E
            >
    >
    Ret transform(const Result<T, E> &result, Func func) {
        return Ok::Transform<Func>::transform(result, func);
    }

    template<typename T, typename E, typename Func,
            typename Ret = Result<T,
                    typename details::result_error_type<
                            typename std::invoke_result_t<Func, E>
                    >::type
            >
    >
    Ret transform_error(const Result<T, E>& result, Func func) {
        return Err::Transform<Func>::transform(result, func);
    }

    template<typename T, typename E, typename Func>
    Result<T, E> if_then(const Result<T, E>& result, Func func) {
        return If::Then<Func>::if_then(result, func);
    }

    template<typename T, typename E, typename Func>
    Result<T, E> else_then(const Result<T, E>& result, Func func) {
        return Else::Then<Func>::else_then(result, func);
    }

    template<typename T, typename E, typename Func,
            typename Ret = Result<
                    typename details::result_ok_type<
                            typename std::invoke_result_t<Func, T>
                    >::type,
                    E
            >
    >
    Ret and_then(const Result<T, E>& result, Func func) {
        return And::Then<Func>::and_then(result, func);
    }

    template<typename T, typename E, typename Func,
            typename Ret = Result<T,
                    typename details::result_error_type<
                            typename std::invoke_result_t<Func, E>
                    >::type
            >
    >
    Ret or_else(const Result<T, E>& result, Func func) {
        return Or::Else<Func>::or_else(result, func);
    }

} // namespace details

namespace concepts {
    template<typename T, typename = void> struct EqualityComparable : std::false_type { };

    template<typename T>
    struct EqualityComparable<T,
            typename std::enable_if<
                    true,
                    typename details::void_t<decltype(std::declval<T>() == std::declval<T>())>::type
            >::type
    > : std::true_type { };

} // namespace concepts

template<typename T, typename E>
class [[nodiscard]] Result {
    static_assert(!std::is_same<E, void>::value, "void error type is not allowed");

public:
    using value_type = T;
    using error_type = E;

    /// result with value
    Result(types::Ok<T> &&ok) : has_val_(true), // NOLINT(*-explicit-constructor)
            storage_(std::in_place_index<0>, std::move(ok.val)) {

    }

    /// result with error
    Result(types::Err<E> &&err) : has_val_(false), // NOLINT(*-explicit-constructor)
            storage_(std::in_place_index<1>, std::move(err.val)) {

    }

    // operator->, operator*

    template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
    const T *operator->() const noexcept {
        return &std::get<0>(storage_);
    }
    template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
    T *operator->() noexcept {
        return &std::get<0>(storage_);
    }
    template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
    const T &operator*() const& noexcept {
        return std::get<0>(storage_);
    }
    template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
    T &operator*() & noexcept {
        return std::get<0>(storage_);
    }

    inline operator bool() const noexcept { // NOLINT(*-explicit-constructor)
        return has_value();
    }

    // accessors

    /// true if Result contains value, false otherwise
    [[nodiscard]] bool has_value() const noexcept {
        return has_val_;
    }

    /// returns value if has_value(), throws otherwise
    template<typename U = T>
    [[nodiscard]] const typename std::enable_if_t<!std::is_same_v<U, void>, U> &value() const {
        if (has_value())
            return std::get<0>(storage_);

        throw result_error("Attempting to call value() on an error Result");
    }
    /// no-op if has_value(), throws otherwise
    template<typename U = T>
    typename std::enable_if_t<std::is_same_v<U, void>, U> value() const {
        if (!has_value())
            throw result_error("Attempting to call value() on an error Result");
    }

    /// throws if has_value(), returns error otherwise
    [[nodiscard]] const error_type &error() const {
        if (!has_value())
            return std::get<1>(storage_);

        throw result_error("Attempting to call error() on an ok Result");
    }

    /// returns value if has_value(), returns default_value otherwise
    template<typename U = T>
    [[nodiscard]] const typename std::enable_if_t<!std::is_same_v<U, void>, U> &value_or(const U &default_value) const {
        if (has_value())
            return std::get<0>(storage_);

        return default_value;
    }

    /// returns default_value if has_value(), returns error otherwise
    [[nodiscard]] const error_type &error_or(const error_type &default_value) const {
        if (!has_value())
            return std::get<1>(storage_);

        return default_value;
    }

    // chaining functions

    /// calls `void func(T)` with current value, then returns current result unchanged
    template<typename Func>
    Result<value_type, error_type> if_then(Func func) const {
        return details::if_then(*this, func);
    }
    /// calls `void func(E)` with current error, then returns current result unchanged
    template<typename Func>
    Result<value_type, error_type> else_then(Func func) const {
        return details::else_then(*this, func);
    }

    /// if has_value: calls `\<void, any U> func(T)` with current value, then propagates func result
    /// else: propagates current error
    template<typename Func,
            typename Ret = Result<
                    typename details::result_ok_type<
                            typename std::invoke_result_t<Func, value_type>
                    >::type,
                    error_type
            >
    >
    Ret transform(Func func) const {
        return details::transform(*this, func);
    }
    /// if has_value: propagates current value
    /// else: calls `\<any F> func(E)` with current error, then returns func result
    template<typename Func,
            typename Ret = Result<value_type,
                    typename details::result_error_type<
                            typename std::invoke_result_t<Func, error_type>
                    >::type
            >
    >
    Ret transform_error(Func func) const {
        return details::transform_error(*this, func);
    }

    /// if has_value: calls `Result\<any U, E> func(T)` with current value, then returns func result
    /// else: propagates current error
    template<typename Func,
            typename Ret = Result<
                    typename details::result_ok_type<
                            typename std::invoke_result_t<Func, value_type>
                    >::type,
                    error_type
            >
    >
    Ret and_then(Func func) const {
        return details::and_then(*this, func);
    }
    /// if has_value: propagates current value
    /// else: calls `Result\<T, any F> func(E)` with current error, then returns func result
    template<typename Func,
            typename Ret = Result<value_type,
                    typename details::result_error_type<
                            typename std::invoke_result_t<Func, error_type>
                    >::type
            >
    >
    Ret or_else(Func func) const {
        return details::or_else(*this, func);
    }

private:
    bool has_val_;
    std::variant<T, E> storage_;
};

template<typename T = void, typename CleanT = typename std::decay<T>::type>
inline types::Ok<CleanT> Ok(T &&val) {
    return types::Ok<CleanT>(std::forward<T>(val));
}

inline types::Ok<void> Ok() {
    return { };
}

template<typename E, typename CleanE = typename std::decay<E>::type>
inline types::Err<CleanE> Err(E &&val) {
    return types::Err<CleanE>(std::forward<E>(val));
}

} //namespace result

// @formatter:on
// clang-format on

template<typename T, typename E>
bool operator==(const result::Result<T, E>& lhs, const result::Result<T, E>& rhs) {
    static_assert(result::concepts::EqualityComparable<T>::value,
            "T must be EqualityComparable for Result to be comparable");
    static_assert(result::concepts::EqualityComparable<E>::value,
            "E must be EqualityComparable for Result to be comparable");

    if (lhs.has_value() && rhs.has_value())
        return lhs.value() == rhs.value();
    else if (!lhs.has_value() && !rhs.has_value())
        return lhs.error() == rhs.error();

    return false;
}

template<typename T, typename E>
bool operator==(const result::Result<T, E>& lhs, result::types::Ok<T> ok) {
    static_assert(result::concepts::EqualityComparable<T>::value,
            "T must be EqualityComparable for Result to be comparable");

    if (lhs.has_value())
        return lhs.value() == ok.val;

    return false;
}
template<typename T, typename E>
bool operator==(result::types::Ok<T> ok, const result::Result<T, E>& rhs) {
    return rhs == ok;
}

template<typename E>
bool operator==(const result::Result<void, E>& lhs, result::types::Ok<void>) {
    return lhs.has_value();
}
template<typename E>
bool operator==(result::types::Ok<void> ok, const result::Result<void, E>& rhs) {
    return rhs == ok;
}

template<typename T, typename E>
bool operator==(const result::Result<T, E>& lhs, result::types::Err<E> err) {
    static_assert(result::concepts::EqualityComparable<E>::value,
            "E must be EqualityComparable for Result to be comparable");

    if (!lhs.has_value())
        return lhs.error() == err.val;

    return false;
}
template<typename T, typename E>
bool operator==(result::types::Err<E> err, const result::Result<T, E>& rhs) {
    return rhs == err;
}

#endif //COMMONS_RESULT_H
