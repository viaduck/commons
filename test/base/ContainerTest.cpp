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
#include <flatbuffers/test/TestLegacy.h>
#include <flatbuffers/test/TestFuture.h>

#include "ContainerTest.h"
#include "custom_assert.h"

void generate(sometest &test) {
    test.version(0xBEEFDEAD);
    test.first(0xfe);
    test.second(0xDEAD);
    test.buf().write("\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", 10, 0);
    test.third(0x171);
    test.this_is_a_cool_property(0x1337);
}

void verify(sometest &test) {
    ASSERT_EQ(0xBEEFDEAD, test.version());
    ASSERT_EQ(0xfe, test.first());
    ASSERT_EQ(0xDEAD, test.second());
    ASSERT_EQ(10u, test.buf().size());
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", test.buf().const_data(), 10);
    ASSERT_EQ(0x171, test.third());
    ASSERT_EQ(0x1337u, test.this_is_a_cool_property());
}

void verify_reset(sometest &test) {
    ASSERT_EQ(0u, test.version());
    ASSERT_EQ(0u, test.first());
    ASSERT_EQ(0u, test.second());
    ASSERT_EQ(0u, test.buf().size());
    ASSERT_EQ(0u, test.third());
    ASSERT_EQ(0u, test.this_is_a_cool_property());
}

TEST_F(ContainerTest, SimpleWrite) {
    sometest c, d;
    generate(c);
    verify(c);

    // serialize and deserialize
    Buffer test;
    c.serialize(test);
    ASSERT_TRUE(d.deserialize(test));
    verify(d);
}

TEST_F(ContainerTest, Malformed) {
    sometest test;
    generate(test);

    Buffer out;
    test.serialize(out);

    // manipulate size indicator to be bigger than actual buffer
    uint32_t size1 = flatbuffers::GetPrefixedSize(static_cast<const uint8_t *>(out.const_data())) + 5;
    out.write(&size1, sizeof(uint32_t), 0);
    ASSERT_FALSE(test.deserialize(out));
    verify_reset(test);

    // append garbage to buffer, now size indicator fits buffer
    out.append("asdfg", 5);
    ASSERT_TRUE(test.deserialize(out));
    verify(test);

    // garbage in vtable
    out.write("asdfg", 5, 4);
    ASSERT_FALSE(test.deserialize(out));
    verify_reset(test);

    // serialize valid buffer bigger than max_size
    out.clear();
    for (uint32_t i = 0; i < 200; i++)
        test.buf().append("asdfghjkll", 10);
    test.serialize(out);

    // buffer bigger than max_size should not deserialize
    ASSERT_GE(test.buf().size(), 2000u);
    ASSERT_FALSE(test.deserialize(out));
    verify_reset(test);

    // serialize zero byte buffer
    Buffer out2(0);
    uint32_t size = 0;
    out2.write(&size, sizeof(uint32_t), 0);
    ASSERT_FALSE(test.deserialize(out2));
    verify_reset(test);
}

TEST_F(ContainerTest, Serialize) {
    VarMsg msg, omsg;
    msg.this_is_a_cool_property(123);
    msg.bufVar().append("abc", 3);
    msg.bufVar2().append("defgh", 5);
    msg.bufFixed().append("12345678901", 11);
    msg.some_vector({1, 2, 3, 4});

    Buffer out;
    msg.serialize(out);
    ASSERT_TRUE(omsg.deserialize(out));

    EXPECT_EQ(123u, omsg.this_is_a_cool_property());
    EXPECT_EQ(3u, omsg.bufVar().size());
    EXPECT_ARRAY_EQ(const uint8_t, "abc", omsg.bufVar().const_data(), 3);
    EXPECT_EQ(5u, omsg.bufVar2().size());
    EXPECT_ARRAY_EQ(const uint8_t, "defgh", omsg.bufVar2().const_data(), 5);
    EXPECT_EQ(4u, omsg.some_vector().size());
    EXPECT_EQ(1u, omsg.some_vector()[0]);
    EXPECT_EQ(2u, omsg.some_vector()[1]);
    EXPECT_EQ(3u, omsg.some_vector()[2]);
    EXPECT_EQ(4u, omsg.some_vector()[3]);
}

TEST_F(ContainerTest, Evolve) {
    Buffer test, testData;
    testData.append("asdf", 4);

    // construct and serialize a legacy data object
    TestLegacy legacy(TestField(0), 123, 45, TestEnum::VALUE_BLA, 678, testData, 901, {1, 2, 3});
    legacy.serialize(test);

    // deserialize into a future object having deprecated flags
    TestFuture future;
    ASSERT_TRUE(future.deserialize(test));

    // check the leftover fields
    EXPECT_EQ(123u, future.first());
    EXPECT_EQ(45u, future.second());
    EXPECT_EQ(4u, future.fourth().size());
    EXPECT_ARRAY_EQ(const uint8_t, "asdf", future.fourth().const_data(), 4);
    EXPECT_EQ(901u, future.fifth());
    EXPECT_EQ(TestEnum::VALUE_1, future.newEnum());
    EXPECT_EQ(0u, future.newFieldValue());
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
