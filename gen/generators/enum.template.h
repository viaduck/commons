[[[cog
    import cog
    from gen_enum import EnumDef
    e_def = EnumDef(def_file)

    e_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    e_def.outl("#ifndef {name}_H")
    e_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <string>
#include <iostream>

[[[cog
    e_def.out("{doxygen}")
    e_def.outl("enum class {name} : {type} {{")
    for elem in e_def.elements:
        elem.outl("    {value},        {comment}")
    e_def.outl("}};")

    e_def.outl("inline std::string toString(const {name} &e) {{")
    e_def.outl("    switch (e) {{")
    for elem in e_def.elements:
        elem.outl('    case {e_def.name}::{value}: return "{value}";', e_def=e_def)
    e_def.outl('    }}');
    e_def.outl('    return "";');
    e_def.outl("}}");

    e_def.outl("inline {type} toInt(const {name} &e) {{")
    e_def.outl('    return static_cast<{type}>(e);')
    e_def.outl("}}");

    e_def.outl("inline {name} to{name}({type} val) {{")
    e_def.outl('    if (val > {max_val}) return {name}::{invalid_val};')
    e_def.outl('    return static_cast<{name}>(val);')
    e_def.outl("}}");

    e_def.outl("inline std::ostream &operator<<(std::ostream &os, const {name} &e) {{")
    e_def.outl("    return (os << toString(e));")
    e_def.outl("}}");

    e_def.outl("#endif //{name}_H")
]]]
[[[end]]]