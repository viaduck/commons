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
    f_def = FlatbuffersDef(def_base_dir, def_file, out_file)

    f_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    f_def.outl("#ifndef {name}_H")
    f_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <secure_memory/ISerializable.h>
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
    f_def.outl("class {name} : {serialize_modifier} ISerializable {{")

    if f_def.max_size > 0:
        f_def.outl("    const static uint32_t MAX_SIZE = {max_size};")
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
            elem.outl("    {pub_name}(__{name});")
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
                elem.outl("    {pub_name}(__{name});")
            f_def.outl("}}\n")

        for elem in f_def.elements:
            # getters
            if "basic" in elem.getter:
                elem.outl("inline {type.ref_type} {pub_name}() const {{\n"
                          "    return _{name};\n"
                          "}}")
            if "type_wrap" in elem.getter:
                elem.outl("inline {type.ref_type} {pub_name}Value() const {{\n"
                          "    return _{name};\n"
                          "}}")
                elem.outl("inline {wrap_type} {pub_name}() const {{\n"
                          "    return " + elem.wrap_to_type + ";\n"
                          "}}")

            # setters
            if "basic" in elem.setter:
                elem.outl("inline void {pub_name}({type.ref_type} v) {{\n"
                          "    _{name} = v;\n"
                          "}}")
            if "type_wrap" in elem.setter:
                elem.outl("inline void {pub_name}({set_wrap_type} v) {{\n"
                          "    _{name} = " + elem.wrap_from_type + ";\n"
                          "}}")
            if "bytes" in elem.setter:
                elem.outl("inline void {pub_name}(const uint8_t *v, uint32_t size) {{\n"
                          "    _{name}.clear();\n"
                          "    _{name}.write(v, size, 0);\n"
                          "}}")
                elem.outl("inline void {pub_name}(const BufferRangeConst &v) {{\n"
                          "    _{name}.clear();\n"
                          "    _{name}.write(v, 0);\n"
                          "}}")
                if elem.size > 0:
                    elem.outl("inline uint32_t {pub_name}_size() {{\n"
                              "    return {size}u;\n"
                              "}}")

            # modifier
            if elem.type.is_ref:
                elem.outl("inline {type.ref_mod_type} {pub_name}() {{\n"
                          "    return _{name};\n"
                          "}}")

            elem.outl("")

        # all-in-one setter
        f_def.out("void set(")
        f_def.reset_list()
        for elem in f_def.elements:
            if "type_wrap" in elem.setter:
                elem.lout("{set_wrap_type} {name}_")
            elif "basic" in elem.setter or "bytes" in elem.setter:
                elem.lout("{type.ref_type} {name}_")
        f_def.outl(") {{")

        for elem in f_def.elements:
            elem.outl("    {pub_name}({name}_);")
        f_def.outl("}}")
    ]]]
    [[[end]]]

    void clear() {
        [[[cog
            for elem in f_def.elements:
                elem.outl(elem.type.reset + ";")
        ]]]
        [[[end]]]
    }

    // (de)serialization
[[[cog
    f_def.outl("{serialize_modifier}:")
]]]
[[[end]]]
    void serialize(Buffer &out) const {
        BufferRange r = out.end();
        return serialize(r);
    }

    void serialize(BufferRange &out) const {
        flatbuffers::FlatBufferBuilder fbb;
        fbb.ForceDefaults(true);
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
        if (!out.ensureSize(fbb.GetSize()))
            throw std::range_error("BufferRange::ensureSize failed");
        out.object().write(fbb.GetBufferPointer(), fbb.GetSize(), out.offset());
    }

    bool deserialize(const BufferRangeConst &in) {
        uint32_t unused;
        return deserialize(in.const_data(), in.size(), unused);
    }

    bool deserialize(const Buffer &in) {
        uint32_t unused;
        return deserialize(in.const_data(), in.size(), unused);
    }

    bool deserialize(const Buffer &in, uint32_t &missing) {
        return deserialize(in.const_data(), in.size(), missing);
    }

    bool deserialize(const uint8_t *in, uint32_t size, uint32_t &missing) {
        // invalidate data before deserialization to avoid confusion
        clear();

        // check if buffer has size indicator
        if (size < 4) {
            missing = 4 - size;
            return false;
        }

        uint32_t full_size = 4 + flatbuffers::GetPrefixedSize(in);

        // size check
        [[[cog
            if f_def.max_size > 0:
                f_def.outl("if (full_size > MAX_SIZE) {{\n"
                            "    missing = 0;\n"
                            "    return false;\n"
                            "}}")
        ]]]
        [[[end]]]
        if (size < full_size) {
            // tell caller to retry with the missing bytes
            missing = full_size - size;
            return false;
        }

        [[[cog
            f_def.outl("flatbuffers::Verifier v(in, size);\n"
                       "if (!internal::VerifySizePrefixed{name}Buffer(v)) {{\n"
                       "    missing = 0;\n"
                       "    return false;\n"
                       "}}\n\n"
                       "auto ptr = flatbuffers::GetSizePrefixedRoot<internal::{name}>(in);")

            for elem in f_def.elements:
                elem.outl(elem.type.unpack + ";")

            f_def.outl("")

            # size check
            for elem in f_def.elements:
                if elem.size > 0:
                    elem.outl("if (!{pub_name}().empty() && {pub_name}().size() != {size}) {{\n"
                              "    missing = 0;\n"
                              "    return false;\n"
                              "}}")
        ]]]
        [[[end]]]
        return true;
    }

protected:
    static flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>> CreateVectorOfBuffers(
            flatbuffers::FlatBufferBuilder &fbb, const std::vector<Buffer> &v) {
        std::vector<flatbuffers::Offset<flatbuffers::String>> elems(v.size());
        for (size_t i = 0; i < v.size(); i++) elems[i] = fbb.CreateString(v[i].const_data<char>(), v[i].size());
        return fbb.CreateVector(elems);
    }

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
