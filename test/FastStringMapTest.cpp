//
// Created by John Watson on 02.11.16.
//

#include <libCom/FastStringMap.h>
#include "FastStringMapTest.h"

TEST_F(FastStringMapTest, testBasic) {
    FastStringMap<size_t> map;

    String test1("asdfghhjkksdklasdjlasdj"), test2("asdfghhjkksdk"), test3("asdfghhjkksdk");
    size_t stest1 = 1290128, stest2 = 12811, otest = 0;

    // element not present
    ASSERT_FALSE(map.lookup(test1));

    // add first
    map.add(test1, stest2);
    // overwrite
    map.add(test1, stest1);

    // check value
    ASSERT_TRUE(map.lookup(test1, &otest));
    ASSERT_EQ(stest1, otest);

    // check other key
    ASSERT_FALSE(map.lookup(test2));

    // add small key
    map.add("a", 123);
    // check existence
    ASSERT_TRUE(map.lookup("a", &otest));
    // check value
    ASSERT_EQ(123u, otest);

    // add keys with other reference
    map.add(test2, 1337);
    // check
    ASSERT_TRUE(map.lookup(test3));

    // clear and check
    map.clear();
    ASSERT_FALSE(map.lookup(test3));
}