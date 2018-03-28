
# Copyright (C) 2015-2018 The ViaDuck Project
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
import math
import os
from collections import OrderedDict


enum_include_matcher = re.compile(r"(?P<id>[a-zA-Z0-9_]+) (?P<path>.+)")

# maps variable type to its byte size
i = OrderedDict([
    ("int64_t", 8),
    ("uint64_t", 8),
    ("int32_t", 4),
    ("uint32_t", 4),
    ("int16_t", 2),
    ("uint16_t", 2),
    ("int8_t", 1),
    ("uint8_t", 1),
])


def parse_enum_include(line):
    # parse an enum include
    l = enum_include_matcher.match(line)
    if l is None:
        raise Exception("Parse error:", line)
    try:
        id = l.group("id")
        path = l.group("path")
        print("[*] Enum include: ", id, path)
    except IndexError:
        raise Exception("Parse error:", line)

    if not os.path.exists(path):
        raise Exception("Enum file not found: "+path)

    return id, path


def type_to_bits(type):
    return i[type]*8


def bits_to_type(bits):
    """
    Returns nearest type that can hold atleast the number of bits.
    """
    current_type = list(i.keys())[0]
    for key, value in i.items():
        if 0 <= value*8-bits <= i[current_type]*8-bits:
            current_type = key
    return current_type


def count_to_bits(cnt):
    return math.ceil(math.log(cnt, 2))
