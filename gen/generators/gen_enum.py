
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

import re
from os.path import basename, splitext
from common import CogBase, DefBase, read_definition, suggested_type, comment_pattern, type_bits

# matches
# "enum_name"
# "enum_name, comment"
line_matcher = re.compile(r"(?P<name>[a-zA-Z0-9_]*)\s*,?(?P<comment>[^#]+)?" + comment_pattern)
# matches "type <type> [flags]"
type_matcher = re.compile(r"type\s+(?P<type>[a-z0-9_]+)\s*(?P<flags>flags)?" + comment_pattern)
# matches "import enum/path/to/EnumName.the"
import_matcher = re.compile(r"^import\s(?P<path>enum.+)$")


# a single enum element
class EnumElem(CogBase):
    def __init__(self, name, comment, value=""):
        self.name = name
        self.comment = comment
        self.value = value

        # strip optional field
        self.comment = "" if self.comment is None else self.comment.strip()


# all enum definitions
class EnumDef(DefBase, CogBase):
    def __init__(self, base_dir, filename):
        DefBase.__init__(self, base_dir, filename)
        self.type = None
        self.flags = False
        self.flag_val = 1

        # create elements
        self.parse()
        # handle ALL flags element
        if self.flags:
            self.elements.insert(0, EnumElem("FLAGS_NONE", "/**< enum value for none of the flags */", " = 0"))
            self.elements.append(EnumElem("FLAGS_ALL", "/**< enum value for all flags */", f" = {self.flag_val*2-1}"))
        else:
            # add element for invalid enum value
            self.elements.append(EnumElem("VALUE_INVALID", "/**< invalid enum values are mapped to this */"))

        # name enum after basename of the file
        self.name = splitext(basename(filename))[0]
        # smallest type that can fit all elements
        self.s_type = suggested_type(len(self.elements))
        # use suggested if none given
        self.type = self.s_type if self.type is None else self.type
        # ensure given type is not smaller than suggested
        if type_bits(self.type) < type_bits(self.s_type):
            raise Exception("Given type too small " + self.type + ", required " + self.s_type)
        # include path for enum imports
        self.import_path = splitext(filename)[0] + ".h"

        self.max_val = len(self.elements) - 1
        self.elem_invalid_val = self.elements[0 if self.flags else -1].name

    def next_flag(self):
        v = ""
        if self.flags:
            v = f" = {self.flag_val}"
            self.flag_val *= 2
        return v

    def parse_line(self, line):
        line_match = line_matcher.match(line)
        type_match = type_matcher.match(line)

        if type_match is not None:
            self.type = type_match.group('type').strip()
            self.flags = type_match.group('flags') is not None
            return []
        elif line_match is not None:
            name = line_match.group('name').strip()
            comment = line_match.group('comment')

            # check required fields, strip optional fields
            if len(name) == 0:
                raise Exception("Parsing error in line: " + line)

            comment = "" if comment is None else comment.strip()
            return [EnumElem(name, comment, self.next_flag())]

        raise Exception("parse error on line: " + line)


def enum_import(base_dir, line):
    # match import
    m = import_matcher.match(line)
    if m is None:
        return None

    # extract path
    path = m.group('path').strip()

    # return EnumDef from path
    return EnumDef(base_dir, path)
