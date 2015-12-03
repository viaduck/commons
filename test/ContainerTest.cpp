//
// Created by steffen on 13.08.15.
//

#include "libCom/conversions.h"
#include "libCom/Buffer.h"
#include "test/sometest.h"
#include "ContainerTest.h"
#include "custom_assert.h"


TEST_F(ContainerTest, SimpleRead) {
    Buffer a(20);
    const uint32_t firstInt = hton_uint32_t(0xBEEFDEAD);
    const uint8_t secondInt = 0xfe;
    const uint16_t third = hton_uint16_t(0xDEAD);
    const uint8_t arr[] = { 'a', 'b', 'c' };
//    const uint8_t arr[] = { 'a', 'b', 'c' };

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
    EXPECT_ARRAY_EQ(const uint8_t, "abc", c.const_buf(), 3);
    EXPECT_ARRAY_EQ(const uint8_t, "abc", const_cast<const uint8_t*>(c.buf()), 3);
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.const_buf()+3, 7);
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB", const_cast<const uint8_t*>(c.buf()+3), 7);
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
    memset(c.buf(), 0xAB, 10);
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.const_buf(), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.buf(), 10);

    ASSERT_EQ(hton_uint32_t(0xBEEFDEAD), *reinterpret_cast<const uint32_t*>(c.buffer().const_data()));
    ASSERT_EQ(0xfe, *static_cast<const uint8_t*>(c.buffer().const_data(4)));
    uint16_t bla = hton_uint16_t(0xDEAD);
    ASSERT_EQ(bla, *reinterpret_cast<const uint16_t*>(c.buffer().const_data(5)));
    EXPECT_ARRAY_EQ(const uint8_t, "\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB\xAB", c.buffer().const_data(7), 10);      // no need to respect endianess, since it's a byte-sequence
}
