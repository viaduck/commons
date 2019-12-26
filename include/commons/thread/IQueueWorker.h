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

template <typename T>
class IQueueWorker {
public:
    explicit IQueueWorker(IMessageQueue<T> *queue) : mQueue(queue) { }

    virtual ~IQueueWorker() {
        stopThread();
        delete mQueue;
    }

    void startThread() {
        mThread = std::thread(&IQueueWorker::threadEntry, this);
    }

    void stopThread() {
        if (!mQueue->abort())
            mThread.join();
    }

    void enqueue(T *work) {
        mQueue->push(work);
    }

protected:
    void threadEntry() {
        // some impls require per-thread init
        initThread();

        T *value;
        while (mQueue->pop_wait(value)) {
            doWork(*value);
            delete value;
        }

        releaseThread();
    }

    virtual void initThread() { }
    virtual void releaseThread() { }
    virtual void doWork(T &value) = 0;

    std::thread mThread;
    IMessageQueue<T> *mQueue;
};

#endif //COMMONS_QUEUEWORKER_H
