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

#ifndef COMMONS_SQXBASE_H
#define COMMONS_SQXBASE_H

#include <stdexcept>

class SQXBase {
public:
    struct load_exception : public std::runtime_error {
        explicit load_exception(const std::string &what) : std::runtime_error(what) {}
    };

    virtual sqlite::cryptosqlite_database *db() = 0;

protected:
    virtual void process() {};
};

#endif //COMMONS_SQXBASE_H
