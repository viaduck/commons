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

TEST_F(KeyValueStorageTest, testSingle) {
    KeyValueStorage testContainer;
    String fallback("fallback");

    // write 5 values with one key
    testContainer.setSingle<String>("test1", "test1");
    testContainer.setSingle<String>("test1", "test2");
    testContainer.setSingle<String>("test1", "test3");
    testContainer.setSingle<String>("test1", "test4");
    testContainer.setSingle<String>("test1", "test5");

    // write 3 values with other key
    testContainer.setSingle<String>("test2", "test1");
    testContainer.setSingle<String>("test2", "test2");
    testContainer.setSingle<String>("test2", "test3");

    // ensure the value does not exist
    ASSERT_EQ(nullptr, testContainer.getSingle<String>("test32"));

    // ensure that the fallback is returned
    ASSERT_EQ(fallback, *testContainer.getSingle<String>("test32", &fallback));

    // ensure that the fallback was actually set as value
    ASSERT_EQ(fallback, *testContainer.getSingle<String>("test32"));

    // ensure the values of key overwritten multiple times actually changed
    ASSERT_EQ(String("test5"), *testContainer.getSingle<String>("test1"));
    ASSERT_EQ(String("test3"), *testContainer.getSingle<String>("test2"));
}
