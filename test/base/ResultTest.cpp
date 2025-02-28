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

#include "ResultTest.h"

#include <commons/util/Result.h>

template<typename T, typename E>
void subtestResultAccessors(const T &v1, const T& v2, const E &f1, const E &f2) {
    result::Result<T, E> r1 = result::Ok(v1);
    result::Result<T, E> e1 = result::Err(f1);
    result::Result<T, E> r2 = r1;

    ASSERT_TRUE(r1);
    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r1 == r2);
    ASSERT_TRUE(r1 == result::Ok(v1));
    ASSERT_EQ(v1, r1.value());
    ASSERT_EQ(v1, *r2);
    ASSERT_EQ(v1, *(const decltype(r2) &)r2);
    ASSERT_EQ(v1, r2.value_or(v2));
    ASSERT_EQ(f2, r2.error_or(f2));

    ASSERT_FALSE(e1);
    ASSERT_FALSE(e1.has_value());
    ASSERT_EQ(f1, e1.error());
    ASSERT_EQ(f1, e1.error_or(f2));
    ASSERT_EQ(v2, e1.value_or(v2));
}

TEST_F(ResultTest, testResultAccessors) {
    // value test
    ASSERT_NO_FATAL_FAILURE(subtestResultAccessors(1, 42, 'q', 'r'));

    // reference test
    std::vector<int> ra{1, 2, 3}, rb{0, 5, 2};
    std::string rp = "asdf", rq = "qlewas";
    ASSERT_NO_FATAL_FAILURE(subtestResultAccessors(ra, rb, rp, rq));

    // pointer test
    int pa = 1, pb = 42;
    char pp = 'p', pq = 'q';
    ASSERT_NO_FATAL_FAILURE(subtestResultAccessors(&pa, &pb, &pp, &pq));

    // "->" and "bool" operator
    result::Result<std::vector<int>, char> tr = result::Ok(ra);
    ASSERT_TRUE(tr);
    ASSERT_FALSE(tr->empty());
    ASSERT_EQ(3, ((const decltype(tr) &) tr)->size());
}

static std::string test_transform_function(char) {
    return "pq";
}

template<typename T, typename E>
void subtestResultChaining(const T &v1, const T& v2, const E &f1, const E &f2) {
    result::Result<T, E> r1 = result::Ok(v1);
    result::Result<T, E> e1 = result::Err(f1);

    std::function<T(std::string)> valueTransform = [=] (const std::string &) { return v2; };
    std::function<E(std::string)> errorTransform = [=] (const std::string &) { return f2; };

    // if_then should be called on value
    bool called = false;
    ASSERT_EQ(v1, r1.if_then([&] (const T&) { called = true; })
            .value());
    ASSERT_TRUE(called);

    // if_then should not be called on error
    called = false;
    ASSERT_EQ(f1, e1.if_then([&] (const T&) { called = true; })
            .error());
    ASSERT_FALSE(called);

    // else_then should not be called on value
    called = false;
    ASSERT_EQ(v1, r1.else_then([&] (const E&) { called = true; })
            .value());
    ASSERT_FALSE(called);

    // else_then should be called on error
    called = false;
    ASSERT_EQ(f1, e1.else_then([&] (const E&) { called = true; })
            .error());
    ASSERT_TRUE(called);

    // transform r1 value via callbacks
    ASSERT_EQ(v2, r1.transform([] (const T&) { return "p"; })
            // test anonymous lambda
            .transform([] (const std::string &) { return 'q'; })
            // test free function
            .transform(&test_transform_function)
            // test std::function
            .transform(valueTransform)
            .value());

    // transform should be a no-op on e1
    ASSERT_EQ(f1, e1.transform([] (const T&) { return "p"; })
            .error());

    // transform_error e1 error via callbacks
    ASSERT_EQ(f2, e1.transform_error([] (const E&) { return 1; })
            // test anonymous lambda
            .transform_error([] (int) { return 'q'; })
            // test free function
            .transform_error(&test_transform_function)
            // test std::function
            .transform_error(errorTransform)
            .error());

    // transform_error should be a no-op on r1
    ASSERT_EQ(v1, r1.transform_error([] (const E&) { return "p"; })
            .value());

    // and_then switch entire r1 result
    ASSERT_EQ('p', r1.and_then([] (const T&) { return result::Result<char, E>(result::Ok('p')); })
            .value());
    ASSERT_EQ(f2, r1.and_then([=] (const T&) { return result::Result<char, E>(result::Err(f2)); })
            .error());

    // and_then should be a no-op on e1
    ASSERT_EQ(f1, e1.and_then([] (const T&) { return result::Result<char, E>(result::Ok('o')); })
            .error());

    // or_else should be a no-op on r1
    ASSERT_EQ(v1, r1.or_else([] (const E&) { return result::Result<T, bool>(result::Err(false)); })
            .value());

    // or_else switch entire e1 result
    ASSERT_EQ(v2, e1.or_else([=] (const E&) { return result::Result<T, char>(result::Ok(v2)); })
            .value());
    ASSERT_EQ(false, e1.or_else([] (const E&) { return result::Result<T, bool>(result::Err(false)); })
            .error());
}

TEST_F(ResultTest, testResultChaining) {
    // value test
    ASSERT_NO_FATAL_FAILURE(subtestResultChaining(1, 42, 'q', 'r'));

    // reference test
    std::vector<int> ra{1, 2, 3}, rb{0, 5, 2};
    std::string rp = "asdf", rq = "qlewas";
    ASSERT_NO_FATAL_FAILURE(subtestResultChaining(ra, rb, rp, rq));

    // pointer test
    int pa = 1, pb = 42;
    char pp = 'p', pq = 'q';
    ASSERT_NO_FATAL_FAILURE(subtestResultChaining(&pa, &pb, &pp, &pq));
}
