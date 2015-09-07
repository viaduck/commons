#include <libCom/Buffer.h>
#include "BufferTest.h"
#include "custom_assert.h"


TEST_F(BufferTest, AppendNoOverflow) {
    Buffer a(20);
    ASSERT_EQ(size_t(0), a.size());

    a.append("abcdef", 7);

    ASSERT_EQ(7, a.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.data(2), 5);

    Buffer b(10);
    ASSERT_EQ(0, b.size());
    b.append("abcdef", 0);
    ASSERT_EQ(0, b.size());

    b.append("abcdef", 7);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", b.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "def", b.data(3), 4);
    //bytesNotEqual("defdef", 7, b.data(), 7);
}

TEST_F(BufferTest, AppendOverflowTest) {
    Buffer b(5);
    ASSERT_EQ(0, b.size());

    b.append("abc", 3);
    ASSERT_EQ(3, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abc", b.data(), 3);

    b.append("defghi", 6);
    ASSERT_EQ(9, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.data(), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.const_data(), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.data(4), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.const_data(4), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.const_data(4), 5);

    b.append("abcdefghijklmnopqrstuvwxyz", 26);
    ASSERT_EQ(35, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz", b.data(), 35);
    EXPECT_ARRAY_EQ(const uint8_t, "bcdefghijklmnopqrstuvwxyz", b.data(10), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "bcdefghijklmnopqrstuvwxyz", b.const_data(10), 25);
}

TEST_F(BufferTest, ConsumeTest) {
    Buffer b(5);
    ASSERT_EQ(0, b.size());

    b.append("abc", 3);
    ASSERT_EQ(3, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abc", b.data(), 3);

    b.append("defghi", 6);
    ASSERT_EQ(9, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.data(), 9);

    b.consume(2);
    ASSERT_EQ(7, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "cdefghi", b.data(), 7);

    b.consume(20);
    ASSERT_EQ(0, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "", b.data(), 0);

    b.append("abcd", 4);
    ASSERT_EQ(4, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "abcd", b.data(), 4);
}
