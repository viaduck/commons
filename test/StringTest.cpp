//
// Created by steffen on 11.11.15.
//

#include "StringTest.h"
#include "libCom/String.h"
#include "custom_assert.h"


TEST(StringTest, creationTest) {
    // default constructor
    {
        String s;
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // create from cstring
    {
        String s("");
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size())+1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "abc", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // create from String
    {
        String str("");
        String s(str);
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String str("abc");
        String s(str);
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "abc", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // create from stl string
    {
        std::string stl;
        String s(stl);
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        std::string stl("abc");
        String s(stl);
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
        EXPECT_ARRAY_EQ(const char, "abc", s.stl_str().c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
}

TEST(StringTest, appendTest) {
    // ## append a String
    {
        String s("abc");
        String s2("");
        String sf = s + s2;
        ASSERT_EQ(3, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abc", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        String s2("def");
        String sf = s + s2;
        ASSERT_EQ(6, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abcdef", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        String s2("abc");
        String sf = s + s2;
        ASSERT_EQ(3, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abc", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    // +=
    {
        String s("abc");
        String s2("");
        s += s2;
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        String s2("def");
        s += s2;
        ASSERT_EQ(6, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abcdef", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        String s2("abc");
        s += s2;
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // ## append a cstring
    {
        String s("");
        String sf = s + "abc";
        ASSERT_EQ(3, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abc", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        String sf = s + "def";
        ASSERT_EQ(6, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abcdef", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        String sf = s + "";
        ASSERT_EQ(3, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abc", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    // +=
    {
        String s("");
        s += "abc";
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        s += "def";
        ASSERT_EQ(6, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abcdef", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        s += "";
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // ## append an stl string
    {
        String s("abc");
        std::string s2;
        String sf = s + s2;
        ASSERT_EQ(3, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abc", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        std::string s2("def");
        String sf = s + s2;
        ASSERT_EQ(6, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abcdef", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        std::string s2("abc");
        String sf = s + s2;
        ASSERT_EQ(3, static_cast<int32_t>(sf.size()));
        EXPECT_ARRAY_EQ(const char, "abc", sf.c_str(), static_cast<int32_t>(sf.size()) + 1);       // compare the 0-terminator, too!
    }
    // +=
    {
        String s("abc");
        std::string s2;
        s += s2;
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("abc");
        std::string s2("def");
        s += s2;
        ASSERT_EQ(6, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abcdef", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        std::string s2("abc");
        s += s2;
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "abc", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
}

TEST(StringTest, compareTest) {
    {
        String s("");
        String s2("");
        ASSERT_TRUE(s == s2);
        ASSERT_FALSE(s != s2);
    }
    {
        String s("abc");
        String s2("abc");
        ASSERT_TRUE(s == s2);
        ASSERT_FALSE(s != s2);
    }
    {
        String s("");
        String s2("abc");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
    {
        String s("abc");
        String s2("");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
    {
        String s("abcd");
        String s2("abc");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
    {
        String s("abc");
        String s2("abcd");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }

    // cstrings
    {
        String s("");
        ASSERT_TRUE(s == "");
        ASSERT_FALSE(s != "");
    }
    {
        String s("abc");
        ASSERT_TRUE(s == "abc");
        ASSERT_FALSE(s != "abc");
    }
    {
        String s("");
        ASSERT_FALSE(s == "abc");
        ASSERT_TRUE(s != "abc");
    }
    {
        String s("abc");
        ASSERT_FALSE(s == "");
        ASSERT_TRUE(s != "");
    }
    {
        String s("abcd");
        ASSERT_FALSE(s == "abc");
        ASSERT_TRUE(s != "abc");
    }
    {
        String s("abc");
        String s2("abcd");
        ASSERT_FALSE(s == "abcd");
        ASSERT_TRUE(s != "abcd");
    }

    // stl strings
    {
        String s("");
        std::string s2("");
        ASSERT_TRUE(s == s2);
        ASSERT_FALSE(s != s2);
    }
    {
        String s("abc");
        std::string s2("abc");
        ASSERT_TRUE(s == s2);
        ASSERT_FALSE(s != s2);
    }
    {
        String s("");
        std::string s2("abc");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
    {
        String s("abc");
        std::string s2("");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
    {
        String s("abcd");
        std::string s2("abc");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
    {
        String s("abc");
        std::string s2("abcd");
        ASSERT_FALSE(s == s2);
        ASSERT_TRUE(s != s2);
    }
}

TEST(StringTest, reassignTest) {
    // String
    {
        String s("abc");
        String s2("def");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        s = s2;
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "def", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        String s2("def");
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        s = s2;
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "def", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // cstring
    {
        String s("abc");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        s = "def";
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "def", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        s = "def";
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "def", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }

    // stl string
    {
        String s("abc");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        s = std::string("def");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "def", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
    {
        String s("");
        ASSERT_EQ(0, static_cast<int32_t>(s.size()));
        s = std::string("def");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        EXPECT_ARRAY_EQ(const char, "def", s.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!
    }
}

TEST(StringTest, conversionTest) {
    // cstring
    {
        String s("abc");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        const char *cs1 = s.c_str();
        EXPECT_ARRAY_EQ(const char, "abc", cs1, static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!

        s = s + "def";
        ASSERT_EQ(6, static_cast<int32_t>(s.size()));
        const char *cs2 = s.c_str();
        EXPECT_ARRAY_EQ(const char, "abcdef", cs2, static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!

        // old pointer must retain its value!
        EXPECT_ARRAY_EQ(const char, "abc", cs1, 3 + 1);       // compare the 0-terminator, too!
    }

    // stl
    {
        String s("abc");
        ASSERT_EQ(3, static_cast<int32_t>(s.size()));
        std::string std1 = s.stl_str();
        EXPECT_ARRAY_EQ(const char, "abc", std1.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!

        s = s + "def";
        ASSERT_EQ(6, static_cast<int32_t>(s.size()));
        std::string std2 = s.stl_str();
        EXPECT_ARRAY_EQ(const char, "abcdef", std2.c_str(), static_cast<int32_t>(s.size()) + 1);       // compare the 0-terminator, too!

        // old std::string must retain its value!
        EXPECT_ARRAY_EQ(const char, "abc", std1.c_str(), 3 + 1);       // compare the 0-terminator, too!
    }
}
