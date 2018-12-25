
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
from os.path import basename, splitext
from common import CogBase, DefBase, suggested_type, bits_type, comment_pattern, type_bits

# matches
# "squeeze_name(bits)"
line_matcher = re.compile(r"(?P<name>[a-zA-Z0-9_]*)\s?\((?P<size>\d+)\)" + comment_pattern)
# matches "type <type>"
type_matcher = re.compile(r"type\s+(?P<type>[a-z0-9_]*)" + comment_pattern)
# matches "import bit/path/to/BitfieldName.btx"
import_matcher = re.compile(r"^import\s(?P<path>bit.+)$")


# a single squeeze element
class BitElem(CogBase):
    def __init__(self, name, size):
        self.name = name
        self.size = size
        self.type = bits_type(self.size)
        self.offset = 0


# all enum definitions
class BitDef(DefBase, CogBase):
    def __init__(self, filename):
        DefBase.__init__(self, filename)
        self.type = None

        # create elements
        self.parse()

        # count bits and assign offsets
        offset = 0
        for elem in self.elements:
            elem.offset = offset
            offset = offset + elem.size

        # name bitfield after basename of the file
        self.name = splitext(basename(filename))[0]
        # smallest type that can fit all bits
        self.s_type = bits_type(offset)
        # use suggested if none given
        self.type = self.s_type if self.type is None else self.type
        # ensure given type is not smaller than suggested
        if type_bits(self.type) < type_bits(self.s_type):
            raise Exception("Given type too small " + self.type + ", required " + self.s_type)
        # include path for bitfield imports
        self.import_path = splitext(filename)[0] + ".h"

    def parse_line(self, line):
        line_match = line_matcher.match(line)
        type_match = type_matcher.match(line)

        if type_match is not None:
            self.type = type_match.group('type').strip()
            return []
        elif line_match is not None:
            # extract
            name = line_match.group('name').strip()
            size = int(line_match.group('size').strip())

            # check required fields
            if len(name) == 0:
                raise Exception("Parsing error in line: " + line)

            return [BitElem(name, size)]

        raise Exception("parse error on line: " + line)


def bit_import(line):
    # match import
    m = import_matcher.match(line)
    if m is None:
        return None

    # extract path
    path = m.group('path').strip()

    # return BitDef from path
    return BitDef(path)
