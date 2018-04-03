import math
import cog
from collections import OrderedDict

# maps c++ types to its bit size
types = OrderedDict([
    ("int64_t", 64),
    ("uint64_t", 64),
    ("int32_t", 32),
    ("uint32_t", 32),
    ("int16_t", 16),
    ("uint16_t", 16),
    ("int8_t", 8),
    ("uint8_t", 8),
])


class CogBase:
    _first_comma = True

    def out(self, s, **kwargs):
        cog.out(s.format(**dict(vars(self), **kwargs)))

    def lout(self, s, **kwargs):
        # list out - output comma
        cog.out("" if CogBase._first_comma else ", ")
        CogBase._first_comma = False
        self.out(s, **kwargs)

    @staticmethod
    def reset_list():
        CogBase._first_comma = True

    def outl(self, s, **kwargs):
        cog.outl(s.format(**dict(vars(self), **kwargs)))


def read_definition(filename):
    # pre and post dash
    initial = []
    body = []

    with open(filename, "r") as f:
        for line in f:
            # comment and empty
            if line[0] in ('#', '\n'):
                continue

            # split initial (optional)
            elif line[0] == "-":
                initial, body = body, []

            # general line
            else:
                body.append(line)

    return initial, body


def suggested_type(count):
    # bits required to represent count elements
    bits = math.ceil(math.log(count, 2))
    return bits_type(bits)


def bits_type(bits):
    # default to largest type
    s_type, s_bits = list(types.items())[0]

    # try to find smallest type having bits
    for type_name, bit_size in types.items():
        if bits <= bit_size <= s_bits:
            s_type, s_bits = type_name, bit_size

    # smallest type fitting bits
    return s_type


def type_bits(type_name):
    return 0 if type_name not in types else types[type_name]
