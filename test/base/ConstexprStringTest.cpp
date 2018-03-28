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

#include "ConstexprStringTest.h"

#include <commons/ConstexprString.h>

TEST_F(ConstexprStringTest, Trivial) {
    ConstexprString<3> s("123");

    EXPECT_EQ(3u, s.size());
    EXPECT_STREQ("123", s.c_str());
}

TEST_F(ConstexprStringTest, InferArguments) {
    auto s = MakeConstexprString("123");

    EXPECT_EQ(3u, s.size());
    EXPECT_STREQ("123", s.c_str());
}

TEST_F(ConstexprStringTest, Append) {
    ConstexprString<3> s1("123");
    ConstexprString<2> s2("45");

    ConstexprString<5> s3 = s1 + s2;

    EXPECT_EQ(5u, s3.size());
    EXPECT_STREQ("12345", s3.c_str());
}
