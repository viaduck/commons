
# Copyright (C) 2018 The ViaDuck Project
#
# This file is part of Commons.
#
# Commons is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Commons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Commons.  If not, see <http://www.gnu.org/licenses/>.

import re
from os.path import basename, dirname, splitext, join

from common import CogBase, DefBase, comment_pattern
from generators.gen_bit import bit_import
from generators.gen_enum import enum_import

# matches non-array "type name"
matcher = re.compile(r"(?P<depr>~)?(?P<type>[\w\[\]]*)(?P<size>\(\d+\))?\s+(?P<name>\w*)" + comment_pattern)
# matches "max_size <bytes>"
size_matcher = re.compile(r"max_size\s+(?P<max_size>\d*)" + comment_pattern)
# matches "serialize_public <true|false>"
serialize_matcher = re.compile(r"serialize_public\s+(?P<public>true|false)" + comment_pattern)
# matches camel case transition from upper to lower case "IDTest" -> I[D][T]est
case_matcher_utl = re.compile(r"([^_\s])([A-Z][a-z])")
# matches camel case transition from lower to upper case "testID" -> tes[t][I]D
case_matcher_ltu = re.compile(r"([a-z0-9])([A-Z])")


# types of enums
enum_types = {}
# types of bitfields
bit_types = {}


# convert CamelCase/camelCase to snake_case
def convert_case(string):
    string = case_matcher_utl.sub(r'\1_\2', string)
    return case_matcher_ltu.sub(r'\1_\2', string).lower()


class FlatbuffersTypeDef:
    def __init__(self, t_name, f_type, default="0", m_type=None, r_type=None, pack=None, unpack=None, reset=None):
        # usual types
        self.type_name = t_name
        self.fbs_type = f_type
        self.member_type = t_name if m_type is None else m_type
        self.default = default

        # reference types
        self.is_ref = r_type is not None
        self.ref_type = t_name if r_type is None else "const " + r_type
        self.ref_mod_type = r_type

        # (un)pack
        self.pack = "_{name}" if pack is None else pack
        self.unpack = "_{name} = ptr->{name}()" if unpack is None else unpack
        self.reset = ("_{name} = " + default) if reset is None else reset


class FlatbuffersVectorTypeDef(FlatbuffersTypeDef):
    def __init__(self, t_name, fbs_type=None, create_type="fbb.CreateVector(_{name})", load_type="i"):
        base_type = flatbuffers_type[t_name]

        super().__init__(
            t_name + "[]", "", "",
            "std::vector<"+base_type.member_type+">",
            "std::vector<"+base_type.member_type+"> &",
            "_{name}.size() > 0 ? "+create_type+" : 0",
            "if (ptr->{name}()) for (auto i : *ptr->{name}()) _{name}.push_back("+load_type+")",
            "_{name}.clear()"
        )

        self.fbs_type = "[" + base_type.fbs_type + "]" if fbs_type is None else fbs_type


flatbuffers_type = {
    "bytes": FlatbuffersTypeDef(
        "bytes", "[ubyte]", "", "Buffer", "Buffer &",
        "_{name}.size() > 0 ? fbb.CreateVector(_{name}.const_data(), _{name}.size()) : 0",
        "if (ptr->{name}()) _{name}.write(ptr->{name}()->Data(), ptr->{name}()->size(), 0)", "_{name}.clear()"),
    "string": FlatbuffersTypeDef(
        "string", "string", "", "std::string", "std::string &",
        "_{name}.empty() ? 0 : fbb.CreateString(_{name})",
        "if (ptr->{name}()) _{name} = ptr->{name}()->str()", "_{name}.clear()"),
    "bool": FlatbuffersTypeDef("bool", "bool", "false"),
    "int8_t": FlatbuffersTypeDef("int8_t", "int8"),
    "uint8_t": FlatbuffersTypeDef("uint8_t", "uint8"),
    "int16_t": FlatbuffersTypeDef("int16_t", "int16"),
    "uint16_t": FlatbuffersTypeDef("uint16_t", "uint16"),
    "int32_t": FlatbuffersTypeDef("int32_t", "int32"),
    "uint32_t": FlatbuffersTypeDef("uint32_t", "uint32"),
    "int64_t": FlatbuffersTypeDef("int64_t", "int64"),
    "uint64_t": FlatbuffersTypeDef("uint64_t", "uint64"),
}
for el_name, el_type in dict(flatbuffers_type).items():
    t_vec = None

    if not el_type.is_ref:
        t_vec = FlatbuffersVectorTypeDef(el_name)
    elif el_name == "string":
        t_vec = FlatbuffersVectorTypeDef(el_name, None, "fbb.CreateVectorOfStrings(_{name})", "i->str()")
    elif el_name == "bytes":
        t_vec = FlatbuffersVectorTypeDef(el_name, "[string]", "CreateVectorOfBuffers(fbb, _{name})",
                                         "Buffer(i->data(), i->size())")

    if t_vec is not None:
        flatbuffers_type[t_vec.type_name] = t_vec


class FlatbuffersType(CogBase):
    def __init__(self, elem_type, elem_name, elem_size=0):
        self.type = flatbuffers_type[elem_type] if elem_type is not None else None
        self.pub_name = elem_name
        self.name = convert_case(elem_name)
        self.size = elem_size

        # for wrapped types
        self.wrap_type = None
        self.set_wrap_type = None
        self.wrap_to_type = None
        self.wrap_from_type = None

        self.getter = ['basic']
        self.setter = ['basic']
        if elem_type == "bytes":
            # special setter for bytes
            self.setter.append('bytes')

    def fbs_line(self, is_depr):
        # line in fbs file for this field
        depr = " (deprecated)" if is_depr else ""
        return self.name + ":" + self.type.fbs_type + depr + ";"


class FlatbuffersEnumType(FlatbuffersType):
    def __init__(self, elem_type, elem_name, e_def):
        super().__init__(e_def.type, elem_name)

        # enum specifics
        self.wrap_type = elem_type
        self.set_wrap_type = elem_type
        self.wrap_to_type = "to{wrap_type}(_{name})"
        self.wrap_from_type = "toInt(v)"
        self.getter = ['type_wrap']
        self.setter.append('type_wrap')


class FlatbuffersBitType(FlatbuffersType):
    def __init__(self, elem_type, elem_name, b_def):
        super().__init__(b_def.type, elem_name)

        # bitfield specifics
        self.wrap_type = elem_type
        self.set_wrap_type = "const " + elem_type + " &"
        self.wrap_to_type = "{wrap_type}(_{name})"
        self.wrap_from_type = "v.value()"
        self.getter = ['type_wrap']
        self.setter.append('type_wrap')


# all flatbuffers definitions
class FlatbuffersDef(DefBase, CogBase):
    def __init__(self, base_dir, filename, outfile):
        DefBase.__init__(self, base_dir, filename)
        self.max_size = 500
        self.serialize_modifier = "public"

        # name fbs table after basename of the file
        self.name = splitext(basename(filename))[0]

        # fbs header
        self.fbs = ["namespace internal;",
                    "table " + self.name + " {"]

        # create elements
        self.parse()

        # fbs footer
        self.fbs.append("}")
        self.fbs.append("root_type " + self.name + ";")

        # write to fbs file
        fbs_path = join(dirname(outfile), self.name + ".fbs")
        with open(fbs_path, mode='w') as f:
            f.write("\n".join(self.fbs))

    def parse_line(self, line):
        e_def = enum_import(self.base_dir, line)
        b_def = bit_import(self.base_dir, line)
        if e_def is not None:
            # add enum as a custom type
            enum_types.update({e_def.name: e_def})
            self.includes.append(e_def)
            return []

        elif b_def is not None:
            # add bitfield as custom type
            bit_types.update({b_def.name: b_def})
            self.includes.append(b_def)
            return []

        match = matcher.match(line)
        size_match = size_matcher.match(line)
        serialize_match = serialize_matcher.match(line)

        if size_match is not None:
            self.max_size = int(size_match.group('max_size').strip())
            return []

        elif serialize_match is not None:
            is_public = serialize_match.group("public").strip() == "true"
            self.serialize_modifier = "public" if is_public else "protected"
            return []

        elif match is not None:
            is_depr = match.group('depr') is not None
            elem_type = match.group('type').strip()
            elem_name = match.group('name').strip()
            elem_size = 0 if match.group('size') is None else int(match.group('size')[1:-1])

            # handle enum or bitfield types
            if elem_type in enum_types:
                result = FlatbuffersEnumType(elem_type, elem_name, enum_types[elem_type])
            elif elem_type in bit_types:
                result = FlatbuffersBitType(elem_type, elem_name, bit_types[elem_type])
            else:
                result = FlatbuffersType(elem_type, elem_name, elem_size)

            # add to fbs with deprecation info
            self.fbs.append(result.fbs_line(is_depr))

            # do not add deprecated fields to elements
            return [] if is_depr else [result]

        raise Exception("parse error on line: " + line)
