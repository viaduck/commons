"""
Used from protcol message template header
"""

import re

import common

# tuple of variable types that have array support
arr_supported = ('uint8_t',)
variable_arrays = ("var", "Var", "VAR")
variable_arrays_type = {"var": "uint8_t", "Var": "uint16_t", "VAR": "uint32_t"}
line_matcher = re.compile(r"(?P<type>[a-zA-Z0-9_]*)(?:\((?P<arr_size>(?:\d*)|(?:var)|(?:Var)|(?:VAR))\))?\s*(?P<name>[(), a-z_0-9A-Z]*)\s*#?.*")
squeeze_matcher = re.compile("(?P<name>\w*)\((?P<bits>\d*)\),?\s*?")


def do(filename):
    """
    Processes the protocol message definition file
    :param filename: protocol message definition file path
    """
    with open(filename, "r") as f:
        for line in f:
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
                #print ("--> "+sub_id.group("name")+" with "+sub_id.group("bits"))
                subs.append({"name": sub_id.group("name"), "bits": int(sub_id.group("bits"))})
                bit_sum += subs[-1]["bits"]

            # any sub value
            if len(subs) > 0:
                id = "_".join(sub["name"] for sub in subs)

            if bit_sum > common.type_to_bits(type):
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
            yield (type, id, common.i[type], arr, subs if len(subs) > 0 else None)
