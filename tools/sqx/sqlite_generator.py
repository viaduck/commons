"""
Used from sqx template header
"""
import os
import re

import common
import math
from enum_generator import do as enum_do


class SQXType:
    def __init__(self, io, cpp_type, sql_type, ref_type=None, sql_ref_type=None, additional_setter=None):
        self._io = io
        self._cpp_type = cpp_type
        self._sql_type = sql_type
        self._ref_type = cpp_type if ref_type is None else ref_type
        self._sql_ref_type = self._ref_type if sql_ref_type is None else sql_ref_type
        self._additional_setter = "" if additional_setter is None else additional_setter

    def io(self):
        return self._io

    def cpp_type(self):
        return self._cpp_type

    def sql_type(self):
        return self._sql_type

    def ref_type(self):
        return self._ref_type

    def sql_ref_type(self):
        return self._sql_ref_type

    def additional_setter(self):
        return self._additional_setter


class SQXIO:
    def __init__(self, store, load, setter=None):
        self._store = store
        self._load = load
        self._setter = self._load if setter is None else setter

    def store(self, name, member_name):
        return self._store.format(name=name, member_name=member_name)

    def load(self, name, member_name):
        return self._load.format(name=name, member_name=member_name)

    def setter(self, name, member_name):
        return self._setter.format(name=name, member_name=member_name)


IO_PRIMITIVE = SQXIO('{member_name}', '{member_name} = {name};')
IO_BLOB = SQXIO('sqlite::blob_t({member_name}.const_data(), {member_name}.size())',
                '{member_name}.clear(); {member_name}.write({name}.first, {name}.second, 0);',
                '{member_name}.clear(); {member_name}.write({name}, 0);')
REF_BLOB = 'const sqlite::blob_t &'
SETTER_REF = "inline {cpp_type} & {name}() {{\n" \
             "    return {member_name};\n" \
             "}}\n"
SETTER_RANGE = "inline void {name}(const BufferRangeConst &rng) {{\n" \
               "    {member_name}.clear(); {member_name}.write(rng, 0);\n" \
               "}}\n"

types = [
    SQXType(SQXIO('static_cast<uint8_t>({member_name})', '{member_name} = static_cast<bool>({name});'), 'bool',
            'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint8_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint16_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint32_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint64_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int8_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int16_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int32_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int64_t', 'INTEGER'),

    SQXType(IO_BLOB, 'String', 'BLOB', 'const String &', REF_BLOB, SETTER_REF),
    SQXType(IO_BLOB, 'Buffer', 'BLOB', 'const Buffer &', REF_BLOB, SETTER_REF+SETTER_RANGE),

    SQXType(IO_PRIMITIVE, 'std::string', 'TEXT', 'const std::string &', additional_setter=SETTER_REF),
]

hashed_types = {
    t.cpp_type(): t for t in types
}

line_matcher = re.compile(r"(?P<type>[a-zA-Z0-9_:]*)\s*(?P<name>[a-z_0-9A-Z]*)\s*#?.*")


class SQXEntry:
    def __init__(self, name, sqx_type):
        self._name = name
        self._member_name = 'm' + name[0].upper() + name[1:]
        self._type = hashed_types[sqx_type]

    def name(self):
        return self._name

    def member_name(self):
        return self._member_name

    def type(self):
        return self._type

    def store(self):
        return self._type.io().store(self._name, self._member_name)

    def load(self):
        return self._type.io().load(self._name, self._member_name)

    def setter(self):
        return self._type.io().setter(self._name, self._member_name)


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

            if type not in hashed_types:
                raise Exception("Unsupported type >", type, "<")

            yield SQXEntry(id, type)
