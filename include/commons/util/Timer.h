/*
 * Copyright (C) 2023 The ViaDuck Project
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

#ifndef COMMONS_TIMER_H
#define COMMONS_TIMER_H

#include <commons/util/Time.h>

/**
 * Provides a thread-safe, cross-platform way of measuring elapsed time with millisecond precision.
 */
class Timer {
public:
    Timer() = default;

    explicit Timer(int64_t durationMillis) {
        start(durationMillis);
    }

    /// start timer with specified number of milliseconds, move to "active" state
    void start(int64_t durationMillis) {
        mStartTime = steadyNow();
        mDurationMillis = durationMillis;
    }

    /// reset timer to "not running" state
    void reset() {
        mStartTime = 0;
        mDurationMillis = 0;
    }

    /// @returns True if timer is in "active" state
    bool active() const {
        return mStartTime > 0;
    }

    /// @returns True if timer is in "active" state and not elapsed
    bool running() const {
        return active() && runningMillis() <= mDurationMillis;
    }

    /// @returns True if timer is in "active" state and elapsed
    bool elapsed() const {
        return active() && runningMillis() > mDurationMillis;
    }

    /// @returns Number of milliseconds since start of timer if "active", otherwise 0.
    int64_t runningMillis() const {
        return active() ? steadyNow() - mStartTime : 0;
    }

    /// @returns Number of milliseconds set as duration during the start of timer.
    int64_t durationMillis() const {
        return mDurationMillis;
    }

    /// @returns Number of milliseconds left until timer elapses, or 0 if elapsed/not "active"
    int64_t leftMillis() const {
        auto elapsed = runningMillis();
        if (active() && elapsed <= mDurationMillis)
            return mDurationMillis - elapsed;

        return 0;
    }

protected:
    static inline int64_t steadyNow() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    int64_t mStartTime = 0;
    int64_t mDurationMillis = 0;
};

#endif //COMMONS_TIMER_H
