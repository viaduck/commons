
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

"""
Used from enum template header
"""

import re
from itertools import chain

line_matcher = re.compile(r"(?P<value>[a-zA-Z0-9_]*)\s*,?(?P<comment>[^#]+)?\s*(?:#.*)?")


def do(filename, skip_doxygen=False):
    """
    Processes the enum definition file
    :param filename: enum definition file path
    :param skip_doxygen: Whether to skip searching for the doxygen header block
    """
    with open(filename, "r") as f:
        doxygen_section = not skip_doxygen
        doxygen = ""
        for line in f:
            if doxygen_section:
                if line[0] == "-":      # doxygen separator
                    doxygen_section = False
                    yield (doxygen,)
                else:
                    doxygen += line
                continue
            if line[0] in ('#', '\n'):      # skip empty lines and line comments starting with '#'
                continue

            l = line_matcher.match(line)
            value = l.group('value')
            comment = l.group('comment')

            if value is not None:
                value = value.strip()
            if comment is not None:
                comment = comment.strip()
            else:
                comment = ""

            if value is None or len(value) == 0:
                raise Exception("Parsing error in line: "+line)

            yield (value, comment)
        # there is no doxygen section at all, repeat
        if doxygen_section:
            yield ("",)
            for i in do(filename, True):
                yield i
        else:
            yield ("INVALID_ENUM_VALUE", "// invalid enum values are mapped to this")
