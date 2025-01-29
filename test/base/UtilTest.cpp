/*
 * Copyright (C) 2015-2025 The ViaDuck Project
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

#include "UtilTest.h"

#include <commons/util/Str.h>
#include <commons/util/Time.h>

template<typename T>
void assertHelperVectorEq(const std::vector<T> &va, const std::vector<T> &vb) {
    ASSERT_EQ(va.size(), vb.size());

    for (size_t i = 0; i < va.size(); i++)
        ASSERT_EQ(va[i], vb[i]) << " at " << i;
}

#define ASSERT_VECTOR_EQ(...) \
    ASSERT_NO_FATAL_FAILURE(assertHelperVectorEq(__VA_ARGS__))

TEST_F(UtilTest, testStrSplit) {
    ASSERT_VECTOR_EQ({}, Str::splitAll("Hello World", ""));
    ASSERT_VECTOR_EQ({"Hello", "World"}, Str::splitAll("Hello World", " "));
    ASSERT_VECTOR_EQ({"H", "llo World"}, Str::splitAll("Hello World", "e"));

    ASSERT_VECTOR_EQ({"", "", "e", ""}, Str::splitAll("::e:", ":"));
    ASSERT_VECTOR_EQ({"", "", "e:"}, Str::splitAll("::e:", ":", 3));
    ASSERT_VECTOR_EQ({"", ":e:"}, Str::splitAll("::e:", ":", 2));
    ASSERT_VECTOR_EQ({"::e:"}, Str::splitAll("::e:", ":", 1));
}

TEST_F(UtilTest, testStrJoin) {
    ASSERT_EQ("Hello World", Str::joinAll({"Hello", "World"}, " "));
    ASSERT_EQ("Hello World", Str::joinAll({"H", "llo World"}, "e"));

    ASSERT_EQ("::e:", Str::joinAll({"", "", "e", ""}, ":"));
    ASSERT_EQ("::e:", Str::joinAll({"", "", "e:"}, ":"));
    ASSERT_EQ("::e:", Str::joinAll({"", ":e:"}, ":"));
    ASSERT_EQ("::e:", Str::joinAll({"::e:"}, ":"));
}

TEST_F(UtilTest, testTime) {
    EXPECT_EQ("2018-02-03T21:55:13.160Z", Time(1517694913160).formatIso8601());
    EXPECT_EQ("6 02.03.2018 21:55:13", Time(1517694913160).format("%w %m.%d.%Y %H:%M:%S"));
    EXPECT_EQ("6 02.03.2018 21:55:13", Time(1517694913160).formatFull("%w %m.%d.%Y %H:%M:%S"));
    EXPECT_EQ("6 02.03.2018 21:55:13.160", Time(1517694913160).formatFull("%w %m.%d.%Y %H:%M:%S.%k"));
    EXPECT_EQ("6 02.03.2018 21:55:13.001", Time(1517694913001).formatFull("%w %m.%d.%Y %H:%M:%S.%k"));
    EXPECT_EQ("6 02.03.2018 21:55:13.001+0000", Time(1517694913001).formatFull("%w %m.%d.%Y %H:%M:%S.%k%z"));
}
