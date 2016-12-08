[[[cog
import cog
import generator as g
from os.path import basename, splitext

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
#include <iostream>

[[[cog
    cog.outl("enum class "+name+" {")

    vals = list(g.do(filename))
    # iterate over enum values
    for val in vals:
        id = val[0]
        comment = val[1]
        cog.outl("    {value},        {comment}".format(value=id, comment=comment))
]]]
[[[end]]]
};

[[[cog
    cog.outl("inline std::string toString(const {name} &e) {{".format(name=name))
    cog.outl("    switch (e) {")
    for val in vals:
        id = val[0]
        cog.outl('    case {name}::{value}: return "{name}::{value}";'.format(name=name, value=id))
    cog.outl('    }');
    cog.outl('    return "";');
    cog.outl("}");
]]]
[[[end]]]

[[[cog
cog.outl("inline std::ostream &operator<<(std::ostream &os, const {name} &e) {{".format(name=name))
cog.outl("    return (os << toString(e));")
cog.outl("}");
]]]
[[[end]]]

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
