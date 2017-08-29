"""
Used from sqx template header
"""
import os
import re

import common
import math
from enum_generator import do as enum_do

type_map = {
    'uint8_t': 'INTEGER',
    'uint16_t': 'INTEGER',
    'uint32_t': 'INTEGER',
    'uint64_t': 'INTEGER',
    'int8_t': 'INTEGER',
    'int16_t': 'INTEGER',
    'int32_t': 'INTEGER',
    'int64_t': 'INTEGER',

    'String': 'BLOB',
    'Buffer': 'BLOB',

    'std::string': 'TEXT',
}
type_complex = [
    'String', 'Buffer'
]

line_matcher = re.compile(r"(?P<type>[a-zA-Z0-9_:]*)\s*(?P<name>[a-z_0-9A-Z]*)\s*#?.*")


class SQXEntry:
    def __init__(self, name, var_type, sql_type):
        self._name = name
        self._member_name = name[0].upper() + name[1:]
        self._type = var_type
        self._sql_type = sql_type

    def name(self):
        return self._name

    def member_name(self):
        return self._member_name

    def var_type(self):
        return self._type

    def sql_type(self):
        return self._sql_type

    def is_complex(self):
        return self._type in type_complex

    def is_primitive(self):
        return not self.is_complex()


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
                    # TODO parse includes
                    pass
                continue
            if line[0] in ('#', '\n'):      # skip empty lines and line comments starting with '#'
                continue

            l = line_matcher.match(line)
            type = l.group('type').strip()
            id = l.group('name').strip()

            if len(id.strip()) == 0:
                raise Exception("Parsing error in line: "+line)

            if type not in type_map:
                raise Exception("Unsupported type >", type, "<")

            yield SQXEntry(id, type, type_map[type])
