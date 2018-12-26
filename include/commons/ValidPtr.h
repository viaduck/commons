/*
 * Copyright (C) 2015-2018 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMONS_VALIDPTR_H
#define COMMONS_VALIDPTR_H

#include <mutex>
#include <set>
#include <algorithm>

#define LOCK_SCOPE(x) std::lock_guard<std::mutex> _scope_lock_(x);

class ValidObjectListener {
public:
    virtual ~ValidObjectListener() = default;
    virtual void onDestroy() = 0;
};

template <typename T>
class ValidPtr : public ValidObjectListener {
public:
    /**
     * Construct a pointer with lifetime monitoring
     *
     * @param instance ValidObject to be monitored
     */
    explicit ValidPtr(T* instance) : mInstance(instance) {
        instance->addListener(this);
    }

    /**
     * Destruct the pointer. Releases the object
     */
    ~ValidPtr() override {
        if (mInstance && mValid)
            mInstance->removeListener(this);
    }

    /**
     * Called by monitored object on destruction.
     */
    void onDestroy() override {
        // prevent destruction while object is being used
        LOCK_SCOPE(mUseMutex);

        mValid = false;
    }

    /**
     * Use this lock to access the object. The object will not be destroyed while the lock is held.
     *
     * @return Unique lock ensuring the objects validity for the lock lifetime
     */
    inline std::unique_lock<std::mutex> lockUse() {
        return std::unique_lock<std::mutex>(mUseMutex);
    }

    // use while locked only

    bool valid() const {
        return mValid;
    }

    inline T* get() {
        return mInstance;
    }

    inline T* operator->() {
        return mInstance;
    }

private:
    // monitored object
    T* mInstance;
    // whether object is still valid
    bool mValid = true;
    // mutex for object use
    std::mutex mUseMutex;
};

/**
 * Object with lifetime monitoring
 */
class ValidObject {
public:
    virtual ~ValidObject() {
        LOCK_SCOPE(mMutex);

        // notify all listeners of destruction
        for (auto &listener : mListeners)
            listener->onDestroy();
    }

    /**
     * Sets a listener to this object's lifetime events
     */
    void addListener(ValidObjectListener *p) {
        LOCK_SCOPE(mMutex);

        mListeners.insert(p);
    }

    /**
     * Removes a listener
     */
    void removeListener(ValidObjectListener *p) {
        LOCK_SCOPE(mMutex);

        mListeners.erase(p);
    }

protected:
    // list of listeners: insert is O(log(n)), erase O(1)
    std::set<ValidObjectListener*> mListeners;
    // ensures list of listeners is not modified while being used
    std::mutex mMutex;
};

#endif //COMMONS_VALIDPTR_H
