#include <gtest/gtest.h>
#include "custom_assert.h"

#include <libCom/DevNull.h>
#include <libCom/BufferRange.h>

/*
 * DevNull's actions are no-ops, but indicate success
 */

TEST(DevNullTest, appendTest) {
    DevNull d;
    {
        BufferRangeConst range = d.append("abc", 3);
        ASSERT_EQ(static_cast<uint32_t>(0), d.size());
        ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(3), range.size());
    }
    {
        BufferRangeConst range = d.append(static_cast<const void *>("def"), 3);
        ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(3), range.size());
    }

    Buffer b;
    {
        // this is a buffer
        BufferRangeConst range = b.append("12345", 5);
        ASSERT_EQ(static_cast<uint32_t>(5), b.size());      // Buffer!
        ASSERT_EQ(static_cast<uint32_t>(0), range.offset());    // Buffer!
        ASSERT_EQ(static_cast<uint32_t>(5), range.size());      // Buffer!
    }

    {
        BufferRangeConst range = d.append(b);
        ASSERT_EQ(static_cast<uint32_t>(0), d.size());
        ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(5), range.size());
    }

    {
        BufferRangeConst range = d.append(BufferRangeConst(b));
        ASSERT_EQ(static_cast<uint32_t>(0), d.size());
        ASSERT_EQ(static_cast<uint32_t>(0), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(5), range.size());
    }
}


TEST(DevNullTest, consumeTest) {
    DevNull d;
    d.append("abc", 3);

    d.consume(0);
    ASSERT_EQ(static_cast<uint32_t>(0), d.size());

    d.consume(20);
    ASSERT_EQ(static_cast<uint32_t>(0), d.size());
}

TEST(DevNullTest, useTest) {
    DevNull d;
    d.append("abc", 3);

    d.use(0);
    ASSERT_EQ(static_cast<uint32_t>(0), d.size());

    d.use(20);
    ASSERT_EQ(static_cast<uint32_t>(0), d.size());
}

TEST(DevNullTest, writeTest) {
    DevNull d;
    {
        BufferRangeConst range = d.write(static_cast<const void *>("abc"), 50, 1337);
        ASSERT_EQ(static_cast<uint32_t>(0), d.size());
        ASSERT_EQ(static_cast<uint32_t>(1337), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(50), range.size());
    }

    Buffer b;
    {
        // this is a buffer
        BufferRangeConst range = b.write("12345", 5, 0);
        ASSERT_EQ(static_cast<uint32_t>(5), b.size());          // Buffer!
        ASSERT_EQ(static_cast<uint32_t>(0), range.offset());    // Buffer!
        ASSERT_EQ(static_cast<uint32_t>(5), range.size());      // Buffer!
    }

    {
        BufferRangeConst range = d.write(b, 12);
        ASSERT_EQ(static_cast<uint32_t>(0), d.size());
        ASSERT_EQ(static_cast<uint32_t>(12), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(b.size()), range.size());
    }

    {
        BufferRangeConst range = d.write(BufferRangeConst(b), 10);
        ASSERT_EQ(static_cast<uint32_t>(0), d.size());
        ASSERT_EQ(static_cast<uint32_t>(10), range.offset());
        ASSERT_EQ(static_cast<uint32_t>(5), range.size());
    }
}
