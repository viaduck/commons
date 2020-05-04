/*
 * Copyright (C) 2020 The ViaDuck Project
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
#ifndef COMMONS_THREADLOCAL_H
#define COMMONS_THREADLOCAL_H

#include <thread>
#include <shared_mutex>
#include <unordered_map>
#include <functional>

template<typename T>
class ThreadLocal {
    using factory_t = std::function<T()>;
public:
    ThreadLocal() : mFactory() { }
    explicit ThreadLocal(const factory_t &factory) : mFactory(factory)  { }

    T &load() {
        auto tid = std::this_thread::get_id();
        std::shared_lock<std::shared_timed_mutex> lock(mMutex);

        // if factory set, create object just-in-time
        if (mFactory && mMap.count(tid) == 0) {
            lock.unlock();
            store(mFactory());
            lock.lock();
        }

        // throw if not found
        return mMap.at(tid);
    }

    void store(const T &value) {
        auto tid = std::this_thread::get_id();
        std::unique_lock<std::shared_timed_mutex> lock(mMutex);

        mMap[tid] = value;
    }

    void store(T &&value) {
        auto tid = std::this_thread::get_id();
        std::unique_lock<std::shared_timed_mutex> lock(mMutex);

        mMap[tid] = std::move(value);
    }

    ThreadLocal<T> &operator =(T &&rhs) {
        store(std::move(rhs));
        return *this;
    }

    ThreadLocal<T> &operator =(const T &rhs) {
        store(rhs);
        return *this;
    }

protected:
    factory_t mFactory;
    std::shared_timed_mutex mMutex;
    std::unordered_map<std::thread::id, T> mMap;
};

#endif //COMMONS_THREADLOCAL_H
