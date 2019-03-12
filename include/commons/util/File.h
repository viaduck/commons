/*
 * Copyright (C) 2015-2018 The ViaDuck Project
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

#ifndef COMMONS_FILE_H
#define COMMONS_FILE_H

#include <string>
#include <vector>

#ifndef WIN32
    #include <dirent.h>
#endif

class File {
public:
    static std::vector<std::string> find(std::string path, const std::string &ext) {
        std::vector<std::string> files;

        // append trailing / if missing
        if (!path.empty() && path.back() != '/')
            path += '/';

#ifdef WIN32
        // glob pattern
        std::string pattern = path + "*" + ext;

        WIN32_FIND_DATAA find_data;
        HANDLE find_result = FindFirstFileA(pattern.c_str(), &find_data);

        // error occured, maybe we did not find any file
        if (find_result == INVALID_HANDLE_VALUE)
            return files;

        do files.emplace_back(path + find_data.cFileName);
        while (FindNextFileA(find_result, &find_data));

        FindClose(find_result);

#else
        if (path.empty())
            path = "./";

        DIR *dir = opendir(path.c_str());
        dirent *ent;

        while (dir && (ent = readdir(dir))) {
            std::string filename = ent->d_name;

            if (filename.size() >= ext.size() && filename.compare(filename.size() - ext.size(), ext.size(), ext) == 0)
                files.emplace_back(path + filename);
        }

        closedir(dir);
#endif

        return files;
    }
};

#endif //COMMONS_FILE_H
