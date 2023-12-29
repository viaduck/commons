/*
 * Copyright (C) 2023 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UriTest.h"

#include <network/component/Uri.h>

#define CHECK_URI(uri) \
    EXPECT_EQ((uri), Uri((uri)).str())

TEST_F(UriTest, testUriComplex) {
    Uri uri("https://google.com/search?q=test&oq=test#languages%20and%20whatnot");

    EXPECT_EQ("https", uri.schema());
    EXPECT_EQ("google.com", uri.host());

    EXPECT_EQ("search", uri.path());
    std::vector<std::string> pp = { "search" };
    EXPECT_EQ(pp, uri.pathParts());

    EXPECT_EQ("q=test&oq=test", uri.query());
    EXPECT_EQ("test", uri.queryValue("q"));
    EXPECT_EQ("test", uri.queryValue("oq"));
    EXPECT_FALSE(uri.queryValue("abc"));
    Uri::KeyValuePairList_t qp = { {"q", "test"}, {"oq", "test"} };
    EXPECT_EQ(qp, uri.queryParts());

    EXPECT_EQ("languages%20and%20whatnot", uri.fragment());
    Uri::KeyValuePairList_t fp = { {"languages and whatnot", ""} };
    EXPECT_EQ("", uri.fragmentValue("languages and whatnot"));
    EXPECT_EQ(fp, uri.fragmentParts());
}

TEST_F(UriTest, testUriEmpty) {
    Uri uri("https://google.com/");

    EXPECT_EQ("https", uri.schema());
    EXPECT_EQ("google.com", uri.host());

    EXPECT_EQ("", uri.path());
    EXPECT_EQ(0, uri.pathParts().size());

    EXPECT_EQ("", uri.query());
    EXPECT_FALSE(uri.queryValue("q"));
    EXPECT_EQ(0, uri.queryParts().size());

    EXPECT_EQ("", uri.fragment());
    EXPECT_FALSE(uri.fragmentValue("languages and whatnot"));
    EXPECT_EQ(0, uri.fragmentParts().size());
}

TEST_F(UriTest, testUriParse) {
    CHECK_URI("https://google.com/search?q=test&oq=test#languages%20and%20whatnot");
    CHECK_URI("https://google.com/");
    CHECK_URI("vd://l/c:p:1/m:p:1");
}
