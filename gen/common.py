
# Copyright (C) 2018-2023 The ViaDuck Project
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

import math
import os
import re

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
    ("bool", 1),
])

# matches in-line comment
comment_pattern = r"\s*(?:#.*)?$"
# matches "escaped" curly braces \{ or \}
matcher_escaped_braces = re.compile(r"\\([{}])")
# matches "unescaped" curly braces { or }
matcher_unescaped_braces = re.compile(r"(?:^|[^\\])([{}])")


class SafeDict(dict):
    def __missing__(self, key):
        return '{' + key + '}'


class CogBase:
    _first_element = True

    def _format_with_vars(self, s, **kwargs):
        while matcher_unescaped_braces.search(s) is not None:
            # protect "escaped" braces by doubling them before string format
            sf = matcher_escaped_braces.sub(r"\\\1\1", s)
            s = sf.format(**SafeDict(vars(self), **kwargs))

        # remove escape sequences before using the string
        return matcher_escaped_braces.sub(r"\1", s)

    def _try_print_with_vars(self, s, pcb, **kwargs):
        try:
            pcb(self._format_with_vars(s, **kwargs))
        except Exception as e:
            print(f"Formatting error: '{s}'", e)
            raise e

    def out(self, s, **kwargs):
        self._try_print_with_vars(s, cog.out, **kwargs)

    def outl(self, s, **kwargs):
        self._try_print_with_vars(s, cog.outl, **kwargs)

    def lout(self, s, sep=", ", **kwargs):
        # list out - output comma
        cog.out("" if CogBase._first_element else sep)
        CogBase._first_element = False
        self.out(s, **kwargs)

    @staticmethod
    def is_list_first():
        return CogBase._first_element

    @staticmethod
    def reset_list():
        CogBase._first_element = True


class DefBase:
    def __init__(self, base_dir, filename):
        # split input
        self.initial, self.body = read_definition(os.path.join(base_dir, filename))
        # reassemble doxygen comment
        self.doxygen = "".join(self.initial)
        # placeholder
        self.elements = []
        self.includes = []
        # save base dir of definition files
        self.base_dir = base_dir
        # save filename
        self.filename = filename

    def parse(self):
        # sub elements, allow parse_line to add multiple elements per parsed line
        self.elements = sum((self.parse_line(line) for line in self.body), [])

    def parse_line(self, line):
        return []


def read_definition(filename):
    # pre- and post-dash
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

    # try to find the smallest type having bits
    for type_name, bit_size in types.items():
        if bits <= bit_size <= s_bits:
            s_type, s_bits = type_name, bit_size

    # smallest type fitting bits
    return s_type


def type_bits(type_name):
    return 0 if type_name not in types else types[type_name]
