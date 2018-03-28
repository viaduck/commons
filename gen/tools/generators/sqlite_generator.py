
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
Used from sqx template header
"""
import re
from os.path import basename

from common import parse_enum_include


class SQXIO:
    def __init__(self, store, load, sql_setter=None, pre_hook=None, post_hook=None, member_hook=None):
        self._store = store
        self._load = load
        self._sql_setter = self._load if sql_setter is None else sql_setter
        self._pre_hook = pre_hook
        self._post_hook = post_hook
        self._member_hook = member_hook

    def store(self, name, member_name, cpp_type):
        return self._store.format(name=name, member_name=member_name, cpp_type=cpp_type)

    def load(self, name, member_name, cpp_type):
        return self._load.format(name=name, member_name=member_name, cpp_type=cpp_type)

    def setter(self, name, member_name, cpp_type):
        return self._sql_setter.format(name=name, member_name=member_name, cpp_type=cpp_type)

    def pre_hook(self):
        return "" if self._pre_hook is None else self._pre_hook

    def post_hook(self):
        return "" if self._post_hook is None else self._post_hook

    def member_hook(self):
        return "" if self._member_hook is None else self._member_hook

IO_PRIMITIVE = SQXIO('{member_name}', '{member_name} = {name};')
IO_BLOB = SQXIO('sqlite::blob_t({member_name}.const_data(), {member_name}.size())',
                '{member_name}.clear(); {member_name}.write({name}.first, {name}.second, 0);',
                '{member_name}.clear(); {member_name}.write({name}, 0);')
IO_ENUM = SQXIO('toInt({member_name})', '{member_name} = to{cpp_type}({name});', '{member_name} = {name};')
IO_FOREIGN = \
    SQXIO('({member_name}_id >= 0 ? std::make_unique<int64_t>({member_name}_id) : std::unique_ptr<int64_t>())',
          '{member_name}_id = {name} ? *{name} : -1;',
          member_hook='int64_t {member_name}_id = -1;')
REF_BLOB = 'const sqlite::blob_t &'

GETTER_PRIMITIVE = "inline {ref_type} {name}() const {{\n" \
                   "    return {member_name};\n" \
                   "}}\n"
GETTER_FOREIGN = "inline {ref_type} {name}() {{\n" \
                 "    if (!{member_name}) load_{name}();\n" \
                 "    if (!{member_name})\n" \
                 "        throw SQXBase::load_exception(\"{member_name} in {cpp_type} invalid foreign key\");\n\n" \
                 "    return {member_name};\n" \
                 "}}\n" \
                 "inline const {ref_type} {name}() const {{\n" \
                 "    if (!{member_name})\n" \
                 "        throw SQXBase::load_exception(\"{member_name} in {cpp_type} invalid foreign key\");\n\n" \
                 "    return {member_name};\n" \
                 "}}\n" \
                 "inline void load_{name}(bool force = true) {{\n" \
                 "    if ({member_name}_id >= 0 && (force || !{member_name})) {{\n" \
                 "        {member_name}.reset(new {cpp_type}(this, {member_name}_id));\n" \
                 "        {member_name}->load();\n" \
                 "    }}\n" \
                 "}}\n" \
                 "inline int64_t {name}_id() const {{\n" \
                 "    return {member_name}_id;\n" \
                 "}}"

SETTER_REF = "inline {cpp_type} & {name}() {{\n" \
             "    return {member_name};\n" \
             "}}\n"
SETTER_RANGE = "inline void {name}(const BufferRangeConst &rng) {{\n" \
               "    {member_name}.clear(); {member_name}.write(rng, 0);\n" \
               "}}\n"
SETTER_PRIMITIVE = "inline void {name}({ref_type} {name}) {{\n" \
                   "    {setter}\n" \
                   "}}\n"
SETTER_FOREIGN = "inline void create_{name}() {{\n" \
                 "    {member_name}.reset(new {cpp_type}(this));\n" \
                 "}}\n" \
                 "inline void {name}_id(int64_t value) {{\n" \
                 "    {member_name}.reset();" \
                 "    {member_name}_id = value;\n" \
                 "}}\n" \
                 "inline void store_{name}() {{\n" \
                 "    {member_name}->store();\n"   \
                 "    {member_name}_id = {member_name}->id();\n"   \
                 "}}"


class SQXType:
    def __init__(self, io, cpp_type, sql_type, ref_type=None, sql_ref_type=None, member_type=None, additional_getter=GETTER_PRIMITIVE, additional_setter=SETTER_PRIMITIVE):
        self._io = io
        self._cpp_type = cpp_type
        self._sql_type = sql_type
        self._ref_type = cpp_type if ref_type is None else ref_type
        self._sql_ref_type = self._ref_type if sql_ref_type is None else sql_ref_type
        self._member_type = self._cpp_type if member_type is None else member_type
        self._additional_getter = additional_getter
        self._additional_setter = additional_setter

    def io(self):
        return self._io

    def cpp_type(self):
        return self._cpp_type

    def sql_type(self):
        return self._sql_type

    def ref_type(self):
        return self._ref_type

    def member_type(self):
        return self._member_type

    def sql_ref_type(self):
        return self._sql_ref_type

    def additional_setter(self):
        return self._additional_setter

    def additional_getter(self):
        return self._additional_getter

    def format_kwargs(self):
        return {'cpp_type': self.cpp_type(), 'sql_type': self.sql_type(), 'ref_type': self.ref_type(),
                'sql_ref_type': self.sql_ref_type(), 'member_type': self.member_type()}

types = [
    SQXType(SQXIO('static_cast<uint8_t>({member_name})', '{member_name} = static_cast<bool>({name});'), 'bool',
            'INTEGER', sql_ref_type='uint8_t'),
    SQXType(IO_PRIMITIVE, 'uint8_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint16_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint32_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'uint64_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int8_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int16_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int32_t', 'INTEGER'),
    SQXType(IO_PRIMITIVE, 'int64_t', 'INTEGER'),

    SQXType(IO_BLOB, 'String', 'BLOB', 'const String &', REF_BLOB, additional_setter=SETTER_REF+SETTER_PRIMITIVE),
    SQXType(IO_BLOB, 'Buffer', 'BLOB', 'const Buffer &', REF_BLOB, additional_setter=SETTER_REF+SETTER_RANGE),

    SQXType(IO_PRIMITIVE, 'std::string', 'TEXT', 'const std::string &', additional_setter=SETTER_PRIMITIVE+SETTER_REF),
]

hashed_types = {}


def hash_types():
    global hashed_types
    hashed_types = {
        t.cpp_type(): t for t in types
    }

fkey_matcher = re.compile(r"foreign (?P<type>[a-zA-Z0-9_/]+)\s+(?P<name>[a-z_0-9A-Z]+)\s*(?P<constraints>[A-Z ]+)#?.*")
line_matcher = re.compile(r"(?P<type>[a-zA-Z0-9_:]*)\s*(?P<name>[a-z_0-9A-Z]*)\s*#?.*")


class SQXEntry:
    def __init__(self, name, sqx_type, constraints=None, create_schema=None):
        self._name = name
        self._member_name = 'mSQX' + name[0].upper() + name[1:]
        self._type = hashed_types[sqx_type]
        self._constraints = constraints
        self._create_schema = '' if create_schema is None else create_schema

    def name(self):
        return self._name

    def member_name(self):
        return self._member_name

    def type(self):
        return self._type

    def store(self):
        return self._type.io().store(self._name, self._member_name, self._type.cpp_type())

    def load(self):
        return self._type.io().load(self._name, self._member_name, self._type.cpp_type())

    def setter(self):
        return self._type.io().setter(self._name, self._member_name, self._type.cpp_type())

    def pre_hook(self):
        return self._type.io().pre_hook()

    def post_hook(self):
        return self._type.io().post_hook()

    def store_hook(self):
        return self._type.io().store_hook()

    def member_hook(self):
        return self._type.io().member_hook()

    def format_kwargs(self, what_else=None):
        ret = {'name': self.name(), 'member_name': self.member_name(), 'setter': self.setter()}
        ret.update(self.type().format_kwargs())
        if what_else is not None:
            ret.update(what_else)
        return ret

    def create_stmt(self):
        ret = "{name} {sql_type}"
        if self._constraints is not None:
            ret += (" " + self._constraints if len(self._constraints) > 0 else "")
        return ret

    def create_schema(self):
        return self._create_schema


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

        enums = []
        includes_enum = []
        includes_sqx = []
        for line in f:
            if header:
                if line[0] == "-":      # header separator
                    header = False
                else:
                    enums.append(parse_enum_include(line))
                continue
            if line[0] in ('#', '\n'):      # skip empty lines and line comments starting with '#'
                continue

            # add all enums as new SQXTypes
            for enum in enums:
                id, path = enum
                types.append(SQXType(IO_ENUM, id, 'INTEGER', sql_ref_type='std::underlying_type<'+id+'>::type'))
                includes_enum.append(path)

            # is this a foreign key definition?
            f = fkey_matcher.match(line)
            constraints = None
            schema = None
            if f is not None:
                type = f.group('type').strip()
                id = f.group('name').strip()
                constraints = ("REFERENCES {cpp_type} " + f.group('constraints').strip()).strip()
                schema = '{cpp_type}::createTable(db);\n'

                includes_sqx.append(type)

                type = basename(type)
                types.append(SQXType(IO_FOREIGN, type, 'INTEGER', ref_type='std::unique_ptr<'+type+'> &',
                                     sql_ref_type='const std::unique_ptr<int64_t> &', additional_getter=GETTER_FOREIGN,
                                     additional_setter=SETTER_FOREIGN, member_type='std::unique_ptr<'+type+'>'))
            else:
                l = line_matcher.match(line)
                type = l.group('type').strip()
                id = l.group('name').strip()

            # hash all available SQXTypes
            hash_types()

            if len(id.strip()) == 0:
                raise Exception("Parsing error in line: "+line)

            if type not in hashed_types:
                raise Exception("Unsupported type >", type, "<")

            yield SQXEntry(id, type, constraints, schema), includes_enum, includes_sqx
