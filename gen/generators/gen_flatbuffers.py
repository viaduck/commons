import re
from os.path import basename, dirname, splitext, join
from common import CogBase, DefBase, read_definition, suggested_type, comment_pattern

from generators.gen_enum import enum_import
from generators.gen_bit import bit_import

# matches non-array "type name"
matcher = re.compile(r"(?P<type>\w*)\s+(?P<name>\w*)" + comment_pattern)

# types of enums
enum_types = {}
# types of bitfields
bit_types = {}


class FlatbuffersTypeDef:
    def __init__(self, t_name, f_type, default="0", m_type=None, r_type=None, pack=None, unpack=None):
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


flatbuffers_type = {
    "bytes": FlatbuffersTypeDef(
        "bytes", "[ubyte]", "", "Buffer", "Buffer &",
        "_{name}.size() > 0 ? fbb.CreateVector(static_cast<const uint8_t*>(_{name}.const_data()), _{name}.size()) : 0",
        "if (ptr->{name}()) _{name}.write(ptr->{name}()->Data(), ptr->{name}()->Length(), 0)"),
    "string": FlatbuffersTypeDef(
        "string", "string", "", "std::string", "std::string &",
        "_{name}.empty ? 0 : fbb.CreateString(_{name})",
        "if (ptr->{name}()) _{name} = ptr->{name}()->str()"),
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


class FlatbuffersType(CogBase):
    def __init__(self, elem_type, elem_name):
        self.type = flatbuffers_type[elem_type]
        self.name = elem_name

        # for wrapped types
        self.wrap_type = None
        self.set_wrap_type = None
        self.wrap_to_type = None
        self.wrap_from_type = None

        self.getter = ['basic']
        if elem_type == "bytes":
            # special setter for bytes
            self.setter = ['bytes']
        else:
            self.setter = ['basic']

    def fbs_line(self):
        # line in fbs file for this field
        return self.name + ":" + self.type.fbs_type + ";"


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
    def __init__(self, filename, outfile):
        DefBase.__init__(self, filename)

        # name fbs table after basename of the file
        self.name = splitext(basename(filename))[0]

        # create elements
        self.parse()

        # generate fbs definition
        self.fbs = ["namespace internal;",
                    "table " + self.name + " {"]
        for elem in self.elements:
            self.fbs.append(elem.fbs_line())
        self.fbs.append("}")
        self.fbs.append("root_type " + self.name + ";")

        # write to fbs file
        fbs_path = join(dirname(outfile), self.name + ".fbs")
        with open(fbs_path, mode='w') as f:
            f.write("\n".join(self.fbs))

    def parse_line(self, line):
        e_def = enum_import(line)
        b_def = bit_import(line)
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
        if match is not None:
            elem_type = match.group('type').strip()
            elem_name = match.group('name').strip()

            # handle enum or bitfield types
            if elem_type in enum_types:
                return [FlatbuffersEnumType(elem_type, elem_name, enum_types[elem_type])]
            elif elem_type in bit_types:
                return [FlatbuffersBitType(elem_type, elem_name, bit_types[elem_type])]
            else:
                return [FlatbuffersType(elem_type, elem_name)]

        raise Exception("parse error on line: " + line)
