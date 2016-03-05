"""
Used from protcol message template header
"""

import re

# maps variable type to its  byte size
i = {
    "uint64_t": 8,
    "uint32_t": 4,
    "uint16_t": 2,
    "uint8_t": 1
}

# tuple of variable types that have array support
arr_supported = ('uint8_t',)

def do(filename):
    """
    Processes the protocol message definition file
    :param filename: protocol message definition file path
    """
    with open(filename, "r") as f:
        for line in f:
            if line[0] in ('#', '\n'):      # skip empty lines and line comments starting with '#'
                continue

            l = line.split()
            type, id = l[0], l[1]

            arr_i = re.split("\(([0-9]+|var)\)", type)
            if len(arr_i) > 1:
                if arr_i[1] != "var":
                    arr = int(arr_i[1])     # array element count
                else:
                    arr = "var"
            else:
                arr = None
            type = arr_i[0]

            # is defined as array, but not supported
            if arr is not None and type not in arr_supported:
                raise Exception(type+" is not supported as array type!")
            yield (type, id, i[type], arr)
