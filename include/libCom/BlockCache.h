//
// Created by steffen on 10.09.15.
//

#ifndef LIBCOM_BLOCKCACHE_H
#define LIBCOM_BLOCKCACHE_H

#include <unordered_map>
#include <cinttypes>

class BlockInfo {

public:
    BlockInfo();
    BlockInfo(const uint32_t address);

    uint32_t address;
    uint16_t generation = 1;
};

class BlockCache {

public:
    BlockCache(const uint32_t capacity = DEFAULT_CAPACITY);

    /**
     * Returns the block's address associated with that id
     * @param id Block's id
     * @return Block's address or 0 if no such block is present
     */
    const uint32_t read(const uint32_t id) const;

    /**
     * Returns an item's current generation
     * @param id Block's id
     * @return Generation or 0 if no such block is present
     */
    const uint16_t generation(const uint32_t id) const;

    /**
     * Writes a block into the cache
     * @param id Block's id
     * @param address Block's address
     * @return True if there was enough (no block removed), false if not
     */
    bool write(const uint32_t id, const uint32_t address);

    /**
     * Returns the current cache size
     */
    const inline uint32_t size() const {
        return static_cast<uint32_t>(mMap.size());
    }

    /**
     * Returns the cache capacity
     */
    const inline uint32_t capacity() const {
        return mCapacity;
    }

    /**
     * Default maximum number of elements within the cache
     */
    const static uint32_t DEFAULT_CAPACITY = 4096;

private:
    /**
     * Maps: id -> BlockInfo
     */
    std::unordered_map<uint32_t, BlockInfo> mMap;
    std::unordered_multimap<uint16_t, uint32_t> mGenMap;        // generation -> id

    /**
     * The lowest generation
     */
    uint16_t mLeast = 0;

    /**
     * This cache's capacity
     */
    const uint32_t mCapacity;
};

#endif //LIBCOM_BLOCKCACHE_H
