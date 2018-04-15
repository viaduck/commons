
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
from enum import Enum
from os.path import basename, splitext
from common import CogBase, DefBase, read_definition, type_bits, bits_type, comment_pattern

from generators.gen_enum import enum_import
from generators.gen_bit import bit_import

# matches integral "type name"
integral_matcher = re.compile(r"(?P<type>\w*)\s+(?P<name>\w*)" + comment_pattern)
# matches "type[123] name" integral array
array_matcher = re.compile(r"(?P<type>[\w]*)\[(?P<size>\d+)\]\s+(?P<name>\w*)" + comment_pattern)
# matches "var|Var|VAR name" variable array
var_array_matcher = re.compile(r"(?P<type>[\w]*)\[(?P<size>[a-zA-Z]+)\]\s+(?P<name>\w*)" + comment_pattern)

# types of variable array size indicators
var_types = {"var": "uint8_t", "Var": "uint16_t", "VAR": "uint32_t"}
# types of enums
enum_types = {}
# types of bitfields
bit_types = {}


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


class ProtoBitType(ProtoIntegralType):
    def __init__(self, elem_type, elem_name, b_def):
        super().__init__(b_def.type, elem_name)

        # type of bitfield class
        self.super_type = elem_type

        # load value from argument
        self.ctr_load = "{name}().value(_{name});"
        self.ctr_range_load = self.ctr_load

        # getter is modifier, no setter needed
        self.getter = ['bitfield']
        self.setter = []


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


# all protocol definitions
class ProtoDef(DefBase, CogBase):
    def __init__(self, filename):
        DefBase.__init__(self, filename)

        # create elements
        self.parse()
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

        match = integral_matcher.match(line)
        if match is not None:
            elem_type = match.group('type').strip()
            elem_name = match.group('name').strip()

            # enum, bitfield or integral
            if elem_type in enum_types:
                return [ProtoEnumType(elem_type, elem_name, enum_types[elem_type])]
            elif elem_type in bit_types:
                return [ProtoBitType(elem_type, elem_name, bit_types[elem_type])]
            else:
                return [ProtoIntegralType(elem_type, elem_name)]

        match = array_matcher.match(line)
        if match is not None:
            elem_type = match.group('type').strip()
            elem_name = match.group('name').strip()
            elem_count = match.group('size').strip()

            # integral array
            return [ProtoIntegralArrayType(elem_type, elem_name, elem_count)]

        match = var_array_matcher.match(line)
        if match is not None:
            elem_type = match.group('type').strip()
            elem_name = match.group('name').strip()
            elem_size = match.group('size').strip()

            # variable array
            return [ProtoVariableArrayType(elem_type, elem_name, var_types[elem_size])]

        raise Exception("parse error on line: " + line)


