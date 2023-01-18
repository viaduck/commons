/*
 * Copyright (C) 2023 The ViaDuck Project
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

#include <iostream>
#include <sstream>
#include <string>

[[[cog
    e_def.out("{doxygen}")
    e_def.outl("enum class {name} : {type} {{")
    for elem in e_def.elements:
        elem.outl("    {name}{value},        {comment}")
    e_def.outl("}};")

    e_def.outl("inline std::string toString(const {name} &e);")

    e_def.outl("inline std::ostream &operator<<(std::ostream &os, const {name} &e) {{")
    e_def.outl("    return (os << toString(e));")
    e_def.outl("}}");

    if e_def.flags:
        e_def.outl("inline {name} operator|({name} a, {name} b) {{")
        e_def.outl("    return {name}(static_cast<{type}>(a) | static_cast<{type}>(b));")
        e_def.outl("}}");

        e_def.outl("inline {name} operator&({name} a, {name} b) {{")
        e_def.outl("    return {name}(static_cast<{type}>(a) & static_cast<{type}>(b));")
        e_def.outl("}}");

        e_def.outl("inline std::string toString(const {name} &e) {{")
        e_def.outl("    std::stringstream bruce;")
        e_def.outl("    int i = 0;")

        # NONE and ALL flag value
        special_indices = (0, len(e_def.elements)-1)
        for i in special_indices:
            e_def.outl("    if (e == {name}::{elem.name})", elem=e_def.elements[i])
            e_def.outl('        return "{name}::{elem.name}";', elem=e_def.elements[i])

        for i, elem in enumerate(e_def.elements):
            if i not in special_indices:
                elem.outl('    if ((e & {e_def.name}::{name}) == {e_def.name}::{name})', e_def=e_def)
                elem.outl('        bruce << (i++ > 0 ? " | " : "") << "{e_def.name}::{name}";', e_def=e_def)
        e_def.outl('    return bruce.str();');
        e_def.outl("}}");
    else:
        e_def.outl("inline std::string toString(const {name} &e) {{")
        e_def.outl("    switch (e) {{")
        for elem in e_def.elements:
            elem.outl('    case {e_def.name}::{name}: return "{e_def.name}::{name}";', e_def=e_def)
        e_def.outl('    }}');
        e_def.outl('    return "";');
        e_def.outl("}}");

    e_def.outl("inline {type} toInt(const {name} &e) {{")
    e_def.outl('    return static_cast<{type}>(e);')
    e_def.outl("}}");

    e_def.outl("inline {name} to{name}({type} val) {{")
    e_def.outl('    if (val > {max_val}) return {name}::{elem_invalid_val};')
    e_def.outl('    return static_cast<{name}>(val);')
    e_def.outl("}}");

    e_def.outl("template<typename T>")
    e_def.outl("T toEnum({type});")

    e_def.outl("template<>")
    e_def.outl("inline {name} toEnum({type} val) {{")
    e_def.outl("    return to{name}(val);")
    e_def.outl("}}")

    e_def.outl("#endif //{name}_H")
]]]
[[[end]]]