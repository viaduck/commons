/*
 * Copyright (C) 2015-2023 The ViaDuck Project
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

#ifndef COMMONS_CONNECTIONTEST_H
#define COMMONS_CONNECTIONTEST_H

#include <gtest/gtest.h>

class ConnectionTest : public ::testing::TestWithParam<bool> {
public:
    static bool isBlockingMode() {
        return GetParam();
    }

    template<typename exp_err = int>
    void testConnectBlocking(Connection &conn) {
        if (isBlockingMode()) {
            if (std::is_same_v<exp_err, int>)
                EXPECT_NO_THROW(conn.connect());
            else
                EXPECT_THROW(conn.connect(), exp_err);
        }
    }
    template<typename exp_err = int>
    void testConnectNonBlocking(Connection &conn, const NetworkResult &expected = NetworkResultType::SUCCESS) {
        if (!isBlockingMode()) {
            if (std::is_same_v<exp_err, int>)
                EXPECT_EQ(expected, eventually([&] ()  { return conn.connectNonBlocking(); }));
            else
                EXPECT_THROW(eventually([&] ()  { return conn.connectNonBlocking(); }), exp_err);
        }
    }

    template<typename exp_err = int>
    void testConnect(Connection &conn, const NetworkResult &expected = NetworkResultType::SUCCESS) {
        if (isBlockingMode())
            testConnectBlocking<exp_err>(conn);
        else
            testConnectNonBlocking<exp_err>(conn, expected);
    }

protected:
    // retry non-blocking operation while result is wait/retry
    template<typename Rep = int64_t, typename Period = std::milli>
    NetworkResult eventually(const std::function<NetworkResult()> &fun,
            const std::chrono::duration<Rep, Period> timeout = std::chrono::milliseconds(3000)) {
        NetworkResult result = NetworkResultType::WAIT_EVENT;
        int i = 0, limit = 1000;

        while (i++ < limit && result.isDeferred()) {
            result = fun();

            std::this_thread::sleep_for(timeout / limit);
        }

        if (i == limit && result.isDeferred())
            return NetworkTimeoutError();
        return result;
    }
};


#endif //COMMONS_CONNECTIONTEST_H
