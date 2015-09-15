//
// Created by steffen on 10.09.15.
//

#include <libCom/BufferRange.h>
#include "libCom/BlockCache.h"

template<class T>
BlockInfo<T>::BlockInfo() : address(0) { }

template<class T>
BlockInfo<T>::BlockInfo(const T address) : address(address) { }

// ##
template<class K, class V>
BlockCache<K, V>::BlockCache(const uint32_t capacity) : mCapacity(capacity) { }

template<class K, class V>
const V BlockCache<K, V>::read(const K id) const {
    return mMap.find(id) != mMap.end() ? mMap.find(id)->second.address : V();
}

template<class K, class V>
const uint16_t BlockCache<K, V>::generation(const K id) const {
    return mMap.find(id) != mMap.end() ? mMap.find(id)->second.generation : 0;
}

template<class K, class V>
bool BlockCache<K, V>::write(const K id, const V address) {
    // element is present
    auto e = mMap.find(id);
    if (e != mMap.end()) {
        // increase generation
        BlockInfo<V> &block = e->second;
        // find all elements with same generation
        auto range = mGenMap.equal_range(block.generation);
        // sadly we cannot use "for (auto &el: range)" here..
        for (typename std::unordered_multimap<uint16_t , K>::iterator it = range.first; it != range.second; ++it)
            if ((*it).second == id) {       // if this is our block, remove it from the generation map...
                mGenMap.erase(it);
                break;
            }

        // if this is a least block
        if (block.generation == mLeast && mGenMap.find(mLeast) == mGenMap.end())       // look if there aren't any other blocks with same generation
                mLeast++;                   // if so, adjust lowest generation

        block.generation++;
        mGenMap.emplace(block.generation, id);      // ... to insert it again in an increased generation
    } else {
        // element is not present -> check capacity
        if (mMap.size() < mCapacity) {        // we can safely insert
            mMap[id] = BlockInfo<V>(address);
            mGenMap.emplace(1, id);     // add block to generation map
            mLeast = 1;             // just inserted a generation 1 block
        } else {
            auto leastElem = mGenMap.find(mLeast);      // get one least used block id (with lowest generation). There is always one, so leastElem cannot be it::end
            mMap.erase(leastElem->second);     // remove it
            mGenMap.erase(leastElem);           // and remove it form generation map

            mMap[id] = BlockInfo<V>(address);          // insert new block
            mGenMap.emplace(1, id);         // and place the block in generation map
            mLeast = 1;             // just inserted a generation 1 element

            return false;       // we had to remove one block, so return false
        }
    }
    return true;
}

void BlockCache::clear() {
    mMap.clear();
    mGenMap.clear();
    mLeast = 0;
}


#include "libCom/BufferRange.h"

template class BlockCache<BufferRange, unsigned int>;
template class BlockCache<unsigned int, unsigned int>;
