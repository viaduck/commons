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


#include <string>
#include <type_traits>

#include <secure_memory/String.h>

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
                param_list.append('{name} {sql_type}'.format(name=v.name(), sql_type=v.type().sql_type()))
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
                param_list.append('{type} {name}'.format(type=v.type().sql_ref_type(), name=v.name()))
            cog.outl('    '+', '.join(param_list))
            cog.outl(') {')
            cog.outl('    found = true;')
            # lambda
            for v in vars:
                cog.outl('    {0}'.format(v.load()))
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
                    cog.outl('    << {0}'.format(v.store()))
                cog.outl(';')
            ]]]
            [[[end]]]
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

    [[[cog
    for v in vars:
        cog.outl("// - "+v.name()+" - //")
        cog.outl(("inline {type} {name}() const {{\n"
                 "    return {member_name};\n"
                 "}}\n"
                 "inline void {name}({type} {name}) {{\n"
                 "    {setter}\n"
                 "}}\n"
                 +v.type().additional_setter()+"\n").format(type=v.type().ref_type(), cpp_type=v.type().cpp_type(),
                                                            name=v.name(), member_name=v.member_name(),
                                                            setter=v.setter()))
    ]]]
    [[[end]]]
protected:
    virtual sqlite::cryptosqlite_database *db() = 0;
    int64_t mId;

    // --
    [[[cog
        for v in vars:
            cog.outl("{type} {member_name};".format(member_name=v.member_name(), type=v.type().cpp_type()))
    ]]]
    [[[end]]]
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
