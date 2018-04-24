#include "UtilTest.h"

TEST_F(UtilTest, testTime) {
    ASSERT_EQ("2018-02-03T21:55:13.160Z", Time(1517694913160).formatIso8601());
    ASSERT_EQ("6 02.03.2018 21:55:13", Time(1517694913160).format("%w %m.%d.%Y %H:%M:%S"));
    ASSERT_EQ("6 02.03.2018 21:55:13", Time(1517694913160).formatFull("%w %m.%d.%Y %H:%M:%S"));
    ASSERT_EQ("6 02.03.2018 21:55:13.160", Time(1517694913160).formatFull("%w %m.%d.%Y %H:%M:%S.%k"));
    ASSERT_EQ("6 02.03.2018 21:55:13.001", Time(1517694913001).formatFull("%w %m.%d.%Y %H:%M:%S.%k"));
    ASSERT_EQ("6 02.03.2018 21:55:13.001+0000", Time(1517694913001).formatFull("%w %m.%d.%Y %H:%M:%S.%k%z"));
}
