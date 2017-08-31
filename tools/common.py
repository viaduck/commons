import re
import math
import os
from collections import OrderedDict


enum_include_matcher = re.compile(r"(?P<id>[a-zA-Z0-9_]+) (?P<path>.+)")

# maps variable type to its byte size
i = OrderedDict([
    ("uint64_t", 8),
    ("uint32_t", 4),
    ("uint16_t", 2),
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
        if 0 <= value*8-bits < i[current_type]*8-bits:
            current_type = key
    return current_type


def count_to_bits(cnt):
    return math.ceil(math.log(cnt, 2))
