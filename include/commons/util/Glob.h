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

#ifndef COMMONS_GLOB_H
#define COMMONS_GLOB_H

#include <secure_memory/String.h>

#ifndef WIN32
    #include <glob.h>
#endif

class Glob {
public:
    static std::vector<std::string> pattern(const std::string &pattern) {
        std::vector<std::string> files;

#ifdef WIN32
        WIN32_FIND_DATA find_data;
        HANDLE find_result = FindFirstFile(pattern.c_str(), &find_data);

        // error occured, maybe we did not find any file
        if (find_result == INVALID_HANDLE_VALUE)
            return files;

        do files.emplace_back(find_data.cFileName);
        while (FindNextFile(find_result, &find_data));

        FindClose(find_result);

#else
        glob_t glob_result;

        // early return on error
        if (0 != glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result))
            return files;

        for (uint32_t i = 0; i < glob_result.gl_pathc; i++)
            files.emplace_back(glob_result.gl_pathv[i]);

        globfree(&glob_result);
#endif

        return files;
    }
};

#endif //COMMONS_GLOB_H
