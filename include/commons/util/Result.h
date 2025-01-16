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

#include "Except.h"

DEFINE_ERROR(result, base_error);

namespace types {
    template<typename T>
    struct Ok {
        /// only for default constructible T
        template<std::enable_if_t<std::is_default_constructible_v<T>>* = nullptr>
        explicit Ok() { }
        /// only if T can be constructed from Args
        template<typename... Args, std::enable_if_t<std::is_constructible_v<T, Args &&...>>* = nullptr>
        explicit Ok(std::in_place_t, Args &&...args) : val(args...) { }

        /// copy ctor
        explicit Ok(const T& val) : val(val) { }
        /// move ctor
        explicit Ok(T&& val) : val(std::move(val)) { }

        T val;
    };

    template<>
    struct Ok<void> { };

    template<typename E>
    struct Err {
        explicit Err(const E& val) : val(val) { }
        explicit Err(E&& val) : val(std::move(val)) { }

        E val;
    };
}

namespace result {
    template<typename T, typename E>
    class Result;
}

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

namespace details {
    using namespace result;

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

    struct ok_tag { };
    struct err_tag { };

    template<typename T, typename E>
    struct Storage {
        static constexpr size_t Size = sizeof(T) > sizeof(E) ? sizeof(T) : sizeof(E);
        static constexpr size_t Align = sizeof(T) > sizeof(E) ? alignof(T) : alignof(E);

        using type = typename std::aligned_storage<Size, Align>::type;

        Storage() : initialized_(false) { }

        void construct(types::Ok<T> ok) {
            new (&storage_) T(ok.val);
            initialized_ = true;
        }

        void construct(types::Err<E> err) {
            new (&storage_) E(err.val);
            initialized_ = true;
        }

        template<typename U>
        void rawConstruct(U&& val) {
            using CleanU = typename std::decay<U>::type;

            new (&storage_) CleanU(std::forward<U>(val));
            initialized_ = true;
        }

        template<typename U>
        const U& get() const& {
            return *reinterpret_cast<const U *>(&storage_);
        }
        template<typename U>
        U& get() {
            return *reinterpret_cast<U *>(&storage_);
        }

        void destroy(ok_tag) {
            if (initialized_) {
                get<T>().~T();
                initialized_ = false;
            }
        }

        void destroy(err_tag) {
            if (initialized_) {
                get<E>().~E();
                initialized_ = false;
            }
        }

        type storage_;
        bool initialized_;
    };

    template<typename E>
    struct Storage<void, E> {
        using type = typename std::aligned_storage<sizeof(E), alignof(E)>::type;

        void construct(types::Ok<void>) {
            initialized_ = true;
        }

        void construct(types::Err<E> err) {
            new (&storage_) E(err.val);
            initialized_ = true;
        }

        template<typename U>
        void rawConstruct(U&& val) {
            using CleanU = typename std::decay<U>::type;

            new (&storage_) CleanU(std::forward<U>(val));
            initialized_ = true;
        }

        void destroy(ok_tag) { initialized_ = false; }
        void destroy(err_tag) {
            if (initialized_) {
                get<E>().~E();
                initialized_ = false;
            }
        }

        template<typename U>
        const U& get() const {
            return *reinterpret_cast<const U *>(&storage_);
        }

        template<typename U>
        U& get() {
            return *reinterpret_cast<U *>(&storage_);
        }

        type storage_;
        bool initialized_;
    };

    template<typename T, typename E>
    struct Constructor {
        static void move(Storage<T, E>&& src, Storage<T, E>& dst, ok_tag) {
            dst.rawConstruct(std::move(src.template get<T>()));
            src.destroy(ok_tag());
        }

        static void copy(const Storage<T, E>& src, Storage<T, E>& dst, ok_tag) {
            dst.rawConstruct(src.template get<T>());
        }

        static void move(Storage<T, E>&& src, Storage<T, E>& dst, err_tag) {
            dst.rawConstruct(std::move(src.template get<E>()));
            src.destroy(err_tag());
        }

        static void copy(const Storage<T, E>& src, Storage<T, E>& dst, err_tag) {
            dst.rawConstruct(src.template get<E>());
        }
    };

    template<typename E>
    struct Constructor<void, E> {
        static void move(Storage<void, E>&&, Storage<void, E>&, ok_tag) { }

        static void copy(const Storage<void, E>&, Storage<void, E>&, ok_tag) { }

        static void move(Storage<void, E>&& src, Storage<void, E>& dst, err_tag) {
            dst.rawConstruct(std::move(src.template get<E>()));
            src.destroy(err_tag());
        }

        static void copy(const Storage<void, E>& src, Storage<void, E>& dst, err_tag) {
            dst.rawConstruct(src.template get<E>());
        }
    };

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

namespace result {
    template<typename T, typename E>
    class [[nodiscard]] Result {
        static_assert(!std::is_same<E, void>::value, "void error type is not allowed");

        using storage_type = details::Storage<T, E>;
        using constructor_type = details::Constructor<T, E>;
    public:

        using value_type = T;
        using error_type = E;

        /// result with value
        Result(types::Ok<T> ok) : has_val_(true) { // NOLINT(*-explicit-constructor)
            storage_.construct(std::move(ok));
        }

        /// result with error
        Result(types::Err<E> err) : has_val_(false) { // NOLINT(*-explicit-constructor)
            storage_.construct(std::move(err));
        }

        /// copy result
        Result(const Result &other) {
            if (other.has_val_) {
                constructor_type::copy(other.storage_, storage_, details::ok_tag());
                has_val_ = true;
            } else {
                constructor_type::copy(other.storage_, storage_, details::err_tag());
                has_val_ = false;
            }
        }

        /// move result
        Result(Result &&other) noexcept {
            if (other.has_val_) {
                constructor_type::move(std::move(other.storage_), storage_, details::ok_tag());
                has_val_ = true;
            } else {
                constructor_type::move(std::move(other.storage_), storage_, details::err_tag());
                has_val_ = false;
            }
        }

        ~Result() {
            if (has_val_)
                storage_.destroy(details::ok_tag());
            else
                storage_.destroy(details::err_tag());
        }

        // operator->, operator*

        template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
        const T *operator->() const noexcept {
            return &storage_.template get<T>();
        }
        template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
        T *operator->() noexcept {
            return &storage_.template get<T>();
        }
        template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
        const T &operator*() const& noexcept {
            return storage_.template get<T>();
        }
        template<typename U = T, std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
        T &operator*() & noexcept {
            return storage_.template get<T>();
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
        const typename std::enable_if_t<!std::is_same_v<U, void>, U> &value() const {
            if (has_value())
                return storage_.template get<U>();

            throw result_error("Attempting to call value() on an error Result");
        }
        /// no-op if has_value(), throws otherwise
        template<typename U = T>
        typename std::enable_if_t<std::is_same_v<U, void>, U> value() const {
            if (!has_value())
                throw result_error("Attempting to call value() on an error Result");
        }

        /// throws if has_value(), returns error otherwise
        const error_type &error() const {
            if (!has_value())
                return storage_.template get<error_type>();

            throw result_error("Attempting to call error() on an ok Result");
        }

        /// returns value if has_value(), returns default_value otherwise
        template<typename U = T>
        const typename std::enable_if_t<!std::is_same_v<U, void>, U> &value_or(const U &default_value) const {
            if (has_value())
                return storage_.template get<U>();

            return default_value;
        }

        /// returns default_value if has_value(), returns error otherwise
        const error_type &error_or(const error_type &default_value) const {
            if (!has_value())
                return storage_.template get<error_type>();

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

        /// if has_value: calls `<void, any U> func(T)` with current value, then propagates func result
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
        /// else: calls `<any F> func(E)` with current error, then returns func result
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

        /// if has_value: calls `Result<any U, E> func(T)` with current value, then returns func result
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
        /// else: calls `Result<T, any F> func(E)` with current error, then returns func result
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
        storage_type storage_;
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

template<typename T, typename E>
bool operator==(const result::Result<T, E>& lhs, const result::Result<T, E>& rhs) {
    static_assert(concepts::EqualityComparable<T>::value, "T must be EqualityComparable for Result to be comparable");
    static_assert(concepts::EqualityComparable<E>::value, "E must be EqualityComparable for Result to be comparable");

    if (lhs.has_value() && rhs.has_value())
        return lhs.value() == rhs.value();
    else if (!lhs.has_value() && !rhs.has_value())
        return lhs.error() == rhs.error();

    return false;
}

template<typename T, typename E>
bool operator==(const result::Result<T, E>& lhs, types::Ok<T> ok) {
    static_assert(concepts::EqualityComparable<T>::value, "T must be EqualityComparable for Result to be comparable");

    if (lhs.has_value())
        return lhs.value() == ok.val;

    return false;
}

template<typename E>
bool operator==(const result::Result<void, E>& lhs, types::Ok<void>) {
    return lhs.has_value();
}

template<typename T, typename E>
bool operator==(const result::Result<T, E>& lhs, types::Err<E> err) {
    static_assert(concepts::EqualityComparable<E>::value, "E must be EqualityComparable for Result to be comparable");

    if (!lhs.has_value())
        return lhs.error() == err.val;

    return false;
}

#endif //COMMONS_RESULT_H
