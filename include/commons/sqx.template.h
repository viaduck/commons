[[[cog
import os
import cog
import sqx.sqlite_generator as g
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


#include <secure_memory/String.h>
#include <string>

[[[cog
    vars = list(g.do(filename))

    cog.outl("class "+name+" {")
]]]
[[[end]]]
public:
    // constructors
    [[[cog
    cog.outl("{name}() : mId(-1) {{}}".format(name=name))
    cog.outl("{name}(int64_t id) : mId(id) {{}}".format(name=name))
    ]]]
    [[[end]]]

    // --
    static void createTable(sqlite::cryptosqlite_database &db) {
        [[[cog
            param_list = []
            for v in vars:
                param_list.append('{name} {sql_type}'.format(name=v.name(), sql_type=v.sql_type()))
            cog.outl('db << "CREATE TABLE {name} (pid INTEGER PRIMARY KEY, {param_list});";'.format(name=name, param_list=', '.join(param_list)))
        ]]]
        [[[end]]]
    }

    // --
    virtual bool load() {
        bool found = false;
        [[[cog
            param_list = []
            for v in vars:
                param_list.append(v.name())
            cog.outl('*db() << "SELECT {select_what} FROM {name} WHERE pid = ?;" << mId >> \n[&] ('.format(name=name, select_what=', '.join(param_list)))
            param_list = []
            for v in vars:
                type = 'const sqlite::blob_t&' if v.is_complex() else v.var_type()
                param_list.append('{type} {name}'.format(type=type, name=v.name()))
            cog.outl('    '+', '.join(param_list))
            cog.outl(') {')
            cog.outl('    found = true;')
            # lambda
            for v in vars:
                if v.is_complex():
                    cog.outl('    m{member_name}.clear(); m{member_name}.write({name}.first, {name}.second, 0);'.format(name=v.name(), member_name=v.member_name()))
                else:
                    cog.outl('    m{member_name} = {name};'.format(name=v.name(), member_name=v.member_name()))
            cog.outl('};')
        ]]]
        [[[end]]]
        return found;
    }
    virtual void store() {
        if (mId >= 0) {
            [[[cog
                param_list = []
                for v in vars:
                    param_list.append(v.name()+' = ?')
                cog.outl('*db() << "UPDATE {name} SET {set_what} WHERE pid = ?;"'.format(name=name, set_what=', '.join(param_list)))
                for v in vars:
                    if v.is_complex():
                        cog.outl('      << sqlite::blob_t(m{member_name}.const_data(), m{member_name}.size())'.format(name=v.name(), member_name=v.member_name()))
                    else:
                        cog.outl('      << m{member_name}'.format(name=v.name(), member_name=v.member_name()))
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
                    if v.is_complex():
                        cog.outl('      << sqlite::blob_t(m{member_name}.const_data(), m{member_name}.size())'.format(name=v.name(), member_name=v.member_name()))
                    else:
                        cog.outl('      << m{member_name}'.format(name=v.name(), member_name=v.member_name()))
                cog.outl(';')
            ]]]
            [[[end]]]
            mId = db()->last_insert_rowid();
        }
    }

    [[[cog
    for v in vars:
        if v.is_complex():
            cog.outl("// - "+v.name()+" - //")
            cog.outl("inline const {type} &{name}() const {{\n"
                     "    return m{member_name};\n"
                     "}}\n"
                     "inline {type} &{name}() {{\n"
                     "    return m{member_name};\n"
                     "}}\n".format(type=v.var_type(), name=v.name(), member_name=v.member_name()))
        else:
            cog.outl("// - "+v.name()+" - //")
            cog.outl("inline {type} {name}() const {{\n"
                     "    return m{member_name};\n"
                     "}}\n"
                     "inline void {name}({type} {name}) {{\n"
                     "    m{member_name} = {name};\n"
                     "}}\n".format(type=v.var_type(), name=v.name(), member_name=v.member_name()))
    ]]]
    [[[end]]]
protected:
    virtual sqlite::cryptosqlite_database *db() = 0;
    int64_t mId;

    // --
    [[[cog
        for v in vars:
            cog.outl("{type} m{member_name};".format(member_name=v.member_name(), type=v.var_type()))
    ]]]
    [[[end]]]
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
