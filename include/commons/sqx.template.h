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
#include <commons/SQXBase.h>

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
            for v in vars:
                param_list.append(v.create_stmt().format(**v.format_kwargs()))
            cog.outl('db << "CREATE TABLE {name} (pid INTEGER PRIMARY KEY, {param_list});";'.format(name=name, param_list=', '.join(param_list)))
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
        [[[cog
            for v in vars:
                store_hook = v.store_hook().format(**v.format_kwargs())
                if len(store_hook) > 0:
                    cog.outl(store_hook)
        ]]]
        [[[end]]]
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
    virtual void remove() {
        if (mId >= 0) {
            [[[cog
                cog.outl('*db() << "DELETE FROM {name} WHERE pid = ?;" << mId;'.format(name=name))
            ]]]
            [[[end]]]
        }
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
            cog.outl("{member_type} {member_name};".format(**v.format_kwargs()))
    ]]]
    [[[end]]]
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
