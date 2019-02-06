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

#ifndef COMMONS_EXCEPT_H
#define COMMONS_EXCEPT_H

#include <commons/log/Log.h>

// defines a exception class "name"_error with full qualified base "fqbase"
#define DEFINE_ERROR_FQ(name, base, fqbase)                         \
class name##_error : public fqbase {                                    \
public:                                                                 \
    explicit name##_error(const char* __arg) : base(__arg) {}           \
    explicit name##_error(const std::string &__arg) : base(__arg) {}    \
};

// defines a exception class "name"_error with base class "base"
#define DEFINE_ERROR(name, base) DEFINE_ERROR_FQ(name, base, base)

// define base_error as base for all other errors
DEFINE_ERROR_FQ(base, runtime_error, std::runtime_error)

// workaround to convert line to string literal
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define __VD_LINE__ STRINGIZE(__LINE__)

// asserts a condition, throws err on fail
#define L_assert(condition, error)    \
    do {                              \
        if (!(condition)) {           \
            Log::err << "Assert failed: \"" #condition "\" in " __FILE__ ":" __VD_LINE__;  \
            throw error("Assert failed: \"" #condition "\" in " __FILE__ ":" __VD_LINE__); \
        }                             \
    } while(false)

#define L_assert_eq(e, a, error)      \
    do {                              \
        if (!(e == a)) {              \
            Log::err << "Assert failed: expected " << (e) << ", got " << (a) << " in " __FILE__ ":" __VD_LINE__; \
            throw error("Assert failed: in " __FILE__ ":" __VD_LINE__);                                          \
        }                             \
    } while(false)

// expects a condition, logs errors
#define L_expect(condition)           \
    do {                              \
        if (!(condition))             \
            Log::err << "Assert failed: \"" #condition "\" in " __FILE__ ":" __VD_LINE__; \
    } while(false)

#endif //COMMONS_EXCEPT_H
