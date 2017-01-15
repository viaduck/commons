from collections import OrderedDict

# maps variable type to its  byte size
import math

i = OrderedDict([
    ("uint64_t", 8),
    ("uint32_t", 4),
    ("uint16_t", 2),
    ("uint8_t", 1),
])


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
