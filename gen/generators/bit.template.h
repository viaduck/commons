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
    from gen_bit import BitDef
    b_def = BitDef(def_file)

    b_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    b_def.outl("#ifndef {name}_H")
    b_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <secure_memory/conversions.h>
#include <commons/Bitfield.h>

[[[cog
    b_def.out("{doxygen}")
    b_def.outl("class {name} {{")
]]]
[[[end]]]
public:
    // constructors
    [[[cog
        # ctr with existing field
        b_def.outl("explicit {name}({type} field = 0) : mField(field) {{ }}\n")

        # copy ctr
        b_def.outl("{name}(const {name} &other) : mField(other.mField) {{ }}\n")

        # ctr with new field, set args
        b_def.out("{name}(")

        BitDef.reset_list()
        for elem in b_def.elements:
            elem.lout("{type} _{name}")

        b_def.outl(") : {name}() {{")
        for elem in b_def.elements:
            elem.outl("    {name}(_{name});")
        b_def.outl("}}")
    ]]]
    [[[end]]]

    // getter and setter

    [[[cog
        b_def.outl("{type} value() const {{\n"
                   "    return mField;\n"
                   "}}\n")

        b_def.outl("void value({type} v) {{\n"
                   "    mField = v;\n"
                   "}}\n")

        for elem in b_def.elements:
            elem.outl("{type} {name}() const {{\n"
                      "    return Bitfield::get<{type}>({offset}, {size}, value());\n"
                      "}}\n")

            elem.outl("uint32_t {name}_width() {{\n"
                      "    return {size};\n"
                      "}}\n")

            elem.outl("void {name}({type} v) {{\n"
                      "    auto fld = value();\n"
                      "    Bitfield::set({offset}, {size}, v, fld);\n"
                      "    value(fld);\n"
                      "}}\n")
    ]]]
    [[[end]]]

private:
    // members
    [[[cog
        b_def.outl("{type} mField;")
    ]]]
    [[[end]]]

[[[cog
    b_def.outl("}};")
    b_def.outl("#endif //{name}_H")
]]]
[[[end]]]
