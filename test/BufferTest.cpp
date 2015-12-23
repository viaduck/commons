#include <libCom/Buffer.h>
#include <libCom/Range.h>
#include <libCom/BufferRange.h>
#include "BufferTest.h"
#include "custom_assert.h"


TEST_F(BufferTest, CopyConstructor) {
    Buffer a(20);
    ASSERT_EQ(size_t(0), a.size());
    a.append("abcdef", 6);

    ASSERT_EQ(6, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.data(), 6);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.data(2), 4);

    Buffer b(a);
    ASSERT_EQ(a.size(), b.size());
    EXPECT_ARRAY_EQ(const uint8_t, a.data(), b.data(), 6);

    // append to the copied buffer -> must not modify original
    b.append("012345", 6);

    // new buffer
    ASSERT_EQ(12, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef012345", b.data(), 12);

    // old buffer
    ASSERT_EQ(6, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.data(), 6);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.data(2), 4);
}

TEST_F(BufferTest, ConstDataRange) {
    Buffer a(20);
    ASSERT_EQ(0, static_cast<int32_t>(a.size()));
    a.append("0123456789", 10);
    ASSERT_EQ(10, static_cast<int32_t>(a.size()));

    BufferRangeConst br1 = a.const_data(0, 10);
    ASSERT_EQ(0, static_cast<int32_t>(br1.offset()));
    ASSERT_EQ(10, static_cast<int32_t>(br1.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", br1.const_data(), 10);

    BufferRangeConst br2 = a.const_data(3, 4);
    ASSERT_EQ(3, static_cast<int32_t>(br2.offset()));
    ASSERT_EQ(4, static_cast<int32_t>(br2.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "3456", br2.const_data(), 4);

    // TODO const_data(uint32_t, uint32_t) should check for Buffer boundaries (offset, size)!
}

TEST_F(BufferTest, AppendNoOverflow) {
    Buffer a(20);
    ASSERT_EQ(size_t(0), a.size());

    a.append("abcdef", 7);

    ASSERT_EQ(7, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.const_data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.data(2), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.const_data(2), 5);


    Buffer b(10);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    b.append("fedcba", 0);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.append("fedcba", 7);
    ASSERT_EQ(7, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba", b.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba", b.const_data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "cba", b.data(3), 4);
    EXPECT_ARRAY_EQ(const uint8_t, "cba", b.const_data(3), 4);

    // append Buffer
    Buffer c(50);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));
    c.append(b);
    c.append(a);

    ASSERT_EQ(14, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0abcdef\0", c.data(), 14);
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0abcdef\0", c.const_data(), 14);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0abcdef\0", c.data(4), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0abcdef\0", c.const_data(4), 10);

    // append BufferRange
    Buffer d(50);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));
    d.append(BufferRangeConst(b, 0, b.size()));
    d.append(BufferRangeConst(a, 3, 3));

    ASSERT_EQ(10, static_cast<int32_t>(d.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0def\0", d.data(), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0def\0", d.const_data(), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0def\0", d.data(4), 6);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0def\0", d.const_data(4), 6);
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

    // append Buffer
    Buffer c(5);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));
    c.append(b);
    ASSERT_EQ(35, static_cast<int32_t>(c.size()));
    c.append("01234", 5);
    ASSERT_EQ(40, static_cast<int32_t>(c.size()));

    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz012345", c.data(), 40);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz012345", c.const_data(), 40);
    EXPECT_ARRAY_EQ(const uint8_t, "ghijklmnopqrstuvwxyz012345", c.data(15), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "ghijklmnopqrstuvwxyz012345", c.const_data(15), 25);

    // append BufferRange
    Buffer d(5);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));
    d.append(BufferRangeConst(b, 0, b.size()));
    d.append(BufferRangeConst(c, 10, 15));
    ASSERT_EQ(50, static_cast<int32_t>(d.size()));
    
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyzbcdefghijklmnop", d.data(), 50);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyzbcdefghijklmnop", d.const_data(), 50);
    EXPECT_ARRAY_EQ(const uint8_t, "efghijklmnopqrstuvwxyzbcdefghijklmnop", d.data(13), 37);
    EXPECT_ARRAY_EQ(const uint8_t, "efghijklmnopqrstuvwxyzbcdefghijklmnop", d.const_data(13), 37);
}

TEST_F(BufferTest, WriteTestNoOverflow) {
    Buffer b(20);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));

    b.write("01234", 5, 0);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "01234fghi", b.data(0), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "01234fghi", b.const_data(0), 9);

    b.write("01234", 5, 3);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "01201234i", b.data(0), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "01201234i", b.const_data(0), 9);

    // buffer range
    Buffer c(20);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));

    c.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(c.size()));
    c.append("defghijklmnopqrstuvxyz", 22);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));

    c.write(BufferRangeConst(b, 0, 5), 4);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201jklmnopqrstuvxyz", c.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201jklmnopqrstuvxyz", c.const_data(0), 25);

    c.write(BufferRangeConst(b, 3, 4), 10);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201j0123opqrstuvxyz", c.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201j0123opqrstuvxyz", c.const_data(0), 25);

    // buffer
    Buffer d(30);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));

    d.append("012", 3);
    ASSERT_EQ(3, static_cast<int32_t>(d.size()));
    d.append("3456789012345678901234", 22);
    ASSERT_EQ(25, static_cast<int32_t>(d.size()));

    Buffer app(5);
    ASSERT_EQ(0, static_cast<int32_t>(app.size()));
    app.append("abcd", 4);
    ASSERT_EQ(4, static_cast<int32_t>(app.size()));
    d.write(app, 6);

    ASSERT_EQ(25, static_cast<int32_t>(d.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "012345abcd012345678901234", d.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "012345abcd012345678901234", d.const_data(0), 25);
}

TEST_F(BufferTest, WriteTestOverflow) {
    Buffer b(20);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));

    b.write("0123456789", 10, 0);
    ASSERT_EQ(10, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.data(0), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.const_data(0), 10);

    b.write("9876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210", 120, 20);
    ASSERT_EQ(140, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.data(0), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.const_data(0), 10);
    // there is a gap from (10, 20)
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210", b.data(20), 120);
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210", b.const_data(20), 120);

    // buffer range
    Buffer c(30);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));

    c.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(c.size()));
    c.append("defghijklmnopqrstuvxyz", 22);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));

    c.write(BufferRangeConst(b, 20, 60), 30);
    ASSERT_EQ(90, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", c.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", c.const_data(0), 25);
    // gap from (25, 30)
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", c.data(30), 60);
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", c.const_data(30), 60);

    // buffer
    Buffer d(20);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));

    d.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(d.size()));
    d.append("defghijklmnopqrstuvxyz", 22);
    ASSERT_EQ(25, static_cast<int32_t>(d.size()));

    d.write(c, 30);
    ASSERT_EQ(120, static_cast<int32_t>(d.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.const_data(0), 25);
    // gap from (25, 30)
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.data(30), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.const_data(30), 25);
    // gap from (55, 60)
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", d.data(60), 60);
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", d.const_data(60), 60);
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

TEST_F(BufferTest, ResetTest) {
    Buffer a(100);
    ASSERT_EQ(0, static_cast<int32_t>(a.size()));
    a.use(20);
    ASSERT_EQ(20, static_cast<int32_t>(a.size()));
    a.reset(10);
    ASSERT_EQ(20, static_cast<int32_t>(a.size()));      // no offset yet
    a.consume(5);
    ASSERT_EQ(15, static_cast<int32_t>(a.size()));
    a.reset(5);
    ASSERT_EQ(20, static_cast<int32_t>(a.size()));
    a.reset(5);
    ASSERT_EQ(20, static_cast<int32_t>(a.size()));      // no offset anymore
    a.reset(0);
    ASSERT_EQ(20, static_cast<int32_t>(a.size()));
}

TEST_F(BufferTest, PaddTest) {
    Buffer a(10);
    ASSERT_EQ(0, static_cast<int32_t>(a.size()));
    a.append("abcdef", 6);
    ASSERT_EQ(6, static_cast<int32_t>(a.size()));
    a.padd(45, 0xBE);
    ASSERT_EQ(45, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", a.data(), 45);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", a.const_data(), 45);
}

TEST_F(BufferTest, IncreaseTest) {
    Buffer a(4);
    ASSERT_EQ(0, static_cast<int32_t>(a.size()));
    a.append("abcd", 4);
    ASSERT_EQ(4, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd", a.data(), 4);

    a.increase(100, static_cast<uint8_t>(0xAD));            // should prevents segmentation fault to non allocated memory in calls below
    EXPECT_ARRAY_EQ(const uint8_t, "\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD", a.data(4), 96);        // newly allocated memory must be set to 0xAD
    EXPECT_ARRAY_EQ(const uint8_t, "\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD", a.const_data(4), 96);        // newly allocated memory must be set to 0xAD
    memset(a.data(), 0xBE, 100);
    EXPECT_ARRAY_EQ(const uint8_t, "\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", a.data(0), 100);
    EXPECT_ARRAY_EQ(const uint8_t, "\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", a.const_data(0), 100);

    // increase by
    Buffer b(4);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    b.append("abcd", 4);
    ASSERT_EQ(4, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd", b.data(), 4);

    b.increase(96, static_cast<uint8_t>(0xAD), true);            // should prevents segmentation fault to non allocated memory in calls below
    EXPECT_ARRAY_EQ(const uint8_t, "\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD", b.data(4), 96);        // newly allocated memory must be set to 0xAD
    EXPECT_ARRAY_EQ(const uint8_t, "\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD\xAD", b.const_data(4), 96);        // newly allocated memory must be set to 0xAD
    memset(b.data(), 0xBE, 100);
    EXPECT_ARRAY_EQ(const uint8_t, "\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", b.data(0), 100);
    EXPECT_ARRAY_EQ(const uint8_t, "\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", b.const_data(0), 100);
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
