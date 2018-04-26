/*
 * Copyright (C) 2015-2018 The ViaDuck Project
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

#include "UtilTest.h"

TEST_F(UtilTest, testTime) {
    ASSERT_EQ("2018-02-03T21:55:13.160Z", Time(1517694913160).formatIso8601());
    ASSERT_EQ("6 02.03.2018 21:55:13", Time(1517694913160).format("%w %m.%d.%Y %H:%M:%S"));
    ASSERT_EQ("6 02.03.2018 21:55:13", Time(1517694913160).formatFull("%w %m.%d.%Y %H:%M:%S"));
    ASSERT_EQ("6 02.03.2018 21:55:13.160", Time(1517694913160).formatFull("%w %m.%d.%Y %H:%M:%S.%k"));
    ASSERT_EQ("6 02.03.2018 21:55:13.001", Time(1517694913001).formatFull("%w %m.%d.%Y %H:%M:%S.%k"));
    ASSERT_EQ("6 02.03.2018 21:55:13.001+0000", Time(1517694913001).formatFull("%w %m.%d.%Y %H:%M:%S.%k%z"));
}
