#include <libCom/Buffer.h>
#include <libCom/BufferRange.h>
#include "BufferTest.h"
#include "custom_assert.h"
#include <libCom/BufferRange.h>


TEST_F(BufferTest, AppendNoOverflow) {
    Buffer a(20);
    ASSERT_EQ(size_t(0), a.size());

    a.append("abcdef", 7);

    ASSERT_EQ(7, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.data(2), 5);

    Buffer b(10);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    b.append("abcdef", 0);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.append("abcdef", 7);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", b.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "def", b.data(3), 4);
    //bytesNotEqual("defdef", 7, b.data(), 7);
}

TEST_F(BufferTest, AppendOverflowTest) {
    Buffer b(5);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abc", b.data(), 3);

    b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.data(), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.const_data(), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.data(4), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.const_data(4), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.const_data(4), 5);

    b.append("abcdefghijklmnopqrstuvwxyz", 26);
    ASSERT_EQ(35, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz", b.data(), 35);
    EXPECT_ARRAY_EQ(const uint8_t, "bcdefghijklmnopqrstuvwxyz", b.data(10), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "bcdefghijklmnopqrstuvwxyz", b.const_data(10), 25);
}

TEST_F(BufferTest, ConsumeTest) {
    Buffer b(5);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abc", b.data(), 3);

    b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.data(), 9);

    b.consume(2);
    ASSERT_EQ(7, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "cdefghi", b.data(), 7);

    b.consume(20);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "", b.data(), 0);

    b.append("abcd", 4);
    ASSERT_EQ(4, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd", b.data(), 4);
}

TEST_F(BufferTest, UseTest) {
    Buffer b(25);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.use(0);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.use(2);
    ASSERT_EQ(2, static_cast<int32_t>(b.size()));

    b.use(0);
    ASSERT_EQ(2, static_cast<int32_t>(b.size()));

    b.use(10);
    ASSERT_EQ(12, static_cast<int32_t>(b.size()));

    b.use(80);       // this would exceed capacity -> size must be capacity now
    ASSERT_EQ(25, static_cast<int32_t>(b.size()));
}

TEST_F(BufferTest, ComparisonTest) {
    // no data
    {
        // no capacity
        Buffer b(0);
        Buffer b2(0);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);
    }
    {
        // different capacity
        Buffer b(10);
        Buffer b2(5);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);
    }
    {
        // default capacity
        Buffer b;
        Buffer b2;
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);
    }
    // equal data
    {
        // no capacity
        Buffer b(0);
        Buffer b2(0);
        b.append("abc", 3);
        b2.append("abc", 3);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);

        b.clear();
        b2.clear();
        b.append("abc", 3);
        b2.append("abc", 3);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);
    }
    {
        // different capacity
        Buffer b(10);
        Buffer b2(5);
        b.append("abc", 3);
        b2.append("abc", 3);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);

        b.clear();
        b2.clear();
        b.append("abc", 3);
        b2.append("abc", 3);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);
    }
    {
        // default capacity
        Buffer b;
        Buffer b2;
        b.append("abc", 3);
        b2.append("abc", 3);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);

        b.clear();
        b2.clear();
        b.append("abc", 3);
        b2.append("abc", 3);
        ASSERT_TRUE(b == b2);
        ASSERT_FALSE(b != b2);
    }
    // different data
    {
        // no capacity
        Buffer b(0);
        Buffer b2(0);
        b.append("abc", 3);
        b2.append("abcd", 4);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("", 0);
        b2.append("abc", 3);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("abc", 3);
        b2.append("", 0);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("abcd", 4);
        b2.append("abc", 3);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);
    }
    {
        // different capacity
        Buffer b(10);
        Buffer b2(5);
        b.append("abc", 3);
        b2.append("abcd", 4);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("", 0);
        b2.append("abc", 3);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("abc", 3);
        b2.append("", 0);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("abcd", 4);
        b2.append("abc", 3);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);
    }
    {
        // default capacity
        Buffer b;
        Buffer b2;
        b.append("abc", 3);
        b2.append("abcd", 4);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("", 0);
        b2.append("abc", 3);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("abc", 3);
        b2.append("", 0);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);

        b.clear();
        b2.clear();
        b.append("abcd", 4);
        b2.append("abc", 3);
        ASSERT_FALSE(b == b2);
        ASSERT_TRUE(b != b2);
    }
}
