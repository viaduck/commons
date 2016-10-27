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

TEST_F(BufferTest, MoveConstructor) {
    Buffer a(20);
    a.append("abcdefghijkl", 12);
    void *aOldInternal = a.data();
    uint32_t aOldSize = a.size();

    Buffer b(std::move(a));
    ASSERT_EQ(0u, a.size());
    ASSERT_EQ(nullptr, a.data());

    ASSERT_EQ(aOldSize, b.size());
    ASSERT_EQ(aOldInternal, b.data());

    Buffer c = std::move(b);
    ASSERT_EQ(0u, b.size());
    ASSERT_EQ(nullptr, b.data());

    ASSERT_EQ(aOldSize, c.size());
    ASSERT_EQ(aOldInternal, c.data());
}

TEST_F(BufferTest, Swap) {
    Buffer a(20);
    Buffer b(40);

    a.append("abcdefghijkl", 12);
    b.append("0123456789", 10);

    libcom::swap(a, b);
    EXPECT_EQ(10u, a.size());
    EXPECT_EQ(12u, b.size());
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", a.const_data(), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijkl", b.const_data(), 12);
}

TEST_F(BufferTest, UnsafeDataAccess) {
    // tests for invalid values of raw data pointer offset
    Buffer a(20);
    ASSERT_EQ(0, static_cast<int32_t>(a.size()));
    a.append("0123456789", 10);
    ASSERT_EQ(10, static_cast<int32_t>(a.size()));
    memset(a.data(a.size()), 0xFF, 10);     // set "invalid" indices to 0xFF

    // offset too big, must return pointer after last index (next 10 indices must be 0xFF)
    EXPECT_ARRAY_EQ(const uint8_t, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", a.const_data(12), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", a.const_data(-1), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", a.data(12), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", a.data(-1), 10);
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

    // range exceeds buffer -> BufferRange must have offset == size() and size == 0
    BufferRangeConst br3 = a.const_data(a.size()+20, 234);
    ASSERT_EQ(a.size(), br3.offset());
    ASSERT_EQ(0, static_cast<int32_t>(br3.size()));

    // range exceeds buffer (offset within bounds, but size not) -> BufferRange must have offset unaltered and size == size() - offset
    BufferRangeConst br4 = a.const_data(3, 234);
    ASSERT_EQ(3, static_cast<int32_t>(br4.offset()));
    ASSERT_EQ(a.size()-3, br4.size());
}

TEST_F(BufferTest, AppendNoOverflow) {
    Buffer a(20);
    ASSERT_EQ(size_t(0), a.size());

    BufferRangeConst range = a.append("abcdef", 7);

    ASSERT_EQ(7, static_cast<int32_t>(a.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdef", a.const_data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.data(2), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "cdef", a.const_data(2), 5);

    ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
    ASSERT_EQ(static_cast<uint32_t>(7), range.size());


    Buffer b(10);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    BufferRangeConst range2 = b.append("fedcba", 0);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range2.offset());
    ASSERT_EQ(static_cast<uint32_t>(0), range2.size());

    BufferRangeConst range3 = b.append("fedcba", 7);
    ASSERT_EQ(7, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba", b.data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba", b.const_data(), 7);
    EXPECT_ARRAY_EQ(const uint8_t, "cba", b.data(3), 4);
    EXPECT_ARRAY_EQ(const uint8_t, "cba", b.const_data(3), 4);
    ASSERT_EQ(static_cast<uint32_t>(0), range3.offset());
    ASSERT_EQ(static_cast<uint32_t>(7), range3.size());

    // append Buffer
    Buffer c(50);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));
    BufferRangeConst range4 = c.append(b);
    ASSERT_EQ(static_cast<uint32_t>(0), range4.offset());
    ASSERT_EQ(static_cast<uint32_t>(7), range4.size());
    BufferRangeConst range5 = c.append(a);
    ASSERT_EQ(static_cast<uint32_t>(7), range5.offset());
    ASSERT_EQ(static_cast<uint32_t>(7), range5.size());

    ASSERT_EQ(14, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0abcdef\0", c.data(), 14);
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0abcdef\0", c.const_data(), 14);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0abcdef\0", c.data(4), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0abcdef\0", c.const_data(4), 10);


    // append BufferRange
    Buffer d(50);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));
    BufferRangeConst range6 = d.append(BufferRangeConst(b, 0, b.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range6.offset());
    ASSERT_EQ(static_cast<uint32_t>(b.size()), range6.size());

    BufferRangeConst range7 = d.append(BufferRangeConst(a, 3, 3));
    ASSERT_EQ(static_cast<uint32_t>(b.size()), range7.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range7.size());

    ASSERT_EQ(10, static_cast<int32_t>(d.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0def\0", d.data(), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "fedcba\0def\0", d.const_data(), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0def\0", d.data(4), 6);
    EXPECT_ARRAY_EQ(const uint8_t, "ba\0def\0", d.const_data(4), 6);
}

TEST_F(BufferTest, AppendOverflowTest) {
    Buffer b(5);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    BufferRangeConst range = b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abc", b.data(), 3);
    ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range.size());

    BufferRangeConst range1 = b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.data(), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghi", b.const_data(), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.data(4), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.const_data(4), 5);
    EXPECT_ARRAY_EQ(const uint8_t, "efghi", b.const_data(4), 5);
    ASSERT_EQ(static_cast<uint32_t>(3), range1.offset());
    ASSERT_EQ(static_cast<uint32_t>(6), range1.size());

    BufferRangeConst range2 = b.append("abcdefghijklmnopqrstuvwxyz", 26);
    ASSERT_EQ(35, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz", b.data(), 35);
    EXPECT_ARRAY_EQ(const uint8_t, "bcdefghijklmnopqrstuvwxyz", b.data(10), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "bcdefghijklmnopqrstuvwxyz", b.const_data(10), 25);
    ASSERT_EQ(static_cast<uint32_t>(9), range2.offset());
    ASSERT_EQ(static_cast<uint32_t>(26), range2.size());

    // append Buffer
    Buffer c(5);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));
    BufferRangeConst range3 = c.append(b);
    ASSERT_EQ(35, static_cast<int32_t>(c.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range3.offset());
    ASSERT_EQ(static_cast<uint32_t>(b.size()), range3.size());

    BufferRangeConst range4 = c.append("01234", 5);
    ASSERT_EQ(40, static_cast<int32_t>(c.size()));
    ASSERT_EQ(static_cast<uint32_t>(b.size()), range4.offset());
    ASSERT_EQ(static_cast<uint32_t>(5), range4.size());

    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz012345", c.data(), 40);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyz012345", c.const_data(), 40);
    EXPECT_ARRAY_EQ(const uint8_t, "ghijklmnopqrstuvwxyz012345", c.data(15), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "ghijklmnopqrstuvwxyz012345", c.const_data(15), 25);

    // append BufferRange
    Buffer d(5);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));
    BufferRangeConst range5 = d.append(BufferRangeConst(b, 0, b.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range5.offset());
    ASSERT_EQ(static_cast<uint32_t>(b.size()), range5.size());

    BufferRangeConst range6 = d.append(BufferRangeConst(c, 10, 15));
    ASSERT_EQ(50, static_cast<int32_t>(d.size()));
    ASSERT_EQ(static_cast<uint32_t>(b.size()), range6.offset());
    ASSERT_EQ(static_cast<uint32_t>(15), range6.size());
    
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyzbcdefghijklmnop", d.data(), 50);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghiabcdefghijklmnopqrstuvwxyzbcdefghijklmnop", d.const_data(), 50);
    EXPECT_ARRAY_EQ(const uint8_t, "efghijklmnopqrstuvwxyzbcdefghijklmnop", d.data(13), 37);
    EXPECT_ARRAY_EQ(const uint8_t, "efghijklmnopqrstuvwxyzbcdefghijklmnop", d.const_data(13), 37);
}

TEST_F(BufferTest, WriteTestNoOverflow) {
    Buffer b(20);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    BufferRangeConst range = b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range.size());

    BufferRangeConst range2 = b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    ASSERT_EQ(static_cast<uint32_t>(3), range2.offset());
    ASSERT_EQ(static_cast<uint32_t>(6), range2.size());

    BufferRangeConst range3 = b.write("01234", 5, 0);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "01234fghi", b.data(0), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "01234fghi", b.const_data(0), 9);
    ASSERT_EQ(static_cast<uint32_t>(0), range3.offset());
    ASSERT_EQ(static_cast<uint32_t>(5), range3.size());

    BufferRangeConst range4 = b.write("01234", 5, 3);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "01201234i", b.data(0), 9);
    EXPECT_ARRAY_EQ(const uint8_t, "01201234i", b.const_data(0), 9);
    ASSERT_EQ(static_cast<uint32_t>(3), range4.offset());
    ASSERT_EQ(static_cast<uint32_t>(5), range4.size());

    // buffer range
    Buffer c(20);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));

    BufferRangeConst range5 = c.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(c.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range5.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range5.size());

    BufferRangeConst range6 = c.append("defghijklmnopqrstuvxyz", 22);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));
    ASSERT_EQ(static_cast<uint32_t>(3), range6.offset());
    ASSERT_EQ(static_cast<uint32_t>(22), range6.size());

    BufferRangeConst range7 = c.write(BufferRangeConst(b, 0, 5), 4);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201jklmnopqrstuvxyz", c.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201jklmnopqrstuvxyz", c.const_data(0), 25);
    ASSERT_EQ(static_cast<uint32_t>(4), range7.offset());
    ASSERT_EQ(static_cast<uint32_t>(5), range7.size());

    BufferRangeConst range8 = c.write(BufferRangeConst(b, 3, 4), 10);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201j0123opqrstuvxyz", c.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcd01201j0123opqrstuvxyz", c.const_data(0), 25);
    ASSERT_EQ(static_cast<uint32_t>(10), range8.offset());
    ASSERT_EQ(static_cast<uint32_t>(4), range8.size());

    // buffer
    Buffer d(30);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));

    BufferRangeConst range9 = d.append("012", 3);
    ASSERT_EQ(3, static_cast<int32_t>(d.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range9.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range9.size());

    BufferRangeConst range10 = d.append("3456789012345678901234", 22);
    ASSERT_EQ(25, static_cast<int32_t>(d.size()));
    ASSERT_EQ(static_cast<uint32_t>(3), range10.offset());
    ASSERT_EQ(static_cast<uint32_t>(22), range10.size());

    Buffer app(5);
    ASSERT_EQ(0, static_cast<int32_t>(app.size()));
    BufferRangeConst range11 = app.append("abcd", 4);
    ASSERT_EQ(4, static_cast<int32_t>(app.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range11.offset());
    ASSERT_EQ(static_cast<uint32_t>(4), range11.size());

    BufferRangeConst range12 = d.write(app, 6);
    ASSERT_EQ(25, static_cast<int32_t>(d.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "012345abcd012345678901234", d.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "012345abcd012345678901234", d.const_data(0), 25);
    ASSERT_EQ(static_cast<uint32_t>(6), range12.offset());
    ASSERT_EQ(static_cast<uint32_t>(app.size()), range12.size());
}

TEST_F(BufferTest, WriteTestOverflow) {
    Buffer b(20);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    BufferRangeConst range = b.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(b.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range.size());

    BufferRangeConst range2 = b.append("defghi", 6);
    ASSERT_EQ(9, static_cast<int32_t>(b.size()));
    ASSERT_EQ(static_cast<uint32_t>(3), range2.offset());
    ASSERT_EQ(static_cast<uint32_t>(6), range2.size());

    BufferRangeConst range3 = b.write("0123456789", 10, 0);
    ASSERT_EQ(10, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.data(0), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.const_data(0), 10);
    ASSERT_EQ(static_cast<uint32_t>(0), range3.offset());
    ASSERT_EQ(static_cast<uint32_t>(10), range3.size());

    BufferRangeConst range4 = b.write("9876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210", 120, 20);
    ASSERT_EQ(140, static_cast<int32_t>(b.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.data(0), 10);
    EXPECT_ARRAY_EQ(const uint8_t, "0123456789", b.const_data(0), 10);
    // there is a gap from (10, 20)
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210", b.data(20), 120);
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210987654321098765432109876543210", b.const_data(20), 120);
    ASSERT_EQ(static_cast<uint32_t>(20), range4.offset());
    ASSERT_EQ(static_cast<uint32_t>(120), range4.size());

    // buffer range
    Buffer c(30);
    ASSERT_EQ(0, static_cast<int32_t>(c.size()));

    BufferRangeConst range5 = c.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(c.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range5.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range5.size());

    BufferRangeConst range6 = c.append("defghijklmnopqrstuvxyz", 22);
    ASSERT_EQ(25, static_cast<int32_t>(c.size()));
    ASSERT_EQ(static_cast<uint32_t>(3), range6.offset());
    ASSERT_EQ(static_cast<uint32_t>(22), range6.size());

    BufferRangeConst range7 = c.write(BufferRangeConst(b, 20, 60), 30);
    ASSERT_EQ(90, static_cast<int32_t>(c.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", c.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", c.const_data(0), 25);
    // gap from (25, 30)
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", c.data(30), 60);
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", c.const_data(30), 60);
    ASSERT_EQ(static_cast<uint32_t>(30), range7.offset());
    ASSERT_EQ(static_cast<uint32_t>(60), range7.size());

    // buffer
    Buffer d(20);
    ASSERT_EQ(0, static_cast<int32_t>(d.size()));

    BufferRangeConst range8 = d.append("abc", 3);
    ASSERT_EQ(3, static_cast<int32_t>(d.size()));
    ASSERT_EQ(static_cast<uint32_t>(0), range8.offset());
    ASSERT_EQ(static_cast<uint32_t>(3), range8.size());

    BufferRangeConst range9 = d.append("defghijklmnopqrstuvxyz", 22);
    ASSERT_EQ(25, static_cast<int32_t>(d.size()));
    ASSERT_EQ(static_cast<uint32_t>(3), range9.offset());
    ASSERT_EQ(static_cast<uint32_t>(22), range9.size());

    BufferRangeConst range10 = d.write(c, 30);
    ASSERT_EQ(120, static_cast<int32_t>(d.size()));
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.data(0), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.const_data(0), 25);
    // gap from (25, 30)
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.data(30), 25);
    EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvxyz", d.const_data(30), 25);
    // gap from (55, 60)
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", d.data(60), 60);
    EXPECT_ARRAY_EQ(const uint8_t, "987654321098765432109876543210987654321098765432109876543210", d.const_data(60), 60);
    ASSERT_EQ(static_cast<uint32_t>(30), range10.offset());
    ASSERT_EQ(static_cast<uint32_t>(c.size()), range10.size());
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

TEST_F(BufferTest, UnuseTest) {
    Buffer b(25);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.use(0);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
    b.unuse(0);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.use(2);
    ASSERT_EQ(2, static_cast<int32_t>(b.size()));
    b.unuse(2);
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));

    b.use(10);
    ASSERT_EQ(10, static_cast<int32_t>(b.size()));
    b.unuse(3);
    ASSERT_EQ(7, static_cast<int32_t>(b.size()));

    b.use(80);       // this would exceed capacity -> size must be capacity now
    ASSERT_EQ(25, static_cast<int32_t>(b.size()));
    b.unuse(500);       // this exceeds used size() ->  size must be 0 now
    ASSERT_EQ(0, static_cast<int32_t>(b.size()));
}

TEST_F(BufferTest, ConsumeUseTest) {
    {
        Buffer b(10);

        b.append("abcdefgh", 8);

        b.consume(3);
        ASSERT_EQ(5, static_cast<int32_t>(b.size()));
        EXPECT_ARRAY_EQ(const uint8_t, "defgh", b.data(), 5);

        b.use(50);
        ASSERT_EQ(7, static_cast<int32_t>(b.size()));
    }

    {
        Buffer b(10);

        b.append("abcdefgh", 8);

        b.consume(3);
        ASSERT_EQ(5, static_cast<int32_t>(b.size()));
        EXPECT_ARRAY_EQ(const uint8_t, "defgh", b.data(), 5);

        b.use(3);
        ASSERT_EQ(7, static_cast<int32_t>(b.size()));
    }
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

    {
        // padd no-op
        Buffer b(30);
        ASSERT_EQ(0, static_cast<int32_t>(b.size()));
        b.append("abcdefghijklmnopqrstuvwxyz0123456789", 36);
        ASSERT_EQ(36, static_cast<int32_t>(b.size()));
        b.padd(10, 0xBE);       // 10 < size() -> no-op
        ASSERT_EQ(36, static_cast<int32_t>(b.size()));
        EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvwxyz0123456789", b.data(), 36);
    }

    {
        // padd in middle of Buffer -> no-op
        Buffer b(30);
        ASSERT_EQ(0, static_cast<int32_t>(b.size()));
        b.append("abcdefghijklmnopqrstuvwxyz0123456789", 36);
        ASSERT_EQ(36, static_cast<int32_t>(b.size()));
        b.padd(5, 10, 0xBE);
        ASSERT_EQ(36, static_cast<int32_t>(b.size()));
        EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvwxyz0123456789", b.data(),
                        36);
    }
    {
        // padd partly out of range
        Buffer b(30);
        ASSERT_EQ(0, static_cast<int32_t>(b.size()));
        b.append("abcdefghijklmnopqrstuvwxyz0123456789", 36);
        ASSERT_EQ(36, static_cast<int32_t>(b.size()));
        b.padd(30, 10, 0xBE);
        ASSERT_EQ(40, static_cast<int32_t>(b.size()));
        EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvwxyz0123456789\xBE\xBE\xBE\xBE", b.data(),
                        40);
    }
    {
        // padd out of range
        Buffer b(30);
        ASSERT_EQ(0, static_cast<int32_t>(b.size()));
        b.append("abcdefghijklmnopqrstuvwxyz0123456789", 36);
        ASSERT_EQ(36, static_cast<int32_t>(b.size()));
        b.padd(40, 10, 0xBE);
        ASSERT_EQ(50, static_cast<int32_t>(b.size()));
        EXPECT_ARRAY_EQ(const uint8_t, "abcdefghijklmnopqrstuvwxyz0123456789", b.data(), 36);
        EXPECT_ARRAY_EQ(const uint8_t, "\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE\xBE", b.data(40), 10);
    }
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
