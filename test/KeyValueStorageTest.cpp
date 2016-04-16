//
// Created by John Watson on 18.03.16.
//

#include "KeyValueStorageTest.h"
#include <libCom/KeyValueStorage.h>

TEST_F(KeyValueStorageTest, GetCallback) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    std::vector<String> test1Values = {"1", "2", "3", "4", "5"};
    testContainer.set<String>("test1", test1Values[0]);
    testContainer.set<String>("test1", test1Values[1]);
    testContainer.set<String>("test1", test1Values[2]);
    testContainer.set<String>("test1", test1Values[3]);
    testContainer.set<String>("test1", test1Values[4]);

    // write 3 values with other key
    std::vector<String> test2Values = {"abc", "def", "ghi"};
    testContainer.set<String>("test2", test2Values[0]);
    testContainer.set<String>("test2", test2Values[1]);
    testContainer.set<String>("test2", test2Values[2]);

    // write 1 value with another key
    testContainer.set<String>("test3", "lonely");

    uint32_t count = 0;
    ASSERT_TRUE(testContainer.get<String>("test1", [&] (const String &str) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), str);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5u, count);

    count = 0;
    ASSERT_TRUE(testContainer.get<String>("test2", [&] (const String &str) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), str);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3u, count);

    count = 0;
    ASSERT_TRUE(testContainer.get<String>("test3", [&] (const String& str) -> bool {
        EXPECT_EQ(str, "lonely");

        count ++;
        return true;
    }));

    // ensure that 1 value was found
    ASSERT_EQ(1u, count);
}

TEST_F(KeyValueStorageTest, GetUnique) {
    KeyValueStorage testContainer;

    testContainer.set<String>("2vals", "abc");
    testContainer.set<String>("2vals", "123");
    testContainer.set<String>("1val", "123");

    EXPECT_EQ(nullptr, testContainer.get<String>("noval"));

    // "2vals" does not return any value since key is not unique
    EXPECT_EQ(nullptr, testContainer.get<String>("2vals"));

    String *val = testContainer.get<String>("1val");
    EXPECT_NE(nullptr, val);        // value for key exists
    EXPECT_EQ(String("123"), *val);
}

TEST_F(KeyValueStorageTest, GetUniqueFallback) {
    KeyValueStorage testContainer;

    testContainer.set<String>("2vals", "abc");
    testContainer.set<String>("2vals", "123");
    testContainer.set<String>("1val", "123");

    String *fallback1 = new String("fall");
    // key not present -> fallback must have been inserted
    EXPECT_NE(nullptr, testContainer.get<String>("noval", fallback1));
    EXPECT_NE(nullptr, testContainer.get<String>("noval"));     // key present now


    // "2vals" does not return any value since key is not unique
    String *fallback2 = new String("fall");
    EXPECT_EQ(nullptr, testContainer.get("2vals", fallback2));

    String *fallback3 = new String("fall");
    String *val = testContainer.get("1val", fallback3);
    ASSERT_NE(nullptr, val);        // value for key exists
    EXPECT_EQ(String("123"), *val);
}

TEST_F(KeyValueStorageTest, SetUnique) {
    std::vector<String> test1Values = {"somethingNew"};
    std::vector<String> test2Values = {"abc", "123"};

    KeyValueStorage testContainer;
    testContainer.set<String>("1val", "123");
    testContainer.set<String>("2vals", "abc");
    testContainer.set<String>("2vals", "123");

    // overwrite
    ASSERT_TRUE(testContainer.set("1val", String("somethingNew"), true));

    uint32_t count = 0;
    EXPECT_TRUE(testContainer.get<String>("1val", [&] (const String &str) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), str);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count ++;
        return true;
    }));
    ASSERT_EQ(1u, count);
    ASSERT_EQ(0u, test1Values.size());

    // overwrite fails since key has 2 asssoicated values
    ASSERT_FALSE(testContainer.set("2vals", String("456"), true));

    count = 0;
    EXPECT_TRUE(testContainer.get<String>("2vals", [&] (const String &str) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), str);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));
    ASSERT_EQ(2u, count);
}

TEST_F(KeyValueStorageTest, PrimitiveTypes) {
    uint32_t count;
    std::vector<int> ints = {123, 456, 1337};
    std::vector<float> floats = {3.14159e+10f, 1.0f};
    std::vector<double> doubles = {1.133742e42, 0.0};

    KeyValueStorage kvs;
    EXPECT_TRUE(kvs.set<int>("ints", 123));
    EXPECT_TRUE(kvs.set<int>("ints", 456));
    EXPECT_TRUE(kvs.set<int>("ints", 1337));
    EXPECT_TRUE(kvs.set<float>("floats", 3.14159e+10f));
    EXPECT_TRUE(kvs.set<float>("floats", 1.0f));
    EXPECT_TRUE(kvs.set<double>("doubles", 1.133742e42));
    EXPECT_TRUE(kvs.set<double>("doubles", 0.0));

    count = 0;
    EXPECT_TRUE(kvs.get<int>("ints", [&] (const int &i) -> bool {
        auto it = std::find(ints.begin(), ints.end(), i);
        EXPECT_NE(ints.end(), it);           // values must be in values list
        ints.erase(it);
        count++;
        return true;
    }));
    EXPECT_EQ(0u, ints.size());

    count = 0;
    EXPECT_TRUE(kvs.get<float>("floats", [&] (const float &i) -> bool {
        auto it = std::find(floats.begin(), floats.end(), i);
        EXPECT_NE(floats.end(), it);           // values must be in values list
        floats.erase(it);
        count++;
        return true;
    }));
    EXPECT_EQ(0u, floats.size());

    count = 0;
    EXPECT_TRUE(kvs.get<double>("doubles", [&] (const double &i) -> bool {
        auto it = std::find(doubles.begin(), doubles.end(), i);
        EXPECT_NE(doubles.end(), it);           // values must be in values list
        doubles.erase(it);
        count++;
        return true;
    }));
    EXPECT_EQ(0u, doubles.size());
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
