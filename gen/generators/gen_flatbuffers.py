import copy
# Copyright (C) 2023 The ViaDuck Project
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

# matches "type name", "type[] name", "type(500) name", "~type name" and "virtual type name"
matcher = re.compile(r"(?P<depr>~)?(?P<virt>virtual)?\s*(?P<type>[\w\[\]]*)(?P<size>\(\d+\))?\s+(?P<name>\w*)" + comment_pattern)
# matches "max_size <bytes>"
size_matcher = re.compile(r"max_size\s+(?P<max_size>\d*)" + comment_pattern)
# matches camel case transition from upper to lower case "IDTest" -> I[D][T]est
case_matcher_utl = re.compile(r"([^_\s])([A-Z][a-z])")
# matches camel case transition from lower to upper case "testID" -> tes[t][I]D
case_matcher_ltu = re.compile(r"([a-z0-9])([A-Z])")
# matches "import flatbuffers/path/to/EnumName.the"
import_matcher = re.compile(r"import\s+(?P<path>flatbuffers.+)" + comment_pattern)
# matches "from custom/import/SomeFile.h import CustomType
custom_import_matcher = re.compile(r"from\s+(?P<path>\S+)\s+import\s+(?P<name>\w+)" + comment_pattern)


# types of enums
enum_types = {}
# types of bitfields
bit_types = {}
# types of embedded flatbuffers
flatbuffers_types = {}


# convert CamelCase/camelCase to snake_case
def convert_case(string):
    string = case_matcher_utl.sub(r'\1_\2', string)
    return case_matcher_ltu.sub(r'\1_\2', string).lower()


class FlatbuffersTypeDef:
    def __init__(self, t_name, fbs_type, default="0", m_type=None, r_type=None, v_type=None, pack=None, unpack=None,
                 reset=None, empty_check=None):
        """
        :param t_name: Name of fbx type. This type is used in fbx definitions.
        :param fbs_type: Name of fbs base type. This type is used in internal fbs definition.
        :param default: Default value of this type.
        :param v_type: Virtual type. This type is the one used by flatbuffers in Offset<...> templates.
        Defaults to t_name.
        :param m_type: Member type. This type is used in C++ members to save this type into. Defaults to t_name.
        :param r_type: Reference type. This type is used when a C++ mutable reference to this type is needed. Defaults
        to t_name.
        :param pack: C++ expression used to "pack" this type and serialize its value. Defaults to returning the member
        variable.
        :param unpack: C++ expression used to "unpack" this type and deserialize its value. Defaults to assigning to
        the member variable.
        :param reset: C++ expression used to "reset" (i.e. clean) the member variable. Defaults to assigning the
        default to the member variable.
        :param empty_check: C++ expression used to check if member variable is "empty" (i.e. unset from default ctor).
        Defaults to comparing the member variable to its default value.
        """
        # usual types
        self.type_name = t_name
        self.fbs_type = fbs_type
        self.default = default
        self.member_type = t_name if m_type is None else m_type

        # reference types
        self.is_ref = r_type is not None
        self.ref_type = t_name if r_type is None else "const " + r_type
        self.ref_mod_type = r_type

        # virtual types
        v_type = t_name if v_type is None else v_type
        self.v_type = "flatbuffers::Offset<"+v_type+">" if self.is_ref else v_type

        # (un)pack
        self.pack = "_{name}" if pack is None else pack
        self.unpack = "_{name} = ptr->{name}()" if unpack is None else unpack
        self.reset = ("_{name} = " + default) if reset is None else reset

        # misc
        self.empty_check = "_{name} == " + default if empty_check is None else empty_check


class FlatbuffersVectorTypeDef(FlatbuffersTypeDef):
    def __init__(self, t_name, fbs_type=None, base_for_v=None, create_type="fbb.CreateVector(_{name})", load_type="i"):
        base_type = flatbuffers_type[t_name]
        base_v_type = base_type.v_type if not base_for_v else flatbuffers_type[base_for_v].v_type

        super().__init__(
            t_name + "[]", "", "",
            "std::vector<"+base_type.member_type+">", "std::vector<"+base_type.member_type+"> &",
            "flatbuffers::Vector<"+base_v_type+">",
            "_{name}.empty() ? 0 : "+create_type,
            "if (ptr->{name}()) for (auto i : *ptr->{name}()) _{name}.push_back("+load_type+")",
            "_{name}.clear()",
            "_{name}.empty()"
        )

        self.fbs_type = "[" + base_type.fbs_type + "]" if fbs_type is None else fbs_type


class FlatbuffersEmbeddedTypeDef(FlatbuffersTypeDef):
    def __init__(self, t_name):
        super().__init__(
            t_name, "[ubyte]", "",
            None, t_name + " &",
            "flatbuffers::Vector<uint8_t>",
            "{name}_packed.empty() ? 0 : fbb.CreateVector({name}_packed.const_data(), {name}_packed.size())",
            "if (ptr->{name}() && !_{name}.deserialize(ptr->{name}()->Data(), ptr->{name}()->size(), unused))\n"
            "    return false",
            "_{name}.clear()",
            "_{name}.empty()",
        )

        self.pre_pack = "Buffer {name}_packed;\n_{name}.serialize({name}_packed);"


flatbuffers_type = {
    "bytes": FlatbuffersTypeDef(
        "bytes", "[ubyte]", "",
        "Buffer", "Buffer &",
        "flatbuffers::Vector<uint8_t>",
        "_{name}.empty() ? 0 : fbb.CreateVector(_{name}.const_data(), _{name}.size())",
        "if (ptr->{name}()) _{name}.write(ptr->{name}()->Data(), ptr->{name}()->size(), 0)",
        "_{name}.clear()", "_{name}.empty()"),
    "string": FlatbuffersTypeDef(
        "string", "string", "",
        "std::string", "std::string &",
        "flatbuffers::String",
        "_{name}.empty() ? 0 : fbb.CreateString(_{name})",
        "if (ptr->{name}()) _{name} = ptr->{name}()->str()",
    "json": FlatbuffersTypeDef(
        "json", "string", "",
        "nlohmann::json", "nlohmann::json &",
        "flatbuffers::String",
        "_{name}.empty() ? 0 : fbb.CreateString(_{name}.dump())",
        "if (ptr->{name}())\n"
        "    _{name} = nlohmann::json::parse(ptr->{name}()->str());\n"
        "if (_{name}.type() == nlohmann::json::value_t::discarded) {{\n"
        "    missing = 0;\n"
        "    return false;\n"
        "}}",
        "_{name}.clear()", "_{name}.empty()"),
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
        t_vec = FlatbuffersVectorTypeDef(
            el_name, None,
            "string", "fbb.CreateVectorOfStrings(_{name})",
            "i->str()")
    elif el_name == "bytes":
        t_vec = FlatbuffersVectorTypeDef(
            el_name, "[string]",
            "string", "CreateVectorOfBuffers(fbb, _{name})",
            "Buffer(i->data(), i->size())")

    if t_vec is not None:
        flatbuffers_type[t_vec.type_name] = t_vec


class FlatbuffersType(CogBase):
    def __init__(self, elem_type, elem_name, elem_size=0):
        self.base_type = flatbuffers_type[elem_type] if elem_type is not None else None
        self.pub_name = elem_name
        self.name = convert_case(elem_name)
        self.size = elem_size
        self.is_virtual = False

        # for wrapped types
        self.wrap_type = None
        self.set_wrap_type = None
        self.wrap_to_type = None
        self.wrap_from_type = None

        self.getter = ["basic"]
        self.setter = ["basic"]
        if elem_type == "bytes":
            # special setter for bytes
            self.setter.append("bytes")

    def fbs_line(self, is_depr):
        # line in fbs file for this field
        depr = " (deprecated)" if is_depr else ""
        return self.name + ":" + self.base_type.fbs_type + depr + ";"


class FlatbuffersEnumType(FlatbuffersType):
    def __init__(self, elem_type, elem_name, e_def):
        super().__init__(e_def.type, elem_name)

        # enum specifics
        self.wrap_type = elem_type
        self.set_wrap_type = elem_type
        self.wrap_to_type = "to{wrap_type}(_{name})"
        self.wrap_from_type = "toInt(v)"
        self.getter = ["type_wrap"]
        self.setter.append("type_wrap")


class FlatbuffersBitType(FlatbuffersType):
    def __init__(self, elem_type, elem_name, b_def):
        super().__init__(b_def.type, elem_name)

        # bitfield specifics
        self.wrap_type = elem_type
        self.set_wrap_type = "const " + elem_type + " &"
        self.wrap_to_type = "{wrap_type}(_{name})"
        self.wrap_from_type = "v.value()"
        self.getter = ["type_wrap"]
        self.setter.append("type_wrap")


class FlatbuffersEmbeddedType(FlatbuffersType):
    def __init__(self, elem_name, base_type):
        super().__init__("bytes", elem_name)

        self.base_type = base_type
        self.setter = ["basic", "embedded"]


def virtualize_type(f_type):
    # triggers generation of virtual serialize_{name} and deserialize_{name} functions
    f_type.is_virtual = True

    # copy base_type before modifying it to avoid affecting other types with same base_type
    f_type.base_type = copy.deepcopy(f_type.base_type)
    # switch pack to vpack and instead call serialize_{name} to pack
    f_type.base_type.vpack = f_type.base_type.pack
    f_type.base_type.pack = "serialize_{pub_name}(fbb)"

    return f_type


class FlatbuffersCustomDef:
    def __init__(self, c_name, c_path):
        self.name = c_name
        self.type = FlatbuffersEmbeddedTypeDef(c_name)
        self.import_path = c_path


# all flatbuffers definitions
class FlatbuffersDef(DefBase, CogBase):
    def __init__(self, base_dir, filename, outfile=None):
        DefBase.__init__(self, base_dir, filename)
        self.max_size = 500

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

        # for flatbuffers imports
        self.type = FlatbuffersEmbeddedTypeDef(self.name)
        self.import_path = splitext(filename)[0] + ".h"

        # write to fbs file
        if outfile:
            fbs_path = join(dirname(outfile), self.name + ".fbs")
            with open(fbs_path, mode='w') as f:
                f.write("\n".join(self.fbs))

    def parse_line(self, line):
        e_def = enum_import(self.base_dir, line)
        b_def = bit_import(self.base_dir, line)
        f_def = flatbuffers_import(self.base_dir, line)
        cf_def = custom_import(line)

        if e_def is not None:
            # add enum as a custom type
            enum_types.update({e_def.name: e_def})
            self.includes.append(e_def.import_path)
            return []

        elif b_def is not None:
            # add bitfield as custom type
            bit_types.update({b_def.name: b_def})
            self.includes.append(b_def.import_path)
            return []

        elif f_def is not None:
            # add flatbuffers as custom type
            flatbuffers_types.update({f_def.name: f_def})
            self.includes.append(f_def.import_path)
            return []

        elif cf_def is not None:
            # custom (serializable) type
            flatbuffers_types.update({cf_def.name: cf_def})
            self.includes.append(cf_def.import_path)
            return []

        match = matcher.match(line)
        size_match = size_matcher.match(line)

        if size_match is not None:
            self.max_size = int(size_match.group('max_size').strip())
            return []

        elif match is not None:
            is_depr = match.group('depr') is not None
            is_virt = match.group('virt') is not None
            elem_type = match.group('type').strip()
            elem_name = match.group('name').strip()
            elem_size = 0 if match.group('size') is None else int(match.group('size')[1:-1])

            # handle enum or bitfield types
            if elem_type in enum_types:
                result = FlatbuffersEnumType(elem_type, elem_name, enum_types[elem_type])
            elif elem_type in bit_types:
                result = FlatbuffersBitType(elem_type, elem_name, bit_types[elem_type])
            elif elem_type in flatbuffers_types:
                result = FlatbuffersEmbeddedType(elem_name, flatbuffers_types[elem_type].type)
            else:
                result = FlatbuffersType(elem_type, elem_name, elem_size)

            if is_virt:
                result = virtualize_type(result)

            # add to fbs with deprecation info
            self.fbs.append(result.fbs_line(is_depr))

            # do not add deprecated fields to elements
            return [] if is_depr else [result]

        raise Exception("parse error on line: " + line)


def custom_import(line):
    # match custom import
    cm = custom_import_matcher.match(line)
    if cm is None:
        return None

    # extract
    c_name = cm.group('name').strip()
    c_path = cm.group('path').strip()

    # return definition from path
    return FlatbuffersCustomDef(c_name, c_path)


def flatbuffers_import(base_dir, line):
    # match import
    m = import_matcher.match(line)
    if m is None:
        return None

    # extract path
    path = m.group('path').strip()

    # return definition from path
    return FlatbuffersDef(base_dir, path)
