import re
from enum import Enum
from os.path import basename, splitext
from common import CogBase, read_definition, type_bits, bits_type

from generators.gen_enum import enum_import

# matches in-line comment
comment_pattern = r"\s*(?:#.*)?$"
# matches name(bits)
squeeze_name_matcher = re.compile("(?P<name>\w*)\((?P<bits>\d*)\),?\s*?")

# matches integral "type name"
integral_matcher = re.compile(r"(?P<type>\w*)\s+(?P<name>\w*)" + comment_pattern)
# matches squeeze "type name1(12),name2(3)"
squeeze_matcher = re.compile(r"(?P<type>\w*)\s+(?P<name>[(),a-zA-Z0-9_ ]*?)" + comment_pattern)
# matches "type[123] name" integral array
array_matcher = re.compile(r"(?P<type>[\w]*)\[(?P<size>\d+)\]\s+(?P<name>\w*)" + comment_pattern)
# matches "var|Var|VAR name" variable array
var_array_matcher = re.compile(r"(?P<type>[\w]*)\[(?P<size>[a-zA-Z]+)\]\s+(?P<name>\w*)" + comment_pattern)

# types of variable array size indicators
var_types = {"var": "uint8_t", "Var": "uint16_t", "VAR": "uint32_t"}
# types of enums
enum_types = {}


class ProtoIntegralType(CogBase):
    def __init__(self, elem_type, elem_name):
        self.type = elem_type
        self.ref_type = elem_type
        self.name = elem_name

        # calc byte size
        self.type_bytes = int(type_bits(self.type) / 8)
        self.byte_size = self.type_bytes
        self.offset = 0

        # present in constructor
        self.is_in_ctr = True
        # argument in constructor
        self.ctr_arg = "{ref_type} _{name}"
        # load value from argument
        self.ctr_load = "{name}(_{name});"

        # load from mBuffer
        self.load = "return hton(*static_cast<const {type}*>(mBuffer.const_data({offset})));"
        # store from v to mBuffer
        self.store = "*static_cast<{type}*>(mBuffer.data({offset})) = hton(v);"
        # calculate size
        self.size = "return sizeof({type});"

        # execute this in copy ctr
        self.copy_extra = ""
        # execute this in size calc
        self.size_extra = ""

        # for range ctr
        self.ctr_range_arg = self.ctr_arg
        self.ctr_range_load = self.ctr_load

        # basic getter
        self.getter = ['basic']
        self.setter = ['basic']


class ProtoEnumType(ProtoIntegralType):
    def __init__(self, elem_type, elem_name, e_def):
        super().__init__(elem_type, elem_name)

        # underlying type
        self.base_type = e_def.type
        # size calculation
        self.type_bytes = int(type_bits(self.base_type) / 8)
        self.byte_size = self.type_bytes

        # load from mBuffer
        self.load = "return to{type}(hton(*static_cast<const {base_type}*>(mBuffer.const_data({offset}))));"
        # store from v to mBuffer
        self.store = "*static_cast<{base_type}*>(mBuffer.data({offset})) = hton(toInt(v));"
        # calculate size
        self.size = "return sizeof({base_type});"

        # add special enum setter
        self.setter.append('enum')


class ProtoIntegralArrayType(ProtoIntegralType):
    def __init__(self, elem_type, elem_name, elem_count):
        super().__init__(elem_type, elem_name)

        # load value from argument
        self.ctr_load = "{name}(_{name}, {byte_size});"
        # range ctr argument
        self.ctr_range_arg = "const BufferRangeConst _{name}"
        # load from range
        self.ctr_range_load = "{name}(static_cast<const {type}*>(_{name}.const_data()), _{name}.size());"

        # load from mBuffer
        self.load = "return static_cast<const {type}*>(mBuffer.const_data({offset}));"
        # calculate size
        self.size = "return sizeof({type})*{array_size};"

        # array count
        self.array_size = int(elem_count)
        # array byte size
        self.byte_size = self.type_bytes * self.array_size
        # ref type
        self.ref_type = "const " + self.type + "*"

        # replace basic setter with special array setter
        self.setter = ['array']


class ProtoVariableArrayType(ProtoIntegralType):
    def __init__(self, elem_type, elem_name, size_type):
        super().__init__(elem_type, elem_name)

        # var array needs special copy
        self.copy_extra = "mBuffer_{name}(other.mBuffer_{name})"
        # var array needs special size calculation
        self.size_extra = "mBuffer_{name}.size()+sizeof({var_type})+"

        # type of size indicator
        self.var_type = size_type
        # no static size
        self.byte_size = 0
        # no constructor arg
        self.is_in_ctr = False

        # replace basic getter with special var getter
        self.getter = ['var']
        # replace basic setter with special var setter
        self.setter = ['var']


class ProtoSubParentType(ProtoIntegralType):
    def __init__(self, elem_type, elem_name):
        super().__init__(elem_type, elem_name)

        # empty type only for size and offset calculation
        self.is_in_ctr = False
        self.getter = []
        self.setter = []

        # create sub types
        self.subs = []
        for sub_match in squeeze_name_matcher.finditer(elem_name):
            sub_name = sub_match.group('name').strip()
            sub_bits = sub_match.group('bits').strip()

            self.subs.append(ProtoSubType(elem_type, sub_name, sub_bits))

        # set each subs shift offset
        offset = 0
        for sub in self.subs:
            sub.shift_offset = offset
            offset = offset + sub.bits

        # check squeeze overflow
        if offset > self.type_bytes * 8:
            raise Exception("Squeeze overflow in: " + elem_name)


class ProtoSubType(ProtoIntegralType):
    def __init__(self, parent_type, sub_name, sub_bits):
        self.name = sub_name
        self.bits = int(sub_bits)
        self.type = bits_type(self.bits)
        self.parent_type = parent_type
        self.shift_offset = 0

        super().__init__(self.type, self.name)

        # load from mBuffer
        self.load = "return Bitfield::get<{type}>({shift_offset}, {bits}, " \
                    "ntoh(*static_cast<const {parent_type}*>(mBuffer.const_data({offset}))));"
        # store from v to mBuffer
        self.store = "{parent_type} _temp = ntoh(*static_cast<{parent_type}*>(mBuffer.data({offset})));\n" \
                     "    Bitfield::set({shift_offset}, {bits}, v, _temp);\n" \
                     "    *static_cast<{parent_type}*>(mBuffer.data({offset})) = hton(_temp);\n"
        # calculate size
        self.size = "return {bits};"

        # sub types don't affect offset
        self.byte_size = 0


def parse_line(line, elements):
    match = integral_matcher.match(line)
    if match is not None:
        elem_type = match.group('type').strip()
        elem_name = match.group('name').strip()

        # enum or integral
        if elem_type in enum_types:
            elements.append(ProtoEnumType(elem_type, elem_name, enum_types[elem_type]))
        else:
            elements.append(ProtoIntegralType(elem_type, elem_name))
        return

    match = squeeze_matcher.match(line)
    if match is not None:
        elem_type = match.group('type').strip()
        elem_name = match.group('name').strip()

        # create dummy parent element
        sub_parent = ProtoSubParentType(elem_type, elem_name)

        # add subs to elements, then parent
        for sub in sub_parent.subs:
            elements.append(sub)

        # parent is only added to ensure following elements have right offset (parent holds combined size of subs)
        elements.append(sub_parent)
        return

    match = array_matcher.match(line)
    if match is not None:
        elem_type = match.group('type').strip()
        elem_name = match.group('name').strip()
        elem_count = match.group('size').strip()

        # integral array
        elements.append(ProtoIntegralArrayType(elem_type, elem_name, elem_count))
        return

    match = var_array_matcher.match(line)
    if match is not None:
        elem_type = match.group('type').strip()
        elem_name = match.group('name').strip()
        elem_size = match.group('size').strip()

        # variable array
        elements.append(ProtoVariableArrayType(elem_type, elem_name, var_types[elem_size]))
        return

    raise Exception("parse error on line: " + line)


# all protocol definitions
class ProtoDef(CogBase):
    def __init__(self, filename):
        # split input
        includes, body = read_definition(filename)

        # collect enum includes
        self.include_enums = [enum_import(i) for i in includes]
        # add each enum as a custom type
        for e_def in self.include_enums:
            enum_types.update({e_def.name: e_def})

        # list of elements with one element per field and squeeze value
        self.elements = []
        for line in body:
            parse_line(line, self.elements)

        # list of elements contained in constructor
        self.ctr_elements = [element for element in self.elements if element.is_in_ctr]

        # set each elements static offset
        offset = 0
        for elem in self.elements:
            elem.offset = offset
            offset = offset + elem.byte_size

        # root variables
        self.name = splitext(basename(filename))[0]
        # size of all static elements
        self.static_size = sum(elem.byte_size for elem in self.elements)
        # number of static (fixed size) elements
        self.n_static = sum(1 for elem in self.elements if 'var' not in elem.getter)
        # number of ranges (non-variable array) elements
        self.n_ranges = sum(1 for elem in self.elements if 'array' in elem.setter)
        # number of variable array elements
        self.n_vars = sum(1 for elem in self.elements if 'var' in elem.getter)



