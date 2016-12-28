//
// Created by John Watson on 18.03.16.
//

#include "KeyValueStorageTest.h"
#include <libCom/KeyValueStorage.h>

TEST_F(KeyValueStorageTest, GetCallback) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    std::vector<String> test1Values = {"1", "2", "3", "4", "5"};
    testContainer.setSerializable("test1", test1Values[0]);
    testContainer.setSerializable("test1", test1Values[1]);
    testContainer.setSerializable("test1", test1Values[2]);
    testContainer.setSerializable("test1", test1Values[3]);
    testContainer.setSerializable("test1", test1Values[4]);

    // write 3 values with other key
    std::vector<String> test2Values = {"abc", "def", "ghi"};
    testContainer.setSerializable("test2", test2Values[0]);
    testContainer.setSerializable("test2", test2Values[1]);
    testContainer.setSerializable("test2", test2Values[2]);

    // write 1 value with another key
    testContainer.setSerializable<String>("test3", "lonely");

    uint32_t count = 0;
    ASSERT_TRUE(testContainer.getSerializables<String>("test1", [&] (const String &str) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), str);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5u, count);

    count = 0;
    ASSERT_TRUE(testContainer.getSerializables<String>("test2", [&] (const String &str) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), str);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3u, count);

    count = 0;
    ASSERT_TRUE(testContainer.getSerializables<String>("test3", [&] (const String& str) -> bool {
        EXPECT_EQ(str, "lonely");

        count ++;
        return true;
    }));

    // ensure that 1 value was found
    ASSERT_EQ(1u, count);
}

TEST_F(KeyValueStorageTest, GetUnique) {
    KeyValueStorage testContainer;
    String output;

    testContainer.setSerializable<String>("2vals", "abc");
    testContainer.setSerializable<String>("2vals", "123");
    testContainer.setSerializable<String>("1val", "123");

    EXPECT_FALSE(testContainer.getSerializable<String>("noval", output));

    // "2vals" returns false, since it is not a unique result
    EXPECT_FALSE(testContainer.getSerializable<String>("2vals", output));

    // "2vals" returns any value since key is not unique
    EXPECT_TRUE(testContainer.getSerializable<String>("2vals", output, false));
    EXPECT_TRUE(output == String("abc") || output == String("123"));

    EXPECT_TRUE(testContainer.getSerializable<String>("1val", output));
    EXPECT_EQ(String("123"), output);
}

TEST_F(KeyValueStorageTest, GetUniqueFallback) {
    KeyValueStorage testContainer;

    testContainer.setSerializable<String>("2vals", "abc");
    testContainer.setSerializable<String>("2vals", "123");
    testContainer.setSerializable<String>("1val", "123");

    String fallback("fall"), output;
    // key not present -> fallback must have been inserted
    EXPECT_TRUE(testContainer.getSetSerializable("noval", output, fallback));
    EXPECT_TRUE(testContainer.getSerializable("noval", output));     // key present now

    // "2vals" returns false, since it is not a unique result
    EXPECT_FALSE(testContainer.getSerializable<String>("2vals", output));

    // "2vals" returns any value since key is not unique
    EXPECT_TRUE(testContainer.getSetSerializable("2vals", output, fallback, false));
    EXPECT_TRUE(output == String("abc") || output == String("123"));

    EXPECT_TRUE(testContainer.getSetSerializable("1val", output, fallback));
    EXPECT_EQ(String("123"), output);
}

TEST_F(KeyValueStorageTest, SetUnique) {
    std::vector<String> test1Values = {"somethingNew"};
    std::vector<String> test2Values = {"abc", "123"};

    KeyValueStorage testContainer;
    testContainer.setSerializable<String>("1val", "123");
    testContainer.setSerializable<String>("2vals", "abc");
    testContainer.setSerializable<String>("2vals", "123");

    // overwrite
    ASSERT_TRUE(testContainer.setSerializable("1val", String("somethingNew"), true));

    uint32_t count = 0;
    EXPECT_TRUE(testContainer.getSerializables<String>("1val", [&] (const String &str) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), str);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count ++;
        return true;
    }));
    ASSERT_EQ(1u, count);
    ASSERT_EQ(0u, test1Values.size());

    // overwrite fails since key has 2 asssoicated values
    ASSERT_FALSE(testContainer.setSerializable("2vals", String("456"), true));

    count = 0;
    EXPECT_TRUE(testContainer.getSerializables<String>("2vals", [&] (const String &str) -> bool {
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
    EXPECT_TRUE(kvs.setValue<int>("ints", 123));
    EXPECT_TRUE(kvs.setValue<int>("ints", 456));
    EXPECT_TRUE(kvs.setValue<int>("ints", 1337));
    EXPECT_TRUE(kvs.setValue<float>("floats", 3.14159e+10f));
    EXPECT_TRUE(kvs.setValue<float>("floats", 1.0f));
    EXPECT_TRUE(kvs.setValue<double>("doubles", 1.133742e42));
    EXPECT_TRUE(kvs.setValue<double>("doubles", 0.0));

    count = 0;
    EXPECT_TRUE(kvs.getValues<int>("ints", [&] (const int &i) -> bool {
        bool containsValue = false;
        for (auto k : ints) containsValue |= (k == i);
        EXPECT_TRUE(containsValue);           // values must be in values list

        count++;
        return true;
    }));
    EXPECT_EQ(ints.size(), count);

    count = 0;
    EXPECT_TRUE(kvs.getValues<float>("floats", [&] (const float &i) -> bool {
        bool containsValue = false;
        for (auto k : floats) containsValue |= (k == i);
        EXPECT_TRUE(containsValue);           // values must be in values list

        count++;
        return true;
    }));
    EXPECT_EQ(floats.size(), count);

    count = 0;
    EXPECT_TRUE(kvs.getValues<double>("doubles", [&] (const double &i) -> bool {
        bool containsValue = false;
        for (auto k : doubles) containsValue |= (k == i);
        EXPECT_TRUE(containsValue);           // values must be in values list

        count++;
        return true;
    }));
    EXPECT_EQ(doubles.size(), count);
}

TEST_F(KeyValueStorageTest, Serialize) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    testContainer.setSerializable<String>("test1", "test1");
    testContainer.setSerializable<String>("test1", "test2");
    testContainer.setSerializable<String>("test1", "test3");
    testContainer.setSerializable<String>("test1", "test4");
    testContainer.setSerializable<String>("test1", "test5");

    // write 3 values with other key
    testContainer.setSerializable<String>("test2", "test1");
    testContainer.setSerializable<String>("test2", "test2");
    testContainer.setSerializable<String>("test2", "test3");

    Buffer testBuf;
    testContainer.serialize(testBuf);

    KeyValueStorage dContainer;
    ASSERT_TRUE(dContainer.deserialize(testBuf));

    uint32_t count = 0;
    ASSERT_TRUE(dContainer.getSerializables<String>("test1", [&] (const String&) -> bool {
        count++;
        return true;
    }));

    // ensure that 5 values were found
    ASSERT_EQ(5u, count);

    count = 0;
    ASSERT_TRUE(dContainer.getSerializables<String>("test2", [&] (const String&) -> bool {
        count ++;
        return true;
    }));

    // ensure that 3 values were found
    ASSERT_EQ(3u, count);
}

TEST_F(KeyValueStorageTest, Buffer) {
    KeyValueStorage testContainer;
    Buffer fallback, fallback_mod;
    fallback.append(String("fallback"));
    fallback_mod.append(String("fallbackbla"));

    // write 5 values with one key
    testContainer.setBuffer("test1", String("test1"));
    testContainer.setBuffer("test1", String("test2"));
    testContainer.setBuffer("test1", String("test3"));
    testContainer.setBuffer("test1", String("test4"));
    testContainer.setBuffer("test1", String("test5"));

    // write 3 values with other key
    testContainer.setBuffer("test2", String("test1"));
    testContainer.setBuffer("test2", String("test2"));
    testContainer.setBuffer("test2", String("test3"));

    Buffer output;

    // ensure the value does not exist
    ASSERT_FALSE(testContainer.getBuffer("test32", output));

    // ensure that the fallback is returned
    testContainer.getSetBuffer("test32", output, fallback);
    ASSERT_EQ(fallback, output);

    // modify buffer
    ASSERT_TRUE(testContainer.modifyBuffers("test32", [&] (Buffer &result) {
        result.append("bla", 3);
        return true;
    }));

    // ensure that the fallback was actually set as value
    ASSERT_TRUE(testContainer.getBuffer("test32", output));
    ASSERT_EQ(fallback_mod, output);

    // TODO: the behavior changed and these tests should be rewritten
    // ensure that getting a single value when multiple values are present fails
    //ASSERT_FALSE(testContainer.getBuffer("test1", output));
    //ASSERT_FALSE(testContainer.getBuffer("test2", output));
}
