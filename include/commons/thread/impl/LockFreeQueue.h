/*
 * Copyright (C) 2019-2023 The ViaDuck Project
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
#ifndef COMMONS_LOCKFREEMESSAGEQUEUE_H
#define COMMONS_LOCKFREEMESSAGEQUEUE_H

#include <commons/thread/IQueue.h>

#include <blockingconcurrentqueue.h>
using namespace moodycamel;

/**
 * Lock free queue implementation of the IQueue interface.
 * Note: designed for multi-consumer, multi-producer thread models
 *
 * @tparam T Element type
 */
template <typename T>
class LockFreeQueue : public IQueue<T> {
public:
    void push(const T &value) override {
        // this will wake up pop_wait
        T const &vv = value;
        mQueue.enqueue(vv);
    }
    void push(T &&value) override {
        // this will wake up pop_wait
        mQueue.enqueue(value);
    }

    bool pop(T &value) override {
        return mQueue.try_dequeue(value);
    }

    bool pop_wait(T &value) override {
        // wait indefinitely for any value
        mQueue.wait_dequeue(value);

        // true on success
        return !mAborted.load();
    }

    bool abort() override {
        // already aborted
        if (mAborted.load())
            return true;

        // set aborted flag
        mAborted.store(true);
        // wake up pop_wait with empty control message
        mQueue.enqueue(T());
        return false;
    }

    void clear() override {
        T ignored;
        while (mQueue.try_dequeue(ignored));
    }

    size_t sizeApprox() const override {
        return mQueue.size_approx();
    }

protected:
    BlockingConcurrentQueue<T> mQueue;
    std::atomic_bool mAborted = ATOMIC_VAR_INIT(false);
};

#endif //COMMONS_LOCKFREEMESSAGEQUEUE_H
