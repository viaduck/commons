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

#include <enum/logger/LogLevel.h>
#include <protocol/test/VarMsg.h>
#include <protocol/test/EnumMsg.h>
#include <protocol/test/sometest.h>
#include <bit/test/TestField.h>

#include <secure_memory/conversions.h>
#include <secure_memory/Buffer.h>

#include "ContainerTest.h"
#include "custom_assert.h"

TEST_F(ContainerTest, SimpleRead) {
    Buffer a(20);
    const uint32_t firstInt = hton(0xBEEFDEAD);
    const uint8_t secondInt = 0xfe;
    const uint16_t third = hton<uint16_t>(0xDEAD);
    const uint8_t arr[] = { 'a', 'b', 'c' };

    a.append(&firstInt, sizeof(firstInt));
    a.append(&secondInt, sizeof(secondInt));
    a.append(&third, sizeof(third));
    a.append(arr, 3);
    a.append("\xAB\xAB\xAB\xAB\xAB\xAB\xAB", 7);

    ASSERT_EQ(sizeof(firstInt)+sizeof(secondInt)+sizeof(third)+10, a.size());

    sometest c(a);
    ASSERT_EQ(c.version(), 0xBEEFDEAD);
    ASSERT_EQ(c.first(), 0xfe);
    ASSERT_EQ(c.second(), 0xDEAD);
    EXPECT_ARRAY_EQ(const uint8_t, "abc", c.buf(), 3);
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.buf()+3, 7);
}

TEST_F(ContainerTest, SimpleWrite) {
    Buffer a(25);

    sometest c(a);

    c.version(0xBEEFDEAD);
    ASSERT_EQ(c.version(), 0xBEEFDEAD);
    c.first(0xfe);
    ASSERT_EQ(c.first(), 0xfe);
    c.second(0xDEAD);
    ASSERT_EQ(c.second(), 0xDEAD);
    c.buf(reinterpret_cast<const uint8_t*>("\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB"), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.buf(), 10);

    ASSERT_EQ(hton(0xBEEFDEAD), *reinterpret_cast<const uint32_t*>(c.buffer().const_data()));
    ASSERT_EQ(0xfe, *static_cast<const uint8_t*>(c.buffer().const_data(4)));
    uint16_t bla = hton<uint16_t>(0xDEAD);
    ASSERT_EQ(bla, *reinterpret_cast<const uint16_t*>(c.buffer().const_data(5)));
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.buffer().const_data(7), 10);      // no need to respect endianess, since it's a byte-sequence
}

TEST_F(ContainerTest, BitField) {
    VarMsg msg;

    EXPECT_EQ(15u, msg.testField().squeezed_one_width());
    EXPECT_EQ(12u, msg.testField().squeezed_two_width());

    EXPECT_EQ(0u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());

    msg.testField().squeezed_one(1337);
    EXPECT_EQ(1337u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());

    msg.testField().squeezed_two(15);
    EXPECT_EQ(1337u, msg.testField().squeezed_one());
    EXPECT_EQ(15u, msg.testField().squeezed_two());

    msg.testField().squeezed_two(0);
    EXPECT_EQ(1337u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());

    msg.testField().squeezed_one(987);
    EXPECT_EQ(987u, msg.testField().squeezed_one());
    EXPECT_EQ(0u, msg.testField().squeezed_two());
}

TEST_F(ContainerTest, Serialize) {
    VarMsg msg;
    msg.this_is_a_cool_property(123);
    msg.bufVar().append("abc", 3);
    msg.bufVar2().append("defgh", 5);

    Buffer out;
    ASSERT_TRUE(msg.serialize(out));

    EXPECT_ARRAY_EQ(const uint8_t,
            "\x00\x00\x00\x00"       // squeezed values
            "\x00\x00\x00\x00"       // version
            "\x00\x00\x00"           // first, second
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"       // buf
            "\x00\x00"               // third
            "\x00\x00\x00\x7b"             // this_is_a_cool_property with "123" as "7b"
            "\x03"      // bufVar - size indicator
            "abc"       // bufVar
            "\x05"      // bufVar2 - size indicator
            "defgh"     // bufVar2
            "\x00\x00"  // bufVarMedium - size indicator
            "\x00\x00\x00\x00" // bufVarBig - size indicator
    , out.const_data(), out.size());
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
