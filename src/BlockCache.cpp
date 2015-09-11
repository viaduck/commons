//
// Created by steffen on 10.09.15.
//

#include "libCom/BlockCache.h"


BlockInfo::BlockInfo() : address(0) { }

BlockInfo::BlockInfo(const uint32_t address) : address(address) { }

// ##

BlockCache::BlockCache(const uint32_t capacity) : mCapacity(capacity) { }

const uint32_t BlockCache::read(const uint32_t id) const {
    return mMap.find(id) != mMap.end() ? mMap.find(id)->second.address : 0;
}

const uint16_t BlockCache::generation(const uint32_t id) const {
    return mMap.find(id) != mMap.end() ? mMap.find(id)->second.generation : 0;
}

bool BlockCache::write(const uint32_t id, const uint32_t address) {
    // element is present
    auto e = mMap.find(id);
    if (e != mMap.end()) {
        // increase generation
        BlockInfo &block = e->second;
        // find all elements with same generation
        auto range = mGenMap.equal_range(block.generation);
        // sadly we cannot use "for (auto &el: range)" here..
        for (std::unordered_multimap<uint16_t , uint32_t>::iterator it = range.first; it != range.second; ++it)
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
            mMap[id] = BlockInfo(address);
            mGenMap.emplace(1, id);     // add block to generation map
            mLeast = 1;             // just inserted a generation 1 block
        } else {
            auto leastElem = mGenMap.find(mLeast);      // get one least used block id (with lowest generation). There is always one, so leastElem cannot be it::end
            mMap.erase(leastElem->second);     // remove it
            mGenMap.erase(leastElem);           // and remove it form generation map

            mMap[id] = BlockInfo(address);          // insert new block
            mGenMap.emplace(1, id);         // and place the block in generation map
            mLeast = 1;             // just inserted a generation 1 element

            return false;       // we had to remove one block, so return false
        }
    }
    return true;
}
