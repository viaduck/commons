[[[cog
import os
import cog
import sqlite_generator as g
from protocol_generator import variable_arrays, variable_arrays_type
import common as c
import pathlib
from os.path import basename, splitext, dirname

name = splitext(basename(filename[:-4]))[0]
filename = base_path+filename
]]]
[[[end]]]
[[[cog
cog.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
]]]
[[[end]]]

[[[cog
cog.outl("#ifndef {name}_H".format(name=name))
cog.outl("#define {name}_H".format(name=name))
]]]
[[[end]]]


#include <string>
#include <type_traits>

#include <secure_memory/String.h>
#include <commons/SQXBase.h>
#include <commons/ConstexprString.h>

[[[cog
    res = list(g.do(filename))
    vars = []
    includes_enum = set()
    includes_sqx = set()
    for r in res:
        vars.append(r[0])
        includes_enum.update(r[1])
        includes_sqx.update(r[2])

    for include in includes_enum:
        p = pathlib.Path(include)
        p = pathlib.Path(*p.parts[1:])
        dir = dirname(str(p))
        file = splitext(basename(str(p)))[0]
        cog.outl('#include "{path}.h"'.format(path=os.path.normpath(os.path.join(dir, file))))

    for include in includes_sqx:
        cog.outl('#include "{path}.h"'.format(path=include))

    cog.outl('\n')
    cog.outl("class "+name+" : public SQXBase {")
]]]
[[[end]]]
public:
    // types required for partial store/load
    [[[cog
        param_list = []
        for v in vars:
            param_list.append('struct Column_{name} {{ static constexpr auto ID() {{ return MakeConstexprString("{name}"); }} }};'.format(**v.format_kwargs()))
        cog.outl('\n'.join(param_list))
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
    cog.outl("{name}() {{}}".format(name=name))
    cog.outl("{name}(int64_t id) : mId(id) {{}}".format(name=name))
    cog.outl("{name}(SQXBase *base) : mParent(base) {{}}".format(name=name))
    cog.outl("{name}(SQXBase *base, int64_t id) : mParent(base), mId(id) {{}}".format(name=name))
    ]]]
    [[[end]]]

    // --
    static void createTable(sqlite::cryptosqlite_database &db) {
        [[[cog
            param_list = []
            schema_creation_set = set()
            for v in vars:
                schema_creation_set.add(v.create_schema().format(**v.format_kwargs()))
                param_list.append(v.create_stmt().format(**v.format_kwargs()))
            cog.outl(''.join(schema_creation_set))
            cog.outl('db << "CREATE TABLE IF NOT EXISTS {name} (pid INTEGER PRIMARY KEY, {param_list});";'.format(name=name, param_list=', '.join(param_list)))
        ]]]
        [[[end]]]
    }

    // --
    virtual void load() {
        bool found = false;
        [[[cog
            param_list = []
            for v in vars:
                param_list.append(v.name())
                # pre hook
                pre_hook = v.pre_hook().format(**v.format_kwargs())
                if len(pre_hook) > 0:
                    cog.outl(pre_hook)
            cog.outl('*db() << "SELECT {select_what} FROM {name} WHERE pid = ?;" << mId >> \n[&] ('.format(name=name, select_what=', '.join(param_list)))
            param_list = []
            for v in vars:
                param_list.append('{type} {name}'.format(type=v.type().sql_ref_type(), name=v.name()))
            cog.outl('    '+', '.join(param_list))
            cog.outl(') {')
            cog.outl('    found = true;')
            # lambda
            for v in vars:
                cog.outl('    {0}'.format(v.load()))
            cog.outl('};')

            # post hook
            for v in vars:
                post_hook = v.post_hook().format(**v.format_kwargs())
                if len(post_hook) > 0:
                    cog.outl(post_hook)
        ]]]
        [[[end]]]

        [[[cog
        cog.outl('if (!found) throw SQXBase::load_exception("{name} could not be loaded");'.format(name=name))
        ]]]
        [[[end]]]

        process();
    }
    virtual void store() {
        if (mId >= 0) {
            [[[cog
                param_list = []
                for v in vars:
                    param_list.append(v.name()+' = ?')
                cog.outl('*db() << "UPDATE {name} SET {set_what} WHERE pid = ?;"'.format(name=name, set_what=', '.join(param_list)))
                for v in vars:
                    cog.outl('      << {0}'.format(v.store()))
                cog.outl('      << mId;')
            ]]]
            [[[end]]]
        } else {
            [[[cog
                param_list = []
                for v in vars:
                    param_list.append(v.name())
                cog.outl('*db() << "INSERT INTO {name} ({set_what}) VALUES ({question_marks});"'.format(name=name, set_what=', '.join(param_list), question_marks=', '.join(['?']*len(param_list))))
                for v in vars:
                    cog.outl('      << {0}'.format(v.store()))
                cog.outl(';')
            ]]]
            [[[end]]]
            mId = db()->last_insert_rowid();
        }
    }
    template <typename... Columns>
    void storeSpecific() {
        if (mId >= 0) {
            [[[cog
            cog.outl('auto update_stmt = MakeConstexprString("UPDATE {name} SET ") + build_update_stmt<Columns...>() + MakeConstexprString("WHERE pid = ?;");'.format(name=name))
            ]]]
            [[[end]]]
            sqlite::database_binder dbb = *db() << update_stmt.c_str();
            internal_store<Columns...>(std::move(dbb)) << mId;
        } else {
            {
                [[[cog
                cog.outl('auto insert_stmt = MakeConstexprString("INSERT INTO {name} (") + build_insert_stmt<Columns...>() + MakeConstexprString(") VALUES (") + (build_stmt_placeholders<Columns...>()) + MakeConstexprString(");");'.format(name=name))
                ]]]
                [[[end]]]

                sqlite::database_binder dbb = *db() << insert_stmt.c_str();
                internal_store<Columns...>(std::move(dbb));
            }
            mId = db()->last_insert_rowid();
        }
    }

    virtual void remove() {
        if (mId >= 0) {
            [[[cog
                cog.outl('*db() << "DELETE FROM {name} WHERE pid = ?;" << mId;'.format(name=name))
            ]]]
            [[[end]]]
        }
    }

    static std::vector<int64_t> selectAll(sqlite::cryptosqlite_database &dbConnection, const std::string &where = "") {
        std::vector<int64_t> elements;

        [[[cog
            cog.outl('dbConnection << "SELECT pid FROM {name}" + (where.empty() ? ";" : " " + where + ";") >>'.format(name=name))
        ]]]
        [[[end]]]
        [&] (int64_t pid) {
            elements.push_back(pid);
        };

        return elements;
    }

    int64_t id() const {
        return mId;
    }

    [[[cog
    for v in vars:
        cog.outl("// - "+v.name()+" - //")
        cog.outl("// getter")
        cog.outl(v.type().additional_getter().format(**v.format_kwargs()))
        cog.outl("// setter")
        cog.outl(v.type().additional_setter().format(**v.format_kwargs()))
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

    // --
    [[[cog
        for v in vars:
            cog.outl(v.member_hook().format(**v.format_kwargs()))
            cog.outl("{member_type} {member_name};".format(**v.format_kwargs()))
    ]]]
    [[[end]]]
};

// template specializations for every member
[[[cog
    for v in vars:
        cog.outl('template<>')
        cog.outl('inline sqlite::database_binder &&{table_name}::member_store<{table_name}::Column_{name}>(sqlite::database_binder &&dbb) {{\n'
                 .format(table_name=name, store=v.store, **v.format_kwargs()))
        cog.outl('    return std::move(dbb << {store});\n'
                 '}}\n'.format(table_name=name, store=v.store(), **v.format_kwargs()))
]]]
[[[end]]]

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
