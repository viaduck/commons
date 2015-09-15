//
// Created by steffen on 10.09.15.
//

#ifndef LIBCOM_BLOCKCACHE_H
#define LIBCOM_BLOCKCACHE_H

#include <unordered_map>
#include <cinttypes>

template<class T>
class BlockInfo {

public:
    BlockInfo();
    BlockInfo(const T address);

    T address;
    uint16_t generation = 1;
};

template<class K, class V>
class BlockCache {

public:
    BlockCache(const uint32_t capacity = DEFAULT_CAPACITY);

    /**
     * Returns V associated with that K
     * @param id K
     * @return V of this K or object V with default constructor
     */
    const V read(const K id) const;

    /**
     * Returns an item's current generation
     * @param id K
     * @return Generation or 0 if no such block is present
     */
    const uint16_t generation(const K id) const;

    /**
     * Writes a K with its V into the cache
     * @param id K
     * @param address V
     * @return True if there was enough (no block removed), false if not
     */
    bool write(const K id, const V address);

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
     * Clears the Cache
     */
    void clear();

    /**
     * Default maximum number of elements within the cache
     */
    const static uint32_t DEFAULT_CAPACITY = 4096;

private:
    /**
     * Maps: K -> BlockInfo
     */
    std::unordered_map<K, BlockInfo<V>> mMap;
    /**
     * Maps: Generation -> K
     */
    std::unordered_multimap<uint16_t, K> mGenMap;

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
