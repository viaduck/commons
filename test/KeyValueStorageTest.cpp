//
// Created by John Watson on 18.03.16.
//

#include "KeyValueStorageTest.h"
#include "custom_assert.h"
#include <libCom/KeyValueStorage.h>

TEST_F(KeyValueStorageTest, Primitive) {
    uint32_t count;
    std::vector<int> ints = {123, 456, 1337};
    std::vector<float> floats = {3.14159e+10f, 1.0f};
    std::vector<double> doubles = {1.133742e42, 0.0};

    KeyValueStorage kvs;
    // unique values
    EXPECT_TRUE(kvs.setValue<int>("int1", ints[0]));
    EXPECT_TRUE(kvs.setValue<int>("int2", ints[1]));
    EXPECT_TRUE(kvs.setValue<int>("int3", ints[2]));
    EXPECT_TRUE(kvs.setValue<float>("float1", floats[0]));
    EXPECT_TRUE(kvs.setValue<float>("float2", floats[1]));
    EXPECT_TRUE(kvs.setValue<double>("double1", doubles[0]));
    EXPECT_TRUE(kvs.setValue<double>("double2", doubles[1]));

    // check if correctly stored
    // .. if uniquely enforced
    EXPECT_EQ(ints[0], kvs.getValue<int>("int1"));
    EXPECT_EQ(ints[1], kvs.getValue<int>("int2"));
    EXPECT_EQ(ints[2], kvs.getValue<int>("int3"));
    EXPECT_EQ(floats[0], kvs.getValue<float>("float1"));
    EXPECT_EQ(floats[1], kvs.getValue<float>("float2"));
    EXPECT_EQ(doubles[0], kvs.getValue<double>("double1"));
    EXPECT_EQ(doubles[1], kvs.getValue<double>("double2"));

    // .. if not uniquely enforced
    EXPECT_EQ(ints[0], kvs.getValue<int>("int1", false));
    EXPECT_EQ(ints[1], kvs.getValue<int>("int2", false));
    EXPECT_EQ(ints[2], kvs.getValue<int>("int3", false));
    EXPECT_EQ(floats[0], kvs.getValue<float>("float1", false));
    EXPECT_EQ(floats[1], kvs.getValue<float>("float2", false));
    EXPECT_EQ(doubles[0], kvs.getValue<double>("double1", false));
    EXPECT_EQ(doubles[1], kvs.getValue<double>("double2", false));


    // multiple values
    EXPECT_TRUE(kvs.setValue<int>("ints", 123));
    EXPECT_TRUE(kvs.setValue<int>("ints", 456));
    EXPECT_TRUE(kvs.setValue<int>("ints", 1337));
    EXPECT_TRUE(kvs.setValue<float>("floats", 3.14159e+10f));
    EXPECT_TRUE(kvs.setValue<float>("floats", 1.0f));
    EXPECT_TRUE(kvs.setValue<double>("doubles", 1.133742e42));
    EXPECT_TRUE(kvs.setValue<double>("doubles", 0.0));

    EXPECT_THROW(kvs.getValue<int>("ints"), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";
    EXPECT_THROW(kvs.getValue<float>("floats"), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";
    EXPECT_THROW(kvs.getValue<double>("doubles"), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";

    // returned value must be any of list
    EXPECT_ANY_OF(ints, kvs.getValue<int>("ints", false));
    EXPECT_ANY_OF(floats, kvs.getValue<float>("floats", false));
    EXPECT_ANY_OF(doubles, kvs.getValue<double>("doubles", false));

    // callback check
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

TEST_F(KeyValueStorageTest, PrimitiveFallback) {
    std::vector<int> ints = {123, 456, 1337};
    std::vector<float> floats = {3.14159e+10f, 1.0f};
    std::vector<double> doubles = {1.133742e42, 0.0};

    KeyValueStorage kvs;

    // store with fallback option
    EXPECT_EQ(ints[0], kvs.getSetValue<int>("int1", ints[0]));
    EXPECT_EQ(ints[1], kvs.getSetValue<int>("int2", ints[1]));
    EXPECT_EQ(ints[2], kvs.getSetValue<int>("int3", ints[2]));
    EXPECT_EQ(floats[0], kvs.getSetValue<float>("floats1", floats[0]));
    EXPECT_EQ(floats[1], kvs.getSetValue<float>("floats2", floats[1]));
    EXPECT_EQ(doubles[0], kvs.getSetValue<double>("doubles1", doubles[0]));
    EXPECT_EQ(doubles[1], kvs.getSetValue<double>("doubles2", doubles[1]));

    // must exist now, new fallback must not be inserted
    EXPECT_EQ(ints[0], kvs.getSetValue<int>("int1", ints[0]+2));
    EXPECT_EQ(ints[1], kvs.getSetValue<int>("int2", ints[1]+2));
    EXPECT_EQ(ints[2], kvs.getSetValue<int>("int3", ints[2]+2));
    EXPECT_EQ(floats[0], kvs.getSetValue<float>("floats1", floats[0]+.5f));
    EXPECT_EQ(floats[1], kvs.getSetValue<float>("floats2", floats[1]+.5f));
    EXPECT_EQ(doubles[0], kvs.getSetValue<double>("doubles1", doubles[0]+.32));
    EXPECT_EQ(doubles[1], kvs.getSetValue<double>("doubles2", doubles[1]+.32));

    // multiple values
    EXPECT_TRUE(kvs.setValue<int>("ints", 123));
    EXPECT_TRUE(kvs.setValue<int>("ints", 456));
    EXPECT_TRUE(kvs.setValue<int>("ints", 1337));
    EXPECT_TRUE(kvs.setValue<float>("floats", 3.14159e+10f));
    EXPECT_TRUE(kvs.setValue<float>("floats", 1.0f));
    EXPECT_TRUE(kvs.setValue<double>("doubles", 1.133742e42));
    EXPECT_TRUE(kvs.setValue<double>("doubles", 0.0));

    // enforced unique
    EXPECT_THROW(kvs.getSetValue<int>("ints", ints[0]), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";
    EXPECT_THROW(kvs.getSetValue<float>("floats", floats[0]), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";
    EXPECT_THROW(kvs.getSetValue<double>("doubles", doubles[0]), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";

    // not enforced unique
    std::vector<int> newInt({ints[0], ints[1], ints[2], ints[0]+4});
    std::vector<float> newFloat({floats[0], floats[1], floats[0]+.5f});
    std::vector<double> newDouble({doubles[0], doubles[1], doubles[0]+.32});
    EXPECT_ANY_OF(newInt, kvs.getSetValue<int>("ints", ints[0]+4, false));
    EXPECT_ANY_OF(newFloat, kvs.getSetValue<float>("floats", floats[0]+.5f, false));
    EXPECT_ANY_OF(newDouble, kvs.getSetValue<double>("doubles", doubles[0]+.32, false));
}

TEST_F(KeyValueStorageTest, PrimitiveReplace) {
    KeyValueStorage kvs;
    std::vector<int> ints = {123, 456, 1337};

    // unique
    EXPECT_TRUE(kvs.setValue<int>("int1", ints[0]));
    EXPECT_TRUE(kvs.setValue<int>("int1", ints[1], true));
    EXPECT_EQ(ints[1], kvs.getValue<int>("int1")) << ints[0] << " must have been replaced to " << ints[1];

    // multiple
    EXPECT_TRUE(kvs.setValue<int>("int1", ints[2]));            // kvs contains ints[1] and ints[2] now
    EXPECT_FALSE(kvs.setValue<int>("int1", ints[0], true)) << "Must not overwrite if there are >= 2 values";
}

TEST_F(KeyValueStorageTest, PrimitiveNonExistent) {
    KeyValueStorage kvs;

    EXPECT_THROW(kvs.getValue<int>("nonexistentkey"), std::invalid_argument);
    EXPECT_FALSE(kvs.getValues<int>("nonexistentkey", [] (const int&) { return true; }));
}

TEST_F(KeyValueStorageTest, PrimitiveModify) {
    KeyValueStorage kvs;
    std::vector<int> ints = {123, 456, 1337};

    kvs.setValue<int>("int1", ints[0]);
    kvs.setValue<int>("int2", ints[1]);
    kvs.setValue<int>("int2", ints[2]);

    // ## existing 1-value key ##
    EXPECT_TRUE(kvs.modifyValues<int>("int1", [&] (int &val) {
        EXPECT_EQ(ints[0], val);
        val = 99;
        return true;
    }));
    EXPECT_EQ(99, kvs.getValue<int>("int1")) << ints[0] << " must have been modified to " << 99;

    EXPECT_TRUE(kvs.modifyValues<int>("int1", [&] (int &val) {
        EXPECT_EQ(99, val);
        return true;
    }));
    EXPECT_EQ(99, kvs.getValue<int>("int1")) << "Must not overwrite " << 99;

    // ## existing n-value key ##
    std::vector<int> toFind = {456, 1337};
    EXPECT_TRUE(kvs.modifyValues<int>("int2", [&] (int &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());

        val += 25;
        return true;
    }));
    EXPECT_EQ(0u, toFind.size()) << "Did not call modify-callback enough times";

    // check if really replaced
    toFind = {456+25, 1337+25};
    EXPECT_TRUE(kvs.modifyValues<int>("int2", [&] (int &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());
        return true;
    }));
    EXPECT_EQ(0u, toFind.size()) << "Did not call modify-callback enough times";

    // now modify first, but stop after it
    toFind = {456+25, 1337+25};
    int modified = 0;
    EXPECT_TRUE(kvs.modifyValues<int>("int2", [&] (int &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());
        val += 25;
        modified = val;
        return false;       // stop after first
    }));
    EXPECT_EQ(1u, toFind.size()) << "Did not call modify-callback exactly 1 time";

    // check if only one replaced, the other must be the same
    toFind = {toFind[0], modified};
    EXPECT_TRUE(kvs.modifyValues<int>("int2", [&] (int &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());
        return true;
    }));
    EXPECT_EQ(0u, toFind.size()) << "Did not call modify-callback enough times";

    // ## non-existent key ##
    EXPECT_FALSE(kvs.modifyValues<int>("non-existent", [&] (int &) {
        ADD_FAILURE() << "Modify-callback called too often";
        return true;
    }));
}

TEST_F(KeyValueStorageTest, PrimitiveGetCallback) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    std::vector<int> test1Values = {1, 2, 3, 4, 5};
    testContainer.setValue<int>("test1", test1Values[0]);
    testContainer.setValue<int>("test1", test1Values[1]);
    testContainer.setValue<int>("test1", test1Values[2]);
    testContainer.setValue<int>("test1", test1Values[3]);
    testContainer.setValue<int>("test1", test1Values[4]);

    // write 3 values with other key
    std::vector<int> test2Values = {1337, 1338, 1339};
    testContainer.setValue<int>("test2", test2Values[0]);
    testContainer.setValue<int>("test2", test2Values[1]);
    testContainer.setValue<int>("test2", test2Values[2]);

    // write 1 value with another key
    testContainer.setValue<int>("test3", 42);

    uint32_t count = 0;
    EXPECT_TRUE(testContainer.getValues<int>("test1", [&] (const int &val) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), val);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count++;
        return true;
    }));

    // ensure that 5 values were found
    EXPECT_EQ(5u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getValues<int>("test2", [&] (const int &val) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), val);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));

    // ensure that 3 values were found
    EXPECT_EQ(3u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getValues<int>("test3", [&] (const int &val) -> bool {
        EXPECT_EQ(val, 42);

        count ++;
        return true;
    }));

    // ensure that 1 value was found
    EXPECT_EQ(1u, count);

    // returning false from callback must stop iteration
    count = 0;
    EXPECT_TRUE(testContainer.getValues<int>("test1", [&] (const int &) -> bool {
        EXPECT_EQ(0u, count);
        count++;
        return false;
    }));
    EXPECT_EQ(1u, count);

    // non-existent key
    EXPECT_FALSE(testContainer.getValues<int>("nonexistent", [&] (const int &) -> bool {
        ADD_FAILURE() << "Callback must not be called for non-existent key";
        return true;
    }));
}

/** #################### COMPLEX DATA TYPES #################### **/
const float finf = std::numeric_limits<float>::infinity();
const double dinf = std::numeric_limits<double>::infinity();
struct ComplexStruct {
    float a;
    int b;
    double c;

    bool operator==(const ComplexStruct &other) const {
        return a == other.a && b == other.b && c == other.c;
    }
    bool operator!=(const ComplexStruct &other) const {
        return !operator==(other);
    }

    friend std::ostream &operator<<(std::ostream &stream, const ComplexStruct &data) {
        std::string out = std::to_string(data.a)+", "+std::to_string(data.b)+", "+std::to_string(data.c);
        return stream.write(out.c_str(), out.size());
    }
};

TEST_F(KeyValueStorageTest, Complex) {
    uint32_t count;
    std::vector<ComplexStruct> test1Values = {{1.23f, 1, 3.14159}, {0.0f, 2, 0}, {-1337.42f, 3, -12390102319023}};

    KeyValueStorage kvs;
    // unique values
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("val1", test1Values[0]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("val2", test1Values[1]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("val3", test1Values[2]));

    // check if correctly stored
    // .. if uniquely enforced
    EXPECT_EQ(test1Values[0], kvs.getValue<ComplexStruct>("val1"));
    EXPECT_EQ(test1Values[1], kvs.getValue<ComplexStruct>("val2"));
    EXPECT_EQ(test1Values[2], kvs.getValue<ComplexStruct>("val3"));

    // .. if not uniquely enforced
    EXPECT_EQ(test1Values[0], kvs.getValue<ComplexStruct>("val1", false));
    EXPECT_EQ(test1Values[1], kvs.getValue<ComplexStruct>("val2", false));
    EXPECT_EQ(test1Values[2], kvs.getValue<ComplexStruct>("val3", false));


    // multiple values
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("vals", test1Values[0]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("vals", test1Values[1]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("vals", test1Values[2]));

    EXPECT_THROW(kvs.getValue<ComplexStruct>("vals"), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";

    // returned value must be any of list
    EXPECT_ANY_OF(test1Values, kvs.getValue<ComplexStruct>("vals", false));

    // callback check
    count = 0;
    EXPECT_TRUE(kvs.getValues<ComplexStruct>("vals", [&] (const ComplexStruct &i) -> bool {
        bool containsValue = false;
        for (auto k : test1Values) containsValue |= (k == i);
        EXPECT_TRUE(containsValue);           // values must be in values list

        count++;
        return true;
    }));
    EXPECT_EQ(test1Values.size(), count);
}

TEST_F(KeyValueStorageTest, ComplexFallback) {
    std::vector<ComplexStruct> test1Values = {{1.23f, 1, 3.14159}, {0.0f, 2, 0}, {-1337.42f, 3, -12390102319023},
                                              {finf, 4, dinf}, {-finf, 5, -dinf}};
    std::vector<ComplexStruct> test2Values = {{42.0f, 1, 42}, {0.0f, 2, 0}, {1.0f, 3, 4}};

    KeyValueStorage kvs;

    // store with fallback option
    EXPECT_EQ(test1Values[0], kvs.getSetValue<ComplexStruct>("val1", test1Values[0]));
    EXPECT_EQ(test1Values[1], kvs.getSetValue<ComplexStruct>("val2", test1Values[1]));
    EXPECT_EQ(test1Values[2], kvs.getSetValue<ComplexStruct>("val3", test1Values[2]));

    // must exist now, new fallback must not be inserted
    EXPECT_EQ(test1Values[0], kvs.getSetValue<ComplexStruct>("val1", test2Values[0]));
    EXPECT_EQ(test1Values[1], kvs.getSetValue<ComplexStruct>("val2", test2Values[1]));
    EXPECT_EQ(test1Values[2], kvs.getSetValue<ComplexStruct>("val3", test2Values[2]));

    // multiple values
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("vals", test2Values[0]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("vals", test2Values[1]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("vals", test2Values[2]));

    // enforced unique
    EXPECT_THROW(kvs.getSetValue<ComplexStruct>("vals", test2Values[0]), std::invalid_argument) << "Must fail if there are multiple values but enforced to be unique";

    // not enforced unique
    std::vector<ComplexStruct> newVals({test2Values[0], test2Values[1], test2Values[2], test1Values[0]});
    EXPECT_ANY_OF(newVals, kvs.getSetValue<ComplexStruct>("vals", test1Values[0], false));
}

TEST_F(KeyValueStorageTest, ComplexReplace) {
    KeyValueStorage kvs;
    std::vector<ComplexStruct> vals = {{1.23f, 1, 3.14159}, {0.0f, 2, 0}, {-1337.42f, 3, -12390102319023},
                                              {finf, 4, dinf}, {-finf, 5, -dinf}};
    // unique
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("val1", vals[0]));
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("val1", vals[1], true));
    EXPECT_EQ(vals[1], kvs.getValue<ComplexStruct>("val1")) << vals[0] << " must have been replaced to " << vals[1];

    // multiple
    EXPECT_TRUE(kvs.setValue<ComplexStruct>("val1", vals[2]));            // kvs contains ints[1] and ints[2] now
    EXPECT_FALSE(kvs.setValue<ComplexStruct>("val1", vals[0], true)) << "Must not overwrite if there are >= 2 values";
}

TEST_F(KeyValueStorageTest, ComplexNonExistent) {
    KeyValueStorage kvs;

    EXPECT_THROW(kvs.getValue<ComplexStruct>("nonexistentkey"), std::invalid_argument);
    EXPECT_FALSE(kvs.getValues<ComplexStruct>("nonexistentkey", [] (const ComplexStruct&) { return true; }));
}

TEST_F(KeyValueStorageTest, ComplexModify) {
    KeyValueStorage kvs;
    std::vector<ComplexStruct> vals = {{1.23f, 1, 3.14159}, {0.0f, 2, 0}, {-1337.42f, 3, -12390102319023}};
    std::vector<ComplexStruct> test2Values = {{42.0f, 1, 42}, {0.0f, 2, 0}, {1.0f, 3, 4}};


    kvs.setValue<ComplexStruct>("val1", vals[0]);
    kvs.setValue<ComplexStruct>("val2", vals[1]);
    kvs.setValue<ComplexStruct>("val2", vals[2]);

    // ## existing 1-value key ##
    EXPECT_TRUE(kvs.modifyValues<ComplexStruct>("val1", [&] (ComplexStruct &val) {
        EXPECT_EQ(vals[0], val);
        val = test2Values[0];
        return true;
    }));
    EXPECT_EQ(test2Values[0], kvs.getValue<ComplexStruct>("val1")) << vals[0] << " must have been modified to " << 99;

    EXPECT_TRUE(kvs.modifyValues<ComplexStruct>("val1", [&] (ComplexStruct &val) {
        EXPECT_EQ(test2Values[0], val);
        return true;
    }));
    EXPECT_EQ(test2Values[0], kvs.getValue<ComplexStruct>("val1")) << "Must not overwrite " << 99;

    // ## existing n-value key ##
    std::vector<ComplexStruct> toFind = {vals[1], vals[2]};
    EXPECT_TRUE(kvs.modifyValues<ComplexStruct>("val2", [&] (ComplexStruct &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());

        val.a += 25;
        return true;
    }));
    EXPECT_EQ(0u, toFind.size()) << "Did not call modify-callback enough times";

    // check if really replaced
    toFind = {vals[1], vals[2]};
    toFind[0].a += 25;
    toFind[1].a += 25;
    EXPECT_TRUE(kvs.modifyValues<ComplexStruct>("val2", [&] (ComplexStruct &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());
        return true;
    }));
    EXPECT_EQ(0u, toFind.size()) << "Did not call modify-callback enough times";

    // now modify first, but stop after it
    //toFind = {456+25, 1337+25};
    toFind = {vals[1], vals[2]};
    toFind[0].a += 25;
    toFind[1].a += 25;
    ComplexStruct modified;
    EXPECT_TRUE(kvs.modifyValues<ComplexStruct>("val2", [&] (ComplexStruct &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());
        val.b += 25;
        modified = val;
        return false;       // stop after first
    }));
    EXPECT_EQ(1u, toFind.size()) << "Did not call modify-callback exactly 1 time";

    // check if only one replaced, the other must be the same
    toFind = {toFind[0], modified};
    EXPECT_TRUE(kvs.modifyValues<ComplexStruct>("val2", [&] (ComplexStruct &val) {
        EXPECT_ANY_OF(toFind, val);
        if (toFind.size() == 0)
            ADD_FAILURE() << "Modify-callback called too often";
        toFind.erase(std::remove(toFind.begin(), toFind.end(), val), toFind.end());
        return true;
    }));
    EXPECT_EQ(0u, toFind.size()) << "Did not call modify-callback enough times";

    // ## non-existent key ##
    EXPECT_FALSE(kvs.modifyValues<ComplexStruct>("non-existent", [&] (ComplexStruct &) {
        ADD_FAILURE() << "Modify-callback called too often";
        return true;
    }));
}

TEST_F(KeyValueStorageTest, ComplexGetCallback) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    float finf = std::numeric_limits<float>::infinity();
    double dinf = std::numeric_limits<double>::infinity();
    std::vector<ComplexStruct> test1Values = {{1.23f, 1, 3.14159}, {0.0f, 2, 0}, {-1337.42f, 3, -12390102319023},
                                              {finf, 4, dinf}, {-finf, 5, -dinf}};
    testContainer.setValue<ComplexStruct>("test1", test1Values[0]);
    testContainer.setValue<ComplexStruct>("test1", test1Values[1]);
    testContainer.setValue<ComplexStruct>("test1", test1Values[2]);
    testContainer.setValue<ComplexStruct>("test1", test1Values[3]);
    testContainer.setValue<ComplexStruct>("test1", test1Values[4]);

    // write 3 values with other key
    std::vector<ComplexStruct> test2Values = {{42.0f, 1, 42}, {0.0f, 2, 0}, {1.0f, 3, 4}};
    testContainer.setValue<ComplexStruct>("test2", test2Values[0]);
    testContainer.setValue<ComplexStruct>("test2", test2Values[1]);
    testContainer.setValue<ComplexStruct>("test2", test2Values[2]);

    // write 1 value with another key
    testContainer.setValue<ComplexStruct>("test3", {10e3, 0, 123e-20});

    uint32_t count = 0;
    EXPECT_TRUE(testContainer.getValues<ComplexStruct>("test1", [&] (const ComplexStruct &val) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), val);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count++;
        return true;
    }));

    // ensure that 5 values were found
    EXPECT_EQ(5u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getValues<ComplexStruct>("test2", [&] (const ComplexStruct &val) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), val);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));

    // ensure that 3 values were found
    EXPECT_EQ(3u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getValues<ComplexStruct>("test3", [&] (const ComplexStruct &val) -> bool {
        EXPECT_EQ(val, ComplexStruct({10e3, 0, 123e-20}));

        count ++;
        return true;
    }));

    // ensure that 1 value was found
    EXPECT_EQ(1u, count);

    // returning false from callback must stop iteration
    count = 0;
    EXPECT_TRUE(testContainer.getValues<ComplexStruct>("test1", [&] (const ComplexStruct &) -> bool {
        EXPECT_EQ(0u, count);
        count++;
        return false;
    }));
    EXPECT_EQ(1u, count);

    // non-existent key
    EXPECT_FALSE(testContainer.getValues<ComplexStruct>("nonexistent", [&] (const ComplexStruct &) -> bool {
        ADD_FAILURE() << "Callback must not be called for non-existent key";
        return true;
    }));
}

TEST_F(KeyValueStorageTest, SerializableGetCallback) {
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
    EXPECT_TRUE(testContainer.getSerializables<String>("test1", [&] (const String &str) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), str);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);

        count++;
        return true;
    }));

    // ensure that 5 values were found
    EXPECT_EQ(5u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getSerializables<String>("test2", [&] (const String &str) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), str);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));

    // ensure that 3 values were found
    EXPECT_EQ(3u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getSerializables<String>("test3", [&] (const String &str) -> bool {
        EXPECT_EQ(str, "lonely");

        count ++;
        return true;
    }));

    // ensure that 1 value was found
    EXPECT_EQ(1u, count);

    // returning false from callback must stop iteration
    count = 0;
    EXPECT_TRUE(testContainer.getSerializables<String>("test1", [&] (const String &) -> bool {
        EXPECT_EQ(0u, count);
        count++;
        return false;
    }));
    EXPECT_EQ(1u, count);

    // non-existent key
    EXPECT_FALSE(testContainer.getSerializables<String>("nonexistent", [&] (const String &) -> bool {
        ADD_FAILURE() << "Callback must not be called for non-existent key";
        return true;
    }));
}

TEST_F(KeyValueStorageTest, BufferGetCallback) {
    KeyValueStorage testContainer;

    // write 5 values with one key
    std::vector<Buffer> test1Values = {String("1"), String("2"), String("3"), String("4"), String("5")};
    testContainer.setBuffer("test1", test1Values[0]);
    testContainer.setBuffer("test1", test1Values[1]);
    testContainer.setBuffer("test1", test1Values[2]);
    testContainer.setBuffer("test1", test1Values[3]);
    testContainer.setBuffer("test1", test1Values[4]);

    // write 3 values with other key
    std::vector<Buffer> test2Values = {String("abc"), String("def"), String("ghi")};
    testContainer.setBuffer("test2", test2Values[0]);
    testContainer.setBuffer("test2", test2Values[1]);
    testContainer.setBuffer("test2", test2Values[2]);

    // write 1 value with another key
    testContainer.setBuffer("test3", String("lonely"));

    uint32_t count = 0;
    EXPECT_TRUE(testContainer.getBuffers("test1", [&] (const Buffer &str) -> bool {
        auto it = std::find(test1Values.begin(), test1Values.end(), str);
        EXPECT_NE(test1Values.end(), it);           // values must be in values list
        test1Values.erase(it);
        test1Values.erase(std::remove(test1Values.begin(), test1Values.end(), str), test1Values.end());

        count++;
        return true;
    }));

    // ensure that 5 values were found
    EXPECT_EQ(5u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getBuffers("test2", [&] (const Buffer &str) -> bool {
        auto it = std::find(test2Values.begin(), test2Values.end(), str);
        EXPECT_NE(test2Values.end(), it);           // values must be in values list
        test2Values.erase(it);

        count ++;
        return true;
    }));

    // ensure that 3 values were found
    EXPECT_EQ(3u, count);

    count = 0;
    EXPECT_TRUE(testContainer.getBuffers("test3", [&] (const Buffer &str) -> bool {
        EXPECT_EQ(str, String("lonely"));

        count ++;
        return true;
    }));

    // ensure that 1 value was found
    EXPECT_EQ(1u, count);

    // returning false from callback must stop iteration
    count = 0;
    EXPECT_TRUE(testContainer.getBuffers("test1", [&] (const Buffer &) -> bool {
        EXPECT_EQ(0u, count);
        count++;
        return false;
    }));
    EXPECT_EQ(1u, count);

    // non-existent key
    EXPECT_FALSE(testContainer.getBuffers("nonexistent", [&] (const Buffer &) -> bool {
        ADD_FAILURE() << "Callback must not be called for non-existent key";
        return true;
    }));
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
