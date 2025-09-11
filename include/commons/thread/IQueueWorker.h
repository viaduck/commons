/*
 * Copyright (C) 2019-2025 The ViaDuck Project
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

#include <commons/log/Log.h>
#include <commons/thread/IQueue.h>
#include <functional>
#include <thread>

#define TAG_QUEUE_WORKER "[Thread/" << std::this_thread::get_id() << \
    (mName.empty() ? "" : "/" +  mName) + "] "

/**
 * Threaded worker with work queue
 *
 * @tparam W Type of work in the queue
 */
template <typename W>
class IQueueWorker {
public:
    /// create worker with specified queue
    template <template<class> class Q>
    explicit IQueueWorker(Q<W> *queue) : IQueueWorker("", queue) { }
    /// create worker with specified thread name and queue
    template <template<class> class Q>
    explicit IQueueWorker(const std::string &name, Q<W> *queue) : mName(name), mQueue(queue) { }

    /**
     * Move constructor
     */
    IQueueWorker(IQueueWorker &&) noexcept = default;

    /**
     * Destructs a worker
     */
    virtual ~IQueueWorker() {
        /*
         * There is no proper way to stop the thread here without causing a bunch of lifetime issues.
         * Require that the thread was already stopped and the thread ended by the time this destructor is called.
         */
        if (!mQueue->abort() || threadActive()) {
            Log::err << "IQueueWorker destructor called while thread was still active";
            std::terminate();
        }
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
    void enqueue(const W &work) {
        mQueue->push(work);
    }
    void enqueue(W &&work) {
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
    virtual void threadEntry() {
        threadInit();

        W value;
        while (mQueue->pop_wait(value)) {
            try {
                doWork(value);
            } catch (const std::exception &e) {
                Log::err << TAG_QUEUE_WORKER << "Thread caught exception: " << e.what();
            } catch (...) {
                Log::err << TAG_QUEUE_WORKER << "Thread caught unspecified error";
            }
        }

        threadRelease();
    }

    void threadInit() const {
        Log::trac << TAG_QUEUE_WORKER << "Thread init";

        // some impls require per-thread init
        if (mInitThread)
            mInitThread();
    }
    void threadRelease() const {
        // some impls require per-thread resources release
        if (mReleaseThread)
            mReleaseThread();

        Log::trac << TAG_QUEUE_WORKER << "Thread release";
    }

    // mandatory work processing
    virtual void doWork(const W &value) = 0;

    // internal thread name for debugging
    std::string mName;

    // internal work thread
    std::thread mThread;
    // internal work queue
    std::unique_ptr<IQueue<W>> mQueue;

    // optional per-thread init/release hooks
    std::function<void()> mInitThread, mReleaseThread;
};

#endif //COMMONS_QUEUEWORKER_H
