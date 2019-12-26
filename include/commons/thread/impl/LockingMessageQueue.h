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

#include <commons/thread/IMessageQueue.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <condition_variable>

template <typename M>
class LockingMessageQueue : public IMessageQueue<M> {
public:
    ~LockingMessageQueue() {
        std::unique_lock<std::mutex> lock(mMutex);
        while (!mQueue.empty()) {
            delete mQueue.front();
            mQueue.pop();
        }
    }

    void push(M *value) override {
        // lock, add value, signal
        std::unique_lock<std::mutex> lock(mMutex);
        mQueue.push(value);
        mCond.notify_one();
    }

    bool pop(M *&value) override {
        std::unique_lock<std::mutex> lock(mMutex);
        if (!mQueue.empty()) {
            value = mQueue.front();
            mQueue.pop();
            return true;
        }
        return false;
    }

    bool pop_wait(M *&value) override {
        std::unique_lock<std::mutex> lock(mMutex);

        bool aborted = false;
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
        std::unique_lock<std::mutex> lock(mMutex);

        if (mAborted.load())
            return true;

        mAborted.store(true);
        mCond.notify_one();
        return false;
    }

protected:
    std::mutex mMutex;
    std::queue<M*> mQueue;
    std::atomic_bool mAborted = ATOMIC_VAR_INIT(false);
    std::condition_variable mCond;
};

#endif //COMMONS_LOCKINGMESSAGEQUEUE_H
