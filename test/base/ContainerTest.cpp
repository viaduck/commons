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

#include <flatbuffers/test/VarMsg.h>
#include <flatbuffers/test/EnumMsg.h>
#include <flatbuffers/test/sometest.h>

#include "ContainerTest.h"
#include "custom_assert.h"

TEST_F(ContainerTest, SimpleWrite) {
    sometest c, d;

    c.version(0xBEEFDEAD);
    EXPECT_EQ(0xBEEFDEAD, c.version());
    c.first(0xfe);
    EXPECT_EQ(0xfe, c.first());
    c.second(0xDEAD);
    EXPECT_EQ(0xDEAD, c.second());
    c.buf().write("\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", 10, 0);
    EXPECT_EQ(10, c.buf().size());
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.buf().const_data(), 10);

    // serialize and deserialize
    Buffer test;
    c.serialize(test);
    ASSERT_TRUE(d.deserialize(test));

    // test deserialized buffer
    EXPECT_EQ(0xBEEFDEAD, d.version());
    EXPECT_EQ(0xfe, d.first());
    EXPECT_EQ(0xDEAD, d.second());
    EXPECT_EQ(10, d.buf().size());
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", d.buf().const_data(), 10);
}

TEST_F(ContainerTest, BitField) {
    VarMsg msg;
    TestField field = msg.testField();

    EXPECT_EQ(15u, msg.testField().squeezed_one_width());
    EXPECT_EQ(12u, msg.testField().squeezed_two_width());

    EXPECT_EQ(0u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());

    field.squeezed_one(1337);
    msg.testField(field);

    EXPECT_EQ(1337u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());

    field.squeezed_two(15);
    msg.testField(field);

    EXPECT_EQ(1337u, msg.testField().squeezed_one());
    EXPECT_EQ(15u, msg.testField().squeezed_two());

    field.squeezed_two(0);
    msg.testField(field);

    EXPECT_EQ(1337u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());

    field.squeezed_one(987);
    msg.testField(field);

    EXPECT_EQ(987u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());
}

TEST_F(ContainerTest, Serialize) {
    VarMsg msg, omsg;
    msg.this_is_a_cool_property(123);
    msg.bufVar().append("abc", 3);
    msg.bufVar2().append("defgh", 5);

    Buffer out;
    msg.serialize(out);
    ASSERT_TRUE(omsg.deserialize(out));

    EXPECT_EQ(123, omsg.this_is_a_cool_property());
    EXPECT_EQ(3, omsg.bufVar().size());
    EXPECT_ARRAY_EQ(const uint8_t, "abc", omsg.bufVar().const_data(), 3);
    EXPECT_EQ(5, omsg.bufVar2().size());
    EXPECT_ARRAY_EQ(const uint8_t, "defgh", omsg.bufVar2().const_data(), 5);
}

TEST_F(ContainerTest, Bit) {
    TestField field;

    // put squeeze values, expect bitfield value
    field.squeezed_one(123);
    EXPECT_EQ(123, field.squeezed_one());
    field.squeezed_two(3);
    EXPECT_EQ(3, field.squeezed_two());
    EXPECT_EQ(0x1807Bu, field.value());

    // put bitfield value, expect squeeze values
    field.value(0x123412);
    EXPECT_EQ(0x3412, field.squeezed_one());
    EXPECT_EQ(0x24, field.squeezed_two());
}

TEST_F(ContainerTest, Enum) {
    EnumMsg msg;
    msg.myTestEnum(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY);
    EXPECT_EQ(TestEnum::NO_STRICT_NAMING_ONLY_CPP_LIMITATIONS_APPLY, msg.myTestEnum());

    msg.myTestEnum(TestEnum::INVALID_ENUM_VALUE);
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, msg.myTestEnum());

    const int val = static_cast<uint16_t>(TestEnum::INVALID_ENUM_VALUE)+2;
    msg.myTestEnum(val);
    EXPECT_EQ(TestEnum::INVALID_ENUM_VALUE, msg.myTestEnum());

    msg.myTestEnum(static_cast<uint16_t>(TestEnum::VALUE_X55));
    EXPECT_EQ(TestEnum::VALUE_X55, msg.myTestEnum());
}
