/*
 * Copyright (C) 2019 The ViaDuck Project
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
#ifndef COMMONS_LOCKINGMESSAGEQUEUE_H
#define COMMONS_LOCKINGMESSAGEQUEUE_H

#include <commons/thread/IQueue.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

/**
 * Lock based queue implementation of the IQueue interface.
 * Note: handles any number of consumers and producers
 *
 * @tparam T Element type
 */
template <typename T>
class LockingQueue : public IQueue<T> {
public:
    void push(const T &value) override {
        std::unique_lock<std::mutex> lock(mMutex);

        // add value, signal
        mQueue.push(value);
        mCond.notify_one();
    }
    void push(T &&value) override {
        std::unique_lock<std::mutex> lock(mMutex);

        // add value, signal
        mQueue.push(value);
        mCond.notify_one();
    }

    bool pop(T &value) override {
        std::unique_lock<std::mutex> lock(mMutex);

        if (!mQueue.empty()) {
            value = mQueue.front();
            mQueue.pop();
            return true;
        }

        return false;
    }

    bool pop_wait(T &value) override {
        std::unique_lock<std::mutex> lock(mMutex);
        bool aborted = mAborted.load();

        // wait while queue empty and not aborted
        while (mQueue.empty() && !(aborted = mAborted.load()))
            mCond.wait(lock);

        if (!aborted) {
            value = mQueue.front();
            mQueue.pop();
            return true;
        }

        return false;
    }

    bool abort() override {
        // already aborted
        if (mAborted.load())
            return true;

        std::unique_lock<std::mutex> lock(mMutex);
        // set aborted flag
        mAborted.store(true);
        // wake up pop_wait
        mCond.notify_one();
        return false;
    }

    void clear() override {
        std::unique_lock<std::mutex> lock(mMutex);

        while (!mQueue.empty())
            mQueue.pop();
    }

    size_t sizeApprox() const override {
        std::unique_lock<std::mutex> lock(mMutex);

        return mQueue.size();
    }

protected:
    mutable std::mutex mMutex;
    std::queue<T> mQueue;
    std::condition_variable mCond;
    std::atomic_bool mAborted = ATOMIC_VAR_INIT(false);
};

#endif //COMMONS_LOCKINGMESSAGEQUEUE_H
