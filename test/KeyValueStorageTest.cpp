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
    ASSERT_TRUE(testContainer.get<String>("test1", [&] (const String&) -> bool {
        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5u, count);

    count = 0;
    ASSERT_TRUE(testContainer.get<String>("test2", [&] (const String&) -> bool {
        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3u, count);
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
    ASSERT_TRUE(dContainer.get<String>("test1", [&] (const String&) -> bool {
        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5u, count);

    count = 0;
    ASSERT_TRUE(dContainer.get<String>("test2", [&] (const String&) -> bool {
        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3u, count);
}

TEST_F(KeyValueStorageTest, testSingle) {
    KeyValueStorage testContainer;
    Buffer fallback, fallback_mod;
    fallback.append(String("fallback"));
    fallback_mod.append(String("fallbackbla"));

    // write 5 values with one key
    testContainer.set<Buffer>("test1", String("test1"));
    testContainer.set<Buffer>("test1", String("test2"));
    testContainer.set<Buffer>("test1", String("test3"));
    testContainer.set<Buffer>("test1", String("test4"));
    testContainer.set<Buffer>("test1", String("test5"));

    // write 3 values with other key
    testContainer.set<Buffer>("test2", String("test1"));
    testContainer.set<Buffer>("test2", String("test2"));
    testContainer.set<Buffer>("test2", String("test3"));

    // ensure the value does not exist
    ASSERT_EQ(nullptr, testContainer.get<Buffer>("test32"));

    // ensure that the fallback is returned
    Buffer* result = testContainer.get<Buffer>("test32", &fallback);
    ASSERT_EQ(fallback, *result);

    // modify buffer
    result->append("bla", 3);

    // ensure that the fallback was actually set as value
    ASSERT_EQ(fallback_mod, *testContainer.get<Buffer>("test32"));

    // ensure that getting a single value when multiple values are present fails
    ASSERT_EQ(nullptr, testContainer.get<Buffer>("test1"));
    ASSERT_EQ(nullptr, testContainer.get<Buffer>("test2"));
}
