/*
 * Copyright (C) 2015-2024 The ViaDuck Project
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

#ifndef COMMONS_EXCEPT_H
#define COMMONS_EXCEPT_H

#include <commons/log/Log.h>

// defines an exception class "name"_error with full qualified base "fqbase"
#define DEFINE_ERROR_FQ(name, base, fqbase)                             \
class name##_error : public fqbase {                                    \
public:                                                                 \
    explicit name##_error(const char* __arg) : base(__arg) {}           \
    explicit name##_error(const std::string &__arg) : base(__arg) {}    \
}

// defines an exception class "name"_error with base class "base"
#define DEFINE_ERROR(name, base) DEFINE_ERROR_FQ(name, base, base)

// define base_error as base for all other errors
DEFINE_ERROR_FQ(base, runtime_error, std::runtime_error);

// workaround to convert line to string literal
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define VD_LINE STRINGIZE(__LINE__)

// report a message unconditionally to loglevel, execute rt
#define L_report_message_ex(message, rt, loglevel)           \
    do {                                                     \
        std::stringstream _message;                          \
        _message << message << " in " __FILE__ ":" VD_LINE;  \
                                                             \
        if (Except::reporting()) {                           \
            Log::loglevel << _message.str();                 \
        }                                                    \
                                                             \
        rt;                                                  \
    } while(false)
#define L_report_error(message, error) \
    L_report_message_ex(message, throw error(_message.str()), err)

// checks a condition, logs message to loglevel and executes rt on error
#define L_check_internal_ex(condition, message, rt, loglevel)    \
    do {                                                         \
        if (!(condition)) {                                      \
            L_report_message_ex(message, rt, loglevel);          \
        }                                                        \
    } while(false)
#define L_assert_internal(condition, message, error, loglevel) \
    L_check_internal_ex(condition, message, throw error(_message.str()), loglevel)
#define L_expect_internal(condition, message, loglevel) \
    L_check_internal_ex(condition, message, ,loglevel)
#define L_assert_ex(condition, message, error) \
    L_assert_internal(condition, message, error, err)
#define L_expect_ex(condition, message) \
    L_expect_internal(condition, message, err)
#define L_assert(condition, error) \
    L_assert_ex(condition, "Assert failed: \"" #condition "\"", error)
#define L_expect(condition) \
    L_expect_ex(condition, "Expect failed: \"" #condition "\"")

// checks (a op b) using single evaluation, logs message to loglevel and executes rt on error
#define L_check_op_internal(a, b, op, message, rt, loglevel) \
    do {                                            \
        auto _a = (a);                              \
        auto _b = (b);                              \
        L_check_internal_ex(_a op _b, message, rt, loglevel);\
    } while(false)
#define L_assert_op_internal(a, b, op, error, loglevel) \
    L_check_op_internal(a, b, op, "Assert failed: (" #a " " #op " " #b ")," \
        " got " << _a << " vs " << _b << " instead", throw error(_message.str()), loglevel)
#define L_assert_eq(a, b, error) \
    L_assert_op_internal(a, b, ==, error, err)
#define L_assert_ne(a, b, error) \
    L_assert_op_internal(a, b, !=, error, err)

// thread-local management of error reporting
class Except {
public:
    static bool enableReporting() {
        bool oldReporting = mReporting;
        mReporting = true;

        return oldReporting;
    }
    static bool disableReporting() {
        bool oldReporting = mReporting;
        mReporting = false;

        return oldReporting;
    }
    static bool toggleReporting() {
        bool oldReporting = mReporting;
        mReporting = !mReporting;

        return oldReporting;
    }

    static bool reporting() { return mReporting; }
    static void setReporting(bool value) { mReporting = value; }

protected:
    thread_local static bool mReporting;
};
inline thread_local bool Except::mReporting = true;

class ScopedEnableReporting {
public:
    ScopedEnableReporting() : mReporting(Except::enableReporting()) { }
    ~ScopedEnableReporting() { Except::setReporting(mReporting); }

protected:
    bool mReporting;
};
class ScopedDisableReporting {
public:
    ScopedDisableReporting() : mReporting(Except::disableReporting()) { }
    ~ScopedDisableReporting() { Except::setReporting(mReporting); }

protected:
    bool mReporting;
};

#endif //COMMONS_EXCEPT_H
