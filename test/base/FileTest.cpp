/*
 * Copyright (C) 2022 The ViaDuck Project
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

#include "FileTest.h"
#include <commons/util/File.h>

#include <algorithm>
#include <fstream>

#ifdef WIN32
    #include <direct.h>
    #define mkdir(p, i) _mkdir(p)
#endif

#define EXPECT_CONTAINS(list, el) \
    EXPECT_NE(list.end(), std::find(list.begin(), list.end(), el))

std::string files[] = {
        "test1.txt", "test2", "tes3.abc", "nabc", "sub/test4.txt"
};
std::string directories[] = {
        "sub",
};

/*static*/ void FileTest::createFiles() {
    // create all directories first
    for (const auto &dir : directories)
        mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    // create files
    for (const auto &file : files)
        std::ofstream { file };
}

/*static*/ void FileTest::removeFiles() {
    // remove all files first
    for (const auto &file : files)
        remove(file.c_str());

    // remove (empty) directories
    for (const auto &dir : directories)
        remove(dir.c_str());
}

TEST_F(FileTest, testFind) {
    auto list1 = File::find("", "", "");
    for (const auto& file : files) {
        if (std::string(file).find('/') == std::string::npos) {
            EXPECT_CONTAINS(list1, file) << file;
        }
    }

    auto list2 = File::find("./", "abc", "");
    EXPECT_EQ(2, list2.size());
    EXPECT_CONTAINS(list2, "./tes3.abc");
    EXPECT_CONTAINS(list2, "./nabc");

    auto list3 = File::find("", "", "test");
    EXPECT_EQ(2, list3.size());
    EXPECT_CONTAINS(list3, "test1.txt");
    EXPECT_CONTAINS(list3, "test2");

    auto list4 = File::find("./", ".txt", "test");
    EXPECT_EQ(1, list4.size());
    EXPECT_CONTAINS(list4, "./test1.txt");

    auto list5 = File::find("sub", ".txt", "test");
    EXPECT_EQ(1, list5.size());
    EXPECT_CONTAINS(list5, "sub/test4.txt");
}
