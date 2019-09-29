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
    from gen_enum import EnumDef
    e_def = EnumDef(def_base_dir, def_file)

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
        elem.outl('    case {e_def.name}::{value}: return "{e_def.name}::{value}";', e_def=e_def)
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