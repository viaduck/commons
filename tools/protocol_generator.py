"""
Used from protcol message template header
"""
import os
import re

import common
import math
from enum_generator import do as enum_do

# tuple of variable types that have array support
arr_supported = ('uint8_t',)
variable_arrays = ("var", "Var", "VAR")
variable_arrays_type = {"var": "uint8_t", "Var": "uint16_t", "VAR": "uint32_t"}
line_matcher = re.compile(r"(?P<type>[a-zA-Z0-9_]*)(?:\((?P<arr_size>(?:\d*)|(?:var)|(?:Var)|(?:VAR))\))?\s*(?P<name>[(), a-z_0-9A-Z]*)\s*#?.*")
squeeze_matcher = re.compile("(?P<name>\w*)\((?P<bits>\d*)\),?\s*?")
enum_include_matcher = re.compile(r"(?P<id>[a-zA-Z0-9_]+) (?P<path>.+)")
enum_types = {}     # list of enum types that are included in file head


def size_bits(type):
    # not a primitive type
    if type not in common.i:
        # must be an enum value
        if type not in enum_types:
            raise Exception("No include specified for unknown type "+type)
        return common.type_to_bits(enum_types[type]['underlying'])
    else:
        return common.type_to_bits(type)


def enum_type(enum):
    return enum_types[enum]['underlying']


def do(filename):
    """
    Processes the protocol message definition file
    :param filename: protocol message definition file path
    """
    print("Processing", filename)
    with open(filename, "r") as f:
        header = False
        # first look if there is any header separator at all
        for line in f:
            if line[0] == "-":
                header = True
        f.seek(0)

        if not header:
            raise Exception("No header present")
        for line in f:
            if header:
                if line[0] == "-":      # header separator
                    header = False
                else:
                    # parse an enum include
                    l = enum_include_matcher.match(line)
                    if l is None:
                        continue
                    try:
                        id = l.group("id")
                        path = l.group("path")
                        print("[*] Enum include: ", id, path)
                    except IndexError:
                        continue                # invalid format

                    if not os.path.exists(path):
                        raise Exception("Enum file not found: "+path)

                    # get all possible enum values to calculate number of required bits
                    vals = list(enum_do(path))[1:]         # first entry is doxygen section
                    bits = common.count_to_bits(len(vals))

                    enum_types[id] = {'bits': bits, 'underlying': common.bits_to_type(bits), 'path': path}
                continue
            if line[0] in ('#', '\n'):      # skip empty lines and line comments starting with '#'
                continue

            l = line_matcher.match(line)
            type = l.group('type').strip()
            id = l.group('name').strip()
            arr_size = l.group('arr_size')

            if len(id.strip()) == 0:
                raise Exception("Parsing error in line: "+line)

            ids = squeeze_matcher.finditer(id)
            # bit field support
            subs = []
            bit_sum = 0
            for sub_id in ids:
                subs.append({"name": sub_id.group("name"), "bits": int(sub_id.group("bits"))})
                bit_sum += subs[-1]["bits"]

            # any sub value
            if len(subs) > 0:
                id = "_".join(sub["name"] for sub in subs)

            if bit_sum > size_bits(type):
                raise Exception("Subvalues bit sum exceed type: "+id)

            # array requested
            if arr_size is not None:
                if arr_size not in ("var", "Var", "VAR"):
                    arr = int(arr_size)
                else:
                    arr = arr_size
            else:
                arr = None

            # is defined as array, but not supported
            if arr is not None and type not in arr_supported:
                raise Exception(type+" is not supported as array type!")

            byte_size = size_bits(type) / 8
            yield (type, id, byte_size, arr, subs if len(subs) > 0 else None)
