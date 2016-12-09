"""
Used from enum template header
"""

import re

line_matcher = re.compile(r"(?P<value>[a-zA-Z0-9_]*)\s*,?(?P<comment>[^#]+)?\s*(?:#.*)?")


def do(filename):
    """
    Processes the enum definition file
    :param filename: enum definition file path
    """
    with open(filename, "r") as f:
        for line in f:
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

        yield ("INVALID_ENUM_VALUE", "// invalid enum values are mapped to this")
