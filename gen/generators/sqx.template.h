/*
 * Copyright (C) 2018 The ViaDuck Project
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

[[[cog
    import cog
    from gen_sqx import SQXDef, ElemType
    s_def = SQXDef(def_file)

    s_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    s_def.outl("#ifndef {name}_H")
    s_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <string>
#include <type_traits>

#include <secure_memory/String.h>
#include <commons/SQXBase.h>
#include <commons/ConstexprString.h>

[[[cog
    # foreign includes
    for elem in s_def.elements:
        if elem.sqx_type == ElemType.Foreign and not elem.recursive:
            elem.outl('#include "{path}.h"')

    # imported includes
    for e_def in s_def.includes:
        e_def.outl('#include "{import_path}"')

    s_def.outl("")
    s_def.outl("{doxygen}")
    s_def.outl("class {name} : public SQXBase {{")
]]]
[[[end]]]
public:

    // types for partial load / store
    [[[cog
        for elem in s_def.elements:
            elem.outl('struct Column_{name} {{\n'
                      '    static constexpr auto ID() {{ return MakeConstexprString("{name}"); }}\n'
                      '}};')
    ]]]
    [[[end]]]

    // template for storing a particular member
    template<typename T>
    sqlite::database_binder &&member_store(sqlite::database_binder &&dbb);

    // variadic template for iterating all template arguments to store them
    template <typename T, typename... Args>
    inline sqlite::database_binder &&internal_store(sqlite::database_binder &&dbb) {
        return std::move(internal_store<Args...>(std::move(member_store<T>(std::move(dbb)))));
    }
    // variadic template recursion anchor
    template <typename... Args>
    inline sqlite::database_binder &&internal_store(sqlite::database_binder &&dbb, typename std::enable_if<sizeof...(Args) == 0>::type* = 0) {
        return std::move(dbb);
    }

    // variadic template for building SQL UPDATE statement
    template<typename Column, typename... Columns>
    static constexpr auto build_update_stmt(typename std::enable_if<sizeof...(Columns) != 0>::type* = 0) {
        return Column::ID() + MakeConstexprString("=?, ") + build_update_stmt<Columns...>();
    }
    // variadic template specialization if there is only one parameter left. This is required, since the last column must not have a trailing comma.
    template<typename Column, typename... Columns>
    static constexpr auto build_update_stmt(typename std::enable_if<sizeof...(Columns) == 0>::type* = 0) {
        return Column::ID() + MakeConstexprString("=?");
    }
    // variadic template recursion anchor
    static constexpr auto build_update_stmt() {
        return MakeConstexprString("");
    }

    // variadic template for building SQL INSERT statement
    template<typename Column, typename... Columns>
    static constexpr auto build_insert_stmt(typename std::enable_if<sizeof...(Columns) != 0>::type* = 0) {
        return Column::ID() + MakeConstexprString(", ") + build_insert_stmt<Columns...>();
    }
    // variadic template specialization if there is only one parameter left. This is required, since the last column must not have a trailing comma.
    template<typename Column, typename... Columns>
    static constexpr auto build_insert_stmt(typename std::enable_if<sizeof...(Columns) == 0>::type* = 0) {
        return Column::ID();
    }
    // variadic template recursion anchor
    static constexpr auto build_insert_stmt() {
        return MakeConstexprString("");
    }

    // variadic template for building placeholder sequence, required by INSERT statement
    template<typename Column, typename... Columns>
    static constexpr auto build_stmt_placeholders(typename std::enable_if<sizeof...(Columns) != 0>::type* = 0) {
        return MakeConstexprString("?, ") + build_stmt_placeholders<Columns...>();
    }
    // variadic template specialization if there is only one parameter left. This is required, since the last placeholder must not have a trailing comma.
    template<typename Column, typename... Columns>
    static constexpr auto build_stmt_placeholders(typename std::enable_if<sizeof...(Columns) == 0>::type* = 0) {
        return MakeConstexprString("?");
    }
    // variadic template recursion anchor
    static constexpr auto build_stmt_placeholders() {
        return MakeConstexprString("");
    }

    // constructors
    [[[cog
        s_def.outl("{name}() {{}}")
        s_def.outl("{name}(int64_t id) : mId(id) {{}}")
        s_def.outl("{name}(SQXBase *base) : mParent(base) {{}}")
        s_def.outl("{name}(SQXBase *base, int64_t id) : mParent(base), mId(id) {{}}")
    ]]]
    [[[end]]]

    // create
    static void createTable(sqlite::cryptosqlite_database &db) {
        [[[cog
            # create foreign sub tables first
            for elem in s_def.elements:
                if elem.sqx_type == ElemType.Foreign and not elem.recursive:
                    elem.outl("{type.cpp_t}::createTable(db);")

            # build CREATE statement
            s_def.out('db << "CREATE TABLE IF NOT EXISTS {name} (pid INTEGER PRIMARY KEY')
            for elem in s_def.elements:
                elem.out(", {name} {type.sql_t}")

                if elem.sqx_type == ElemType.Foreign:
                    elem.out(" REFERENCES {type.cpp_t} {constraints}")

            s_def.outl(');";')
        ]]]
        [[[end]]]
    }

    // load
    virtual void load() {
        bool found = false;

        [[[cog
            s_def.out('*db() << "SELECT ')

            # print comma separated list of names
            SQXDef.reset_list()
            for elem in s_def.elements:
                elem.lout("{name}")

            s_def.outl(' FROM {name} WHERE pid = ?;" << mId >>')
            s_def.out('[&] (')

            # print comma separated list of arguments
            SQXDef.reset_list()
            for elem in s_def.elements:
                elem.lout("{type.sql_ref_t} {name}")

            s_def.outl(") {{")
            s_def.outl("    found = true;")

            # load each argument into a member
            for elem in s_def.elements:
                elem.outl("    " + elem.type.load)

            s_def.outl("}};\n")

            # check found
            s_def.outl('if (!found)\n'
                       '    throw SQXBase::load_error("{name} could not be loaded");')

        ]]]
        [[[end]]]

        process();
    }

    // store
    virtual void store() {
        if (mId >= 0) {
            [[[cog
                s_def.out('*db() << "UPDATE {name} SET ')

                # print comma separated list of names
                SQXDef.reset_list()
                for elem in s_def.elements:
                    elem.lout("{name} = ?")

                s_def.outl(' WHERE pid = ?;"')

                # pipe each store into the statement
                for elem in s_def.elements:
                    elem.outl("      << " + elem.type.store)

                s_def.outl("      << mId;")
            ]]]
            [[[end]]]
        } else {
            [[[cog
                s_def.out('*db() << "INSERT INTO {name} (')

                # print comma separated list of names
                SQXDef.reset_list()
                for elem in s_def.elements:
                    elem.lout("{name}")

                s_def.out(") VALUES (")

                # print comma separated list of question marks
                SQXDef.reset_list()
                for elem in s_def.elements:
                    elem.lout("?")

                s_def.outl(');"')

                # pipe each store into the statement
                for elem in s_def.elements:
                    elem.outl("      << " + elem.type.store)

                s_def.outl(";")
            ]]]
            [[[end]]]

            mId = db()->last_insert_rowid();
        }
    }

    // specific store
    template <typename... Columns>
    void storeSpecific() {
        if (mId >= 0) {
            [[[cog
                s_def.outl('auto update_stmt = MakeConstexprString("UPDATE {name} SET ") + '
                           'build_update_stmt<Columns...>() + MakeConstexprString("WHERE pid = ?;");')
            ]]]
            [[[end]]]

            sqlite::database_binder dbb = *db() << update_stmt.c_str();
            internal_store<Columns...>(std::move(dbb)) << mId;
        } else {
            {
                [[[cog
                    s_def.outl('auto insert_stmt = MakeConstexprString("INSERT INTO {name} (") + '
                               'build_insert_stmt<Columns...>() + MakeConstexprString(") VALUES (") + '
                               '(build_stmt_placeholders<Columns...>()) + MakeConstexprString(");");')
                ]]]
                [[[end]]]

                sqlite::database_binder dbb = *db() << insert_stmt.c_str();
                internal_store<Columns...>(std::move(dbb));
            }
            mId = db()->last_insert_rowid();
        }
    }

    // delete this row
    virtual void remove() {
        if (mId >= 0) {
            [[[cog
                s_def.outl('*db() << "DELETE FROM {name} WHERE pid = ?;" << mId;')
            ]]]
            [[[end]]]
        }
    }

    // select all rows
    static std::vector<int64_t> selectAll(sqlite::cryptosqlite_database &dbConnection, const std::string &where = "") {
        std::vector<int64_t> elements;

        [[[cog
            s_def.outl('dbConnection << "SELECT pid FROM {name}" + (where.empty() ? ";" : " " + where + ";") >>')
        ]]]
        [[[end]]]
        [&] (int64_t pid) {
            elements.push_back(pid);
        };

        return elements;
    }

    // id of this row
    int64_t id() const {
        return mId;
    }

    // getter (basic)
    [[[cog
        for elem in s_def.elements:
            if 'basic' in elem.type.getter:
                elem.outl("inline {type.cpp_const_ref_t} {name}() const {{\n"
                          "    return {member_name};\n"
                          "}}\n")
    ]]]
    [[[end]]]

    // setter (basic, ref, range)
    [[[cog
        for elem in s_def.elements:
            if 'basic' in elem.type.setter:
                elem.outl("inline void {name}({type.cpp_const_ref_t} {name}) {{\n"
                          "    " + elem.type.load_setter + "\n"
                          "}}\n")

            if 'ref' in elem.type.setter:
                elem.outl("inline {type.cpp_ref_t} {name}() {{\n"
                          "    return {member_name};\n"
                          "}}\n")

            if 'range' in elem.type.setter:
                elem.outl("inline void {name}_range(const BufferRangeConst &{name}) {{\n"
                          "    " + elem.type.load_setter + "\n"
                          "}}\n")
    ]]]
    [[[end]]]

    // getter (foreign)
    [[[cog
        for elem in s_def.elements:
            if 'foreign' in elem.type.getter:
                elem.outl("inline {type.cpp_ref_t} {name}() {{\n"
                          "    if (!{member_name}) load_{name}();\n"
                          "    if (!{member_name})\n"
                          "        throw SQXBase::load_error(\"{member_name} in {type.cpp_t} invalid fk\");\n\n"
                          "    return {member_name};\n"
                          "}}\n")

                elem.outl("inline {type.cpp_const_ref_t} {name}() const {{\n"
                          "    if (!{member_name})\n"
                          "        throw SQXBase::load_error(\"{member_name} in {type.cpp_t} invalid fk\");\n\n"
                          "    return {member_name};\n"
                          "}}\n")

                elem.outl("inline void load_{name}(bool force=true) {{\n"
                          "    if ({member_name}_id >= 0 && (force || !{member_name})) {{\n"
                          "        {member_name}.reset(new {type.cpp_t}(this, {member_name}_id));\n"
                          "        {member_name}->load();\n"
                          "    }}\n"
                          "}}\n")

                elem.outl("inline int64_t {name}_id() const {{\n"
                          "    return {member_name}_id;\n"
                          "}}\n")
    ]]]
    [[[end]]]

    // setter (foreign)
    [[[cog
        for elem in s_def.elements:
            if 'foreign' in elem.type.setter:
                elem.outl("inline void create_{name}() {{\n"
                          "    {member_name}.reset(new {type.cpp_t}(this));\n"
                          "}}\n")

                elem.outl("inline void {name}_id(int64_t value) {{\n"
                          "    {member_name}.reset();"
                          "    {member_name}_id = value;\n"
                          "}}\n")

                elem.outl("inline void store_{name}() {{\n"
                          "    {member_name}->store();\n"
                          "    {member_name}_id = {member_name}->id();\n"
                          "}}")
    ]]]
    [[[end]]]

protected:
    sqlite::cryptosqlite_database *db() override {
        if (mParent != nullptr)
            return mParent->db();
        else
            return nullptr;
    };

    SQXBase *mParent = nullptr;
    int64_t mId = -1;

    // members
    [[[cog
        for elem in s_def.elements:
            elem.outl(elem.type.member)
    ]]]
    [[[end]]]

};

// template specialization for every member
[[[cog
    for elem in s_def.elements:
        elem.outl('template<>\n'
                   'inline sqlite::database_binder &&{table.name}::member_store<{table.name}::Column_{name}>(sqlite::database_binder &&dbb) {{\n'
                   '    return std::move(dbb << ' + elem.type.store + ');\n'
                   '}}\n', table=s_def)

    s_def.outl("#endif //{name}_H")
]]]
[[[end]]]