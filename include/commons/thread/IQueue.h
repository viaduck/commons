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
#ifndef COMMONS_IQUEUE_H
#define COMMONS_IQUEUE_H

#include <cstddef>

/**
 * General interface for the elements queue with aborts
 *
 * @tparam T Type of elements in the queue
 */
template <typename T>
class IQueue {
public:
    virtual ~IQueue() = default;

    /**
     * Pushes the given value to the end of the queue.
     *
     * @param value Element to add to the end of queue
     */
    virtual void push(const T &value) = 0;
    virtual void push(T &&value) = 0;

    /**
     * Pops the front of the queue
     *
     * @param value Receives the resulting value on success
     * @return False if the queue was empty
     */
    virtual bool pop(T &value) = 0;

    /**
     * Waits on empty queues to pop until abort
     *
     * @param value Receives the resulting value on success
     * @return False if the operation was aborted
     */
    virtual bool pop_wait(T &value) = 0;

    /**
     * Invalidates the queue and stops all operations on it.
     * This will cause pending pop_wait to abort and all future push/pop operations to fail.
     * @return True if the queue was already aborted
     */
    virtual bool abort() = 0;

    /**
     * Clear the queue. The queue will only appear empty after clear if the effects of all producers
     * have already settled for the calling thread.
     */
    virtual void clear() = 0;

    /**
     * Estimates the size of the queue
     *
     * @return Approximate number of queue entries
     */
    virtual size_t sizeApprox() const = 0;
};

#endif //COMMONS_IQUEUE_H
