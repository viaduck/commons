/*
 * Copyright (C) 2015-2022 The ViaDuck Project
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

#ifdef WIN32
	#include <windows.h>
#else
    #include <glob.h>
#endif

class File {
public:
    static std::vector<std::string> find(const std::string &path, const std::string &ext, const std::string &prefix = "") {
        std::vector<std::string> files;

        // glob pattern
        std::string pattern = joinPath(path, prefix + "*" + ext);

#ifdef WIN32
        WIN32_FIND_DATAA find_data;

        HANDLE find_result = FindFirstFileA(pattern.c_str(), &find_data);
        if (find_result != INVALID_HANDLE_VALUE) {
            do {
                // skip directories
                if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    files.emplace_back(joinPath(path, find_data.cFileName));
            } while (FindNextFileA(find_result, &find_data));

            FindClose(find_result);
        }

#else
        glob_t glob_data;

        if (0 == glob(pattern.c_str(), GLOB_MARK, nullptr, &glob_data)) {
            for (size_t i = 0; i < glob_data.gl_pathc; i++)
                // skip directories
                if (!endsWith(glob_data.gl_pathv[i], "/"))
                    files.emplace_back(glob_data.gl_pathv[i]);

            globfree(&glob_data);
        }
#endif

        return files;
    }

    static std::string joinPath(const std::string &p1, const std::string &p2) {
        if (!p1.empty() && !endsWith(p1, "/"))
            return p1 + "/" + p2;
        else
            return p1 + p2;
    }

protected:
    static bool endsWith(const std::string &str, const std::string &p) {
        return str.size() > p.size() && str.compare(str.size() - p.size(), p.size(), p) == 0;
    }

    static bool startsWith(const std::string &str, const std::string &p) {
        return str.size() > p.size() && str.compare(0, p.size(), p) == 0;
    }
};

#endif //COMMONS_FILE_H
