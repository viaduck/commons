//
// Created by John Watson on 18.03.16.
//

#include "KeyValueStorageTest.h"
#include <libCom/KeyValueStorage.h>

TEST_F(KeyValueStorageTest, testSimple) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    testContainer.set("test1", "test1");
    testContainer.set("test1", "test2");
    testContainer.set("test1", "test3");
    testContainer.set("test1", "test4");
    testContainer.set("test1", "test5");

    // write 3 values with other key
    testContainer.set("test2", "test1");
    testContainer.set("test2", "test2");
    testContainer.set("test2", "test3");

    uint32_t count = 0;
    testContainer.get("test1", [&] (String& s) -> bool {
        count++;
        return true;
    });

    // ensure that 5 values were found
    ASSERT_EQ(5, count);

    count = 0;
    testContainer.get("test2", [&] (String& s) -> bool {
        count ++;
        return true;
    });

    // ensure that 3 values were found
    ASSERT_EQ(3, count);
}

TEST_F(KeyValueStorageTest, testSerialize) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    testContainer.set("test1", "test1");
    testContainer.set("test1", "test2");
    testContainer.set("test1", "test3");
    testContainer.set("test1", "test4");
    testContainer.set("test1", "test5");

    // write 3 values with other key
    testContainer.set("test2", "test1");
    testContainer.set("test2", "test2");
    testContainer.set("test2", "test3");

    Buffer testBuf;
    testContainer.serialize(testBuf);

    KeyValueStorage dContainer;
    ASSERT_TRUE(dContainer.deserialize(testBuf));

    uint32_t count = 0;
    dContainer.get("test1", [&] (String& s) -> bool {
        count++;
        return true;
    });

    // ensure that 5 values were found
    ASSERT_EQ(5, count);

    count = 0;
    dContainer.get("test2", [&] (String& s) -> bool {
        count ++;
        return true;
    });

    // ensure that 3 values were found
    ASSERT_EQ(3, count);
}