/*
 * Copyright (C) 2019 The ViaDuck Project
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

#ifndef COMMONS_FILELOGGER_H
#define COMMONS_FILELOGGER_H

#include <fstream>

#include <commons/log/ILogger.h>

/**
 * This ILogger implementation logs all log levels to file.
 */
class FileLogger : public ILogger {
public:
    FileLogger(const std::string &filename) {
        mFile.open(filename, std::ios_base::app);
    }

    std::ostream &stream() override {
        return mFile;
    }

    bool wantsLog(LogLevel level) override;

protected:
    std::ofstream mFile;
};

#endif //COMMONS_FILELOGGER_H