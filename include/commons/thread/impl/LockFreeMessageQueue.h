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
#ifndef COMMONS_LOCKFREEMESSAGEQUEUE_H
#define COMMONS_LOCKFREEMESSAGEQUEUE_H

#include <commons/thread/IMessageQueue.h>

#include <blockingconcurrentqueue.h>
using namespace moodycamel;

/**
 * Lock free message queue implementation of the IMessageQueue interface.
 * Note: designed for multi-consumer, multi-producer thread models
 *
 * @tparam M Message type
 */
template <typename M>
class LockFreeMessageQueue : public IMessageQueue<M> {
public:
    ~LockFreeMessageQueue() {
        M *value;
        // delete remaining
        while (mQueue.try_dequeue(value))
            delete value;
    }

    void push(M *value) override {
        // this will wake up pop_wait
        mQueue.enqueue(value);
    }

    bool pop(M *&value) override {
        return mQueue.try_dequeue(value);
    }

    bool pop_wait(M *&value) override {
        bool aborted;

        // wait indefinitely for any value
        mQueue.wait_dequeue(value);
        aborted = mAborted.load();

        // invalidate value if aborted
        if (aborted && value != nullptr) {
            delete value;
            value = nullptr;
        }

        return !aborted;
    }

    bool abort() override {
        // already aborted
        if (mAborted.load())
            return true;

        // set aborted flag
        mAborted.store(true);
        // wake up pop_wait with empty control message
        mQueue.enqueue(nullptr);
        return false;
    }

    size_t sizeApprox() const override {
        return mQueue.size_approx();
    }

protected:
    BlockingConcurrentQueue<M*> mQueue;
    std::atomic_bool mAborted = ATOMIC_VAR_INIT(false);
};

#endif //COMMONS_LOCKFREEMESSAGEQUEUE_H
