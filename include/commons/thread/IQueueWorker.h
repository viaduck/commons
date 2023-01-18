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
#ifndef COMMONS_QUEUEWORKER_H
#define COMMONS_QUEUEWORKER_H

#include <commons/thread/IMessageQueue.h>
#include <thread>

/**
 * Threaded worker with message queue
 *
 * @tparam W Type of messages in the queue (work)
 */
template <typename W>
class IQueueWorker {
public:
    /**
     * Constructs new QueueWorker
     *
     * @tparam Q Type of message queue implementation
     */
    template <template<class> class Q>
    explicit IQueueWorker(Q<W> *queue) : mQueue(queue) { }

    /**
     * Move constructor
     */
    IQueueWorker(IQueueWorker<W> &&) noexcept = default;

    /**
     * Destructs a worker
     */
    virtual ~IQueueWorker() {
        /* Intentionally cause exception if stopThread has not been called before destructor */
        if (mThread.joinable())
            mQueue->abort();
    }

    /**
     * Starts the worker thread
     */
    void startThread() {
        mThread = std::thread(&IQueueWorker::threadEntry, this);
    }

    /**
     * Aborts the queue and waits for thread to quit
     */
    void stopThread() {
        if (!mQueue->abort() && mThread.joinable())
            mThread.join();
    }

    /**
     * Indicates whether the worker thread is active
     */
    bool threadActive() const {
        return mThread.joinable();
    }

    /**
     * Enqueues work into queue
     *
     * @param work Work to process in thread. Takes pointer ownership
     */
    void enqueue(W *work) {
        mQueue->push(work);
    }

    /**
     * Approximates the size of the queue
     *
     * @return Approximate queue size
     */
    size_t sizeApprox() {
        return mQueue->sizeApprox();
    }

protected:
    /**
     * Internal thread entry-point
     */
    void threadEntry() {
        // some impls require per-thread init
        initThread();

        W *value;
        while (mQueue->pop_wait(value)) {
            std::unique_ptr<W> lifetime(value);
            doWork(value);
        }

        releaseThread();
    }

    // optional per-thread platform initialization
    virtual void initThread() { }
    // optional per-thread platform cleanup
    virtual void releaseThread() { }
    // mandatory work processing
    virtual void doWork(const W *value) = 0;

    // internal work thread
    std::thread mThread;
    // internal work queue
    std::unique_ptr<IMessageQueue<W>> mQueue;
};

#endif //COMMONS_QUEUEWORKER_H
