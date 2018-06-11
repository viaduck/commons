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
    from gen_flatbuffers import FlatbuffersDef
    f_def = FlatbuffersDef(def_file, out_file)

    f_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    f_def.outl("#ifndef {name}_H")
    f_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <secure_memory/Serializable.h>
#include <secure_memory/Buffer.h>
#include <secure_memory/Range.h>
#include <secure_memory/conversions.h>
#include <commons/Bitfield.h>
#include <flatbuffers/flatbuffers.h>

#include <cstring>

[[[cog
    f_def.outl("#include \"{name}_generated.h\"")

    # imported includes
    for i_def in f_def.includes:
        i_def.outl('#include "{import_path}"')

    f_def.outl("")
    f_def.out("{doxygen}")
    f_def.outl("class {name} : public Serializable {{")
]]]
[[[end]]]
public:

    [[[cog
        # default ctor
        f_def.out("{name}()")
        # only if ref types exist
        if sum(1 for elem in f_def.elements if not elem.type.is_ref) > 0:
            f_def.out(" : ")
            f_def.reset_list()
            for elem in f_def.elements:
                if not elem.type.is_ref:
                    elem.lout("_{name}({type.default})")
        f_def.outl(" {{ }}\n")

        # arguments ctor
        f_def.out("{name}(")
        f_def.reset_list()
        for elem in f_def.elements:
            if "type_wrap" in elem.setter:
                elem.lout("{set_wrap_type} __{name}")
            else:
                elem.lout("{type.ref_type} __{name}")
        f_def.outl(") {{")
        for elem in f_def.elements:
            elem.outl("    {name}(__{name});")
        f_def.outl("}}\n")

        # arguments ctor with ranges if buffer exist
        if sum(1 for elem in f_def.elements if "bytes" in elem.setter) > 0:
            f_def.out("{name}(")
            f_def.reset_list()
            for elem in f_def.elements:
                if "type_wrap" in elem.setter:
                    elem.lout("{set_wrap_type} __{name}")
                elif "bytes" in elem.setter:
                    elem.lout("const BufferRangeConst &__{name}")
                else:
                    elem.lout("{type.ref_type} __{name}")
            f_def.outl(") {{")
            for elem in f_def.elements:
                elem.outl("    {name}(__{name});")
            f_def.outl("}}\n")

        for elem in f_def.elements:
            # getters
            if "basic" in elem.getter:
                elem.outl("inline {type.ref_type} {name}() const {{\n"
                          "    return _{name};\n"
                          "}}")
            if "type_wrap" in elem.getter:
                elem.outl("inline {type.ref_type} {name}Value() const {{\n"
                          "    return _{name};\n"
                          "}}")
                elem.outl("inline {wrap_type} {name}() const {{\n"
                          "    return " + elem.wrap_to_type + ";\n"
                          "}}")

            # setters
            if "basic" in elem.setter:
                elem.outl("inline void {name}({type.ref_type} v) {{\n"
                          "    _{name} = v;\n"
                          "}}")
            if "type_wrap" in elem.setter:
                elem.outl("inline void {name}({set_wrap_type} v) {{\n"
                          "    _{name} = " + elem.wrap_from_type + ";\n"
                          "}}")
            if "bytes" in elem.setter:
                elem.outl("inline void {name}({type.ref_type} v) {{\n"
                          "    _{name}.write(v, 0);\n"
                          "}}")
                elem.outl("inline void {name}(const uint8_t *v, uint32_t size) {{\n"
                          "    _{name}.write(v, size, 0);\n"
                          "}}")
                elem.outl("inline void {name}(const BufferRangeConst &v) {{\n"
                          "    _{name}.write(v, 0);\n"
                          "}}")

            # modifier
            if elem.type.is_ref:
                elem.outl("inline {type.ref_mod_type} {name}() {{\n"
                          "    return _{name};\n"
                          "}}")

            elem.outl("")
    ]]]
    [[[end]]]
    // (de)serialization

    void serialize(Buffer &out) const {
        flatbuffers::FlatBufferBuilder fbb;
        fbb.FinishSizePrefixed(
            [[[cog
                f_def.out("internal::Create{name}(fbb")

                for elem in f_def.elements:
                    elem.outl(",")
                    elem.out(elem.type.pack)

                f_def.out(")")
            ]]]
            [[[end]]]
        );
        out.write(fbb.GetBufferPointer(), fbb.GetSize(), 0);
    }

    bool deserialize(const Buffer &in) {
        uint32_t unused;
        return deserialize(in, unused);
    }

    bool deserialize(const Buffer &in, uint32_t &missing) {
        // check if buffer has size indicator
        if (in.size() < 4) {
            missing = 4 - in.size();
            return false;
        }

        // check size
        uint32_t full_size = 4 + flatbuffers::GetPrefixedSize(static_cast<const uint8_t*>(in.const_data()));
        if (in.size() < full_size) {
            missing = full_size - in.size();
            return false;
        }

        [[[cog
            f_def.outl("auto ptr = flatbuffers::GetSizePrefixedRoot<internal::{name}>(in.const_data());\n"
                       "flatbuffers::Verifier v(static_cast<const uint8_t*>(in.const_data()), in.size());\n"
                       "if (!ptr->Verify(v))\n"
                       "    return false;\n")

            for elem in f_def.elements:
                elem.outl(elem.type.unpack + ";")
        ]]]
        [[[end]]]
        return true;
    }

protected:
    [[[cog
        for elem in f_def.elements:
            elem.outl("{type.member_type} _{name};")
    ]]]
    [[[end]]]

[[[cog
    f_def.outl("}};")
    f_def.outl("#endif //{name}_H")
]]]
[[[end]]]