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


class FlatbuffersType:
    def __init__(self, t_name, fbs_type, **kwargs):
        """
        :param t_name: Name of fbx type. This type is used in fbx definitions.
        :param fbs_type: Name of fbs base type. This type is used in internal fbs definition.
        :param default: Default value of this type.
        :param m_type: Member type. This type is used in C++ members to save this type into. Defaults to t_name.
        :param r_type: Reference type. This type is used when a C++ mutable reference to this type is needed. Defaults
        to t_name.
        :param v_type: Virtual type. This type is the one used by flatbuffers in Offset<...> templates.
        Defaults to t_name.
        :param pack: C++ expression used to "pack" this type and serialize its value. Defaults to returning the member
        variable.
        :param unpack: C++ expression used to "unpack" this type and deserialize its value. Defaults to assigning to
        the member variable.
        :param reset: C++ expression used to "reset" (i.e. clean) the member variable. Defaults to assigning the
        default to the member variable.
        :param e_check: C++ expression used to check if member variable is "empty" (i.e. unset from default ctor).
        Defaults to comparing the member variable to its default value.
        """
        # usual types
        self.type_name = t_name
        self.fbs_type = fbs_type
        self.member_type = kwargs.get("m_type", t_name)

        # reference types
        self.is_ref = bool(kwargs.get("is_ref", False))
        self.ref_mod_type = self.member_type + " &" if self.is_ref else t_name
        self.ref_type = "const " + self.ref_mod_type if self.is_ref else t_name
        self.default = kwargs.get("default", "{}" if self.is_ref else "0")

        # virtual types
        v_type = kwargs.get("v_type", t_name)
        self.v_type = "flatbuffers::Offset<"+v_type+">" if self.is_ref else v_type

        # (un)pack
        self.assign = kwargs.get("assign", "v")
        self.pack = kwargs.get("pack", "_{name}")
        self.unpack = kwargs.get("unpack", "_{name} = ptr->{name}()")
        self.reset = kwargs.get("reset", "_{name} = " + self.default)

        # empty
        self.e_check = kwargs.get("e_check", "_{name} == " + self.default)


class FlatbuffersWrappedType(FlatbuffersType):
    def __init__(self, t_name, base_t_name, **kwargs):
        self.base_type = copy.deepcopy(flatbuffers_type[base_t_name])

        super().__init__(
            t_name, self.base_type.fbs_type,
            v_type=self.base_type.v_type,
            **kwargs
        )


class FlatbuffersVectorType(FlatbuffersType):
    def __init__(self, t_name, **kwargs):
        self.base_type = copy.deepcopy(flatbuffers_type[t_name])

        base_v_type = flatbuffers_type[kwargs["v_base"]].v_type if "v_base" in kwargs else self.base_type.v_type
        create_type = kwargs["c_type"] if "c_type" in kwargs else "fbb.CreateVector(_{name})"
        fbs_type = kwargs["fbs_type"] if "fbs_type" in kwargs else "[" + self.base_type.fbs_type + "]"
        load_type = kwargs["l_type"] if "l_type" in kwargs else "i"

        super().__init__(
            t_name + "[]", fbs_type,
            m_type="std::vector<"+self.base_type.member_type+">", is_ref=True,
            v_type="flatbuffers::Vector<"+base_v_type+">",
            pack="_{name}.empty() ? 0 : "+create_type,
            unpack="if (ptr->{name}())\n    for (auto i : *ptr->{name}()) _{name}.push_back("+load_type+")",
            reset="_{name}.clear()",
            e_check="_{name}.empty()"
        )


class FlatbuffersEmbeddedType(FlatbuffersType):
    def __init__(self, t_name):
        super().__init__(
            t_name, "[ubyte]", is_ref=True,
            v_type="flatbuffers::Vector<uint8_t>",
            pack="{name}_packed.empty() ? 0 : fbb.CreateVector({name}_packed.const_data(), {name}_packed.size())",
            unpack="if (ptr->{name}() && !_{name}.deserialize(ptr->{name}()->Data(), ptr->{name}()->size(), unused))\n"
            "    return false",
            reset="_{name}.clear()",
            e_check="_{name}.empty()",
        )

        self.pre_pack = "Buffer {name}_packed;\n_{name}.serialize({name}_packed);"


flatbuffers_type = {
    "bytes": FlatbuffersType(
        "bytes", "[ubyte]",
        m_type="Buffer", is_ref=True,
        v_type="flatbuffers::Vector<uint8_t>",
        pack="_{name}.empty() ? 0 : fbb.CreateVector(_{name}.const_data(), _{name}.size())",
        unpack="if (ptr->{name}())\n"
        "    _{name}.write(ptr->{name}()->Data(), ptr->{name}()->size(), 0)",
        reset="_{name}.clear()",
        e_check="_{name}.empty()"),
    "string": FlatbuffersType(
        "string", "string",
        default="\"\"",
        m_type="std::string", is_ref=True,
        v_type="flatbuffers::String",
        pack="_{name}.empty() ? 0 : fbb.CreateString(_{name})",
        unpack="if (ptr->{name}())\n"
        "    _{name} = ptr->{name}()->str()",
        reset="_{name}.clear()",
        e_check="_{name}.empty()"),
    "json": FlatbuffersType(
        "json", "string",
        m_type="nlohmann::json", is_ref=True,
        v_type="flatbuffers::String",
        pack="_{name}.empty() ? 0 : fbb.CreateString(_{name}.dump())",
        unpack="if (ptr->{name}())\n"
        "    _{name} = nlohmann::json::parse(ptr->{name}()->str());\n"
        "if (_{name}.type() == nlohmann::json::value_t::discarded) {{\n"
        "    missing = 0;\n"
        "    return false;\n"
        "}}",
        reset="_{name}.clear()",
        e_check="_{name}.empty()"),
    "bool": FlatbuffersType("bool", "bool", default="false"),
    "int8_t": FlatbuffersType("int8_t", "int8"),
    "uint8_t": FlatbuffersType("uint8_t", "uint8"),
    "int16_t": FlatbuffersType("int16_t", "int16"),
    "uint16_t": FlatbuffersType("uint16_t", "uint16"),
    "int32_t": FlatbuffersType("int32_t", "int32"),
    "uint32_t": FlatbuffersType("uint32_t", "uint32"),
    "int64_t": FlatbuffersType("int64_t", "int64"),
    "uint64_t": FlatbuffersType("uint64_t", "uint64"),
}
for el_name, el_type in dict(flatbuffers_type).items():
    t_vec = None

    if not el_type.is_ref:
        t_vec = FlatbuffersVectorType(el_name)
    elif el_name == "string":
        t_vec = FlatbuffersVectorType(
            el_name, v_base="string",
            c_type="fbb.CreateVectorOfStrings(_{name})", l_type="i->str()")
    elif el_name == "bytes":
        t_vec = FlatbuffersVectorType(
            el_name, fbs_type="[string]", v_base="string",
            c_type="CreateVectorOfBuffers(fbb, _{name})", l_type="Buffer(i->data(), i->size())")

    if t_vec is not None:
        flatbuffers_type[t_vec.type_name] = t_vec


class FlatbuffersElement(CogBase):
    def __init__(self, elem_type, elem_name, elem_size=0):
        self.base_type = elem_type if isinstance(elem_type, FlatbuffersType) \
            else copy.deepcopy(flatbuffers_type[elem_type])
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


class FlatbuffersEnumElement(FlatbuffersElement):
    def __init__(self, elem_type, elem_name, e_def):
        super().__init__(FlatbuffersWrappedType(
            elem_type, e_def.type, default="{base_type.type_name}::VALUE_INVALID",
            assign="to{base_type.type_name}(toInt(v))",
            pack="toInt(_{name})",
            unpack="_{name} = to{base_type.type_name}(ptr->{name}())"
        ), elem_name)


class FlatbuffersBitElement(FlatbuffersElement):
    def __init__(self, elem_type, elem_name, b_def):
        super().__init__(FlatbuffersWrappedType(
            elem_type, b_def.type, is_ref=True,
            pack="_{name}.value()",
            unpack="_{name}.value(ptr->{name}())",
            reset="_{name}.value(0)",
            e_check="_{name}.value() == 0"
        ), elem_name)

        # workaround for this semi-ref type
        self.base_type.v_type = self.base_type.base_type.type_name


class FlatbuffersEmbeddedElement(FlatbuffersElement):
    def __init__(self, elem_name, base_type):
        super().__init__("bytes", elem_name)

        self.base_type = copy.deepcopy(base_type)
        self.setter = ["basic", "embedded"]


def virtualize_element(f_elem):
    # triggers generation of virtual serialize_{name} and deserialize_{name} functions
    f_elem.is_virtual = True

    # switch pack to vpack and instead call serialize_{name} to pack
    f_elem.base_type.vpack = f_elem.base_type.pack
    f_elem.base_type.pack = "serialize_{pub_name}(fbb)"

    return f_elem


class FlatbuffersCustomDef:
    def __init__(self, c_name, c_path):
        self.name = c_name
        self.type = FlatbuffersEmbeddedType(c_name)
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
        self.type = FlatbuffersEmbeddedType(self.name)
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
                result = FlatbuffersEnumElement(elem_type, elem_name, enum_types[elem_type])
            elif elem_type in bit_types:
                result = FlatbuffersBitElement(elem_type, elem_name, bit_types[elem_type])
            elif elem_type in flatbuffers_types:
                result = FlatbuffersEmbeddedElement(elem_name, flatbuffers_types[elem_type].type)
            else:
                result = FlatbuffersElement(elem_type, elem_name, elem_size)

            if is_virt:
                result = virtualize_element(result)

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
