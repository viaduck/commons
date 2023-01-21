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

#include <crypto_sqlite/crypto_sqlite.h>
#include <sqlite_modern_cpp/crypto_sqlite.h>
#include <commons/util/Except.h>

class SQXBase {
public:
    DEFINE_ERROR(load, base_error);

    virtual ~SQXBase() = default;
    virtual sqlite::crypto_sqlite_database *db() = 0;

protected:
    virtual void process() {};
};

#endif //COMMONS_SQXBASE_H
