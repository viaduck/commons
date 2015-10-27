//
// Created by steffen on 10.09.15.
//

#include <libCom/BlockCache.h>
#include "BlockCacheTest.h"

TEST_F(BlockCacheTest, WriteNoExceedCapacity) {
    BlockCache<uint32_t, uint32_t> cache(10);
    ASSERT_EQ(10, cache.capacity());

    cache.write(1, 10);
    ASSERT_EQ(1, cache.size());
    cache.write(2, 12);
    ASSERT_EQ(2, cache.size());
    cache.write(3, 14);
    ASSERT_EQ(3, cache.size());
    cache.write(4, 16);
    ASSERT_EQ(4, cache.size());
    cache.write(5, 18);
    ASSERT_EQ(5, cache.size());
    cache.write(6, 20);
    ASSERT_EQ(6, cache.size());
    cache.write(7, 22);
    ASSERT_EQ(7, cache.size());
    cache.write(8, 24);
    ASSERT_EQ(8, cache.size());
    cache.write(9, 26);
    ASSERT_EQ(9, cache.size());
    cache.write(10, 28);
    ASSERT_EQ(10, cache.size());
}

TEST_F(BlockCacheTest, WriteReadNoExceedCapacity) {
    BlockCache<uint32_t, uint32_t> cache(10);
    ASSERT_EQ(10, cache.capacity());

    // write
    cache.write(1, 10);
    ASSERT_EQ(1, cache.size());
    cache.write(2, 12);
    ASSERT_EQ(2, cache.size());
    cache.write(3, 14);
    ASSERT_EQ(3, cache.size());
    cache.write(4, 16);
    ASSERT_EQ(4, cache.size());
    cache.write(5, 18);
    ASSERT_EQ(5, cache.size());
    cache.write(6, 20);
    ASSERT_EQ(6, cache.size());
    cache.write(7, 22);
    ASSERT_EQ(7, cache.size());
    cache.write(8, 24);
    ASSERT_EQ(8, cache.size());
    cache.write(9, 26);
    ASSERT_EQ(9, cache.size());
    cache.write(10, 28);
    ASSERT_EQ(10, cache.size());

    // read
    ASSERT_EQ(26, cache.read(9));
    ASSERT_EQ(10, cache.read(1));
    ASSERT_EQ(20, cache.read(6));
    ASSERT_EQ(28, cache.read(10));
    ASSERT_EQ(16, cache.read(4));
    ASSERT_EQ(18, cache.read(5));
    ASSERT_EQ(12, cache.read(2));
    ASSERT_EQ(22, cache.read(7));
    ASSERT_EQ(14, cache.read(3));
    ASSERT_EQ(24, cache.read(8));
}

TEST_F(BlockCacheTest, WriteExceedCapacity) {
    BlockCache<uint32_t, uint32_t> cache(5);
    ASSERT_EQ(5, cache.capacity());

    cache.write(1, 10);
    ASSERT_EQ(1, cache.size());
    cache.write(2, 12);
    ASSERT_EQ(2, cache.size());
    cache.write(3, 14);
    ASSERT_EQ(3, cache.size());
    cache.write(4, 16);
    ASSERT_EQ(4, cache.size());
    cache.write(5, 18);
    ASSERT_EQ(5, cache.size());

    // this will overwrite entries, so size stays the same
    cache.write(6, 20);
    ASSERT_EQ(5, cache.size());
    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    cache.write(20, 40);
    ASSERT_EQ(5, cache.size());
}

TEST_F(BlockCacheTest, WriteReadExceedCapacity) {
    BlockCache<uint32_t, uint32_t> cache(5);
    ASSERT_EQ(5, cache.capacity());

    cache.write(1, 10);
    ASSERT_EQ(1, cache.size());
    cache.write(2, 12);
    ASSERT_EQ(2, cache.size());
    cache.write(3, 14);
    ASSERT_EQ(3, cache.size());
    cache.write(4, 16);
    ASSERT_EQ(4, cache.size());
    cache.write(5, 18);
    ASSERT_EQ(5, cache.size());

    // this will overwrite entries, so size stays the same
    cache.write(6, 20);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(20, cache.read(6));       // newly inserted

    cache.write(1, 42);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(10, cache.read(1));       // should stay the same, since already present

    cache.write(20, 40);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(40, cache.read(20));      // newly inserted
}

TEST_F(BlockCacheTest, GenerationNoExceedCapacity) {
    BlockCache<uint32_t, uint32_t> cache(10);
    ASSERT_EQ(10, cache.capacity());

    cache.write(1, 10);
    ASSERT_EQ(1, cache.size());
    ASSERT_EQ(1, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(2, 12);
    ASSERT_EQ(2, cache.size());
    ASSERT_EQ(1, cache.generation(2));
    ASSERT_EQ(12, cache.read(2));

    cache.write(3, 14);
    ASSERT_EQ(3, cache.size());
    ASSERT_EQ(1, cache.generation(3));
    ASSERT_EQ(14, cache.read(3));

    cache.write(4, 16);
    ASSERT_EQ(4, cache.size());
    ASSERT_EQ(1, cache.generation(4));
    ASSERT_EQ(16, cache.read(4));

    cache.write(5, 18);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(1, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));

    // push generation
    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(3, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(4, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(4));
    ASSERT_EQ(16, cache.read(4));

    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(4, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));
}

TEST_F(BlockCacheTest, GenerationExceedCapacity) {
    BlockCache<uint32_t, uint32_t> cache(5);
    ASSERT_EQ(5, cache.capacity());

    cache.write(1, 10);
    ASSERT_EQ(1, cache.size());
    ASSERT_EQ(1, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(2, 12);
    ASSERT_EQ(2, cache.size());
    ASSERT_EQ(1, cache.generation(2));
    ASSERT_EQ(12, cache.read(2));

    cache.write(3, 14);
    ASSERT_EQ(3, cache.size());
    ASSERT_EQ(1, cache.generation(3));
    ASSERT_EQ(14, cache.read(3));

    cache.write(4, 16);
    ASSERT_EQ(4, cache.size());
    ASSERT_EQ(1, cache.generation(4));
    ASSERT_EQ(16, cache.read(4));

    cache.write(5, 18);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(1, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));


    // push generation
    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(3, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(4, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(4));
    ASSERT_EQ(16, cache.read(4));

    cache.write(1, 10);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(4, cache.generation(1));
    ASSERT_EQ(10, cache.read(1));

    cache.write(4, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(3, cache.generation(4));
    ASSERT_EQ(16, cache.read(4));

    cache.write(2, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(2));
    ASSERT_EQ(12, cache.read(2));

    cache.write(5, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));

    cache.write(5, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(3, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));

    cache.write(5, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(4, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));

    cache.write(5, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(5, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));

    cache.write(5, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(6, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));

    cache.write(5, 16);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(7, cache.generation(5));
    ASSERT_EQ(18, cache.read(5));


    ASSERT_EQ(1, cache.generation(3));
    ASSERT_EQ(14, cache.read(3));


    // following exceed capacity
    cache.write(6, 20);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(1, cache.generation(6));
    ASSERT_EQ(20, cache.read(6));
    ASSERT_EQ(0, cache.read(3));        // 3 had lowest generation (1) -> got removed

    //
    cache.write(7, 22);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(1, cache.generation(7));
    ASSERT_EQ(22, cache.read(7));
    ASSERT_EQ(0, cache.read(3));        // 6 had lowest generation (1) -> got removed

    cache.write(7, 22);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(2, cache.generation(7));
    ASSERT_EQ(22, cache.read(7));

    cache.write(7, 22);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(3, cache.generation(7));
    ASSERT_EQ(22, cache.read(7));

    //

    cache.write(8, 24);
    ASSERT_EQ(5, cache.size());
    ASSERT_EQ(1, cache.generation(8));
    ASSERT_EQ(24, cache.read(8));
    ASSERT_EQ(0, cache.read(2));        // 2 had lowest generation (2) -> got removed
}
