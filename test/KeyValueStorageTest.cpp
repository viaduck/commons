//
// Created by John Watson on 18.03.16.
//

#include "KeyValueStorageTest.h"
#include <libCom/KeyValueStorage.h>

TEST_F(KeyValueStorageTest, testSimple) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    testContainer.set<String>("test1", "test1");
    testContainer.set<String>("test1", "test2");
    testContainer.set<String>("test1", "test3");
    testContainer.set<String>("test1", "test4");
    testContainer.set<String>("test1", "test5");

    // write 3 values with other key
    testContainer.set<String>("test2", "test1");
    testContainer.set<String>("test2", "test2");
    testContainer.set<String>("test2", "test3");

    uint32_t count = 0;
    ASSERT_TRUE(testContainer.get<String>("test1", [&] (const String& s) -> bool {
        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5, count);

    count = 0;
    ASSERT_TRUE(testContainer.get<String>("test2", [&] (const String& s) -> bool {
        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3, count);
}

TEST_F(KeyValueStorageTest, testSerialize) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    testContainer.set<String>("test1", "test1");
    testContainer.set<String>("test1", "test2");
    testContainer.set<String>("test1", "test3");
    testContainer.set<String>("test1", "test4");
    testContainer.set<String>("test1", "test5");

    // write 3 values with other key
    testContainer.set<String>("test2", "test1");
    testContainer.set<String>("test2", "test2");
    testContainer.set<String>("test2", "test3");

    Buffer testBuf;
    testContainer.serialize(testBuf);

    KeyValueStorage dContainer;
    ASSERT_TRUE(dContainer.deserialize(testBuf));

    uint32_t count = 0;
    ASSERT_TRUE(dContainer.get<String>("test1", [&] (const String& s) -> bool {
        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5, count);

    count = 0;
    ASSERT_TRUE(dContainer.get<String>("test2", [&] (const String& s) -> bool {
        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3, count);
}