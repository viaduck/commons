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

#include <enum/test/TestEnum.h>

#include "EnumTest.h"

TEST_F(EnumTest, ToString) {
    EXPECT_STREQ("TestEnum::VALUE_1", toString(TestEnum::VALUE_1).c_str());
    EXPECT_STREQ("TestEnum::VALUE_3", toString(TestEnum::VALUE_3).c_str());
    EXPECT_STREQ("TestEnum::VALUE_BLA", toString(TestEnum::VALUE_BLA).c_str());
    EXPECT_STREQ("TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY", toString(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY).c_str());
    EXPECT_STREQ("TestEnum::INVALID_ENUM_VALUE", toString(TestEnum::INVALID_ENUM_VALUE).c_str());

    ASSERT_TRUE(true) << TestEnum::VALUE_BLA;       // compilation test for ostream
}

TEST_F(EnumTest, ToInt) {
    EXPECT_EQ(0u, toInt(TestEnum::VALUE_1));
    EXPECT_EQ(1u, toInt(TestEnum::VALUE_3));
    EXPECT_EQ(2u, toInt(TestEnum::VALUE_BLA));
    EXPECT_EQ(3u, toInt(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY));
    EXPECT_EQ(260u, toInt(TestEnum::INVALID_ENUM_VALUE));
}

TEST_F(EnumTest, FromInt) {
    EXPECT_EQ(TestEnum::VALUE_1, toTestEnum(0));
    EXPECT_EQ(TestEnum::VALUE_3, toTestEnum(1));
    EXPECT_EQ(TestEnum::VALUE_BLA, toTestEnum(2));
    EXPECT_EQ(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY, toTestEnum(3));
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, toTestEnum(260));

    // invalid enum values
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, toTestEnum(265));
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, toTestEnum(275));
}
