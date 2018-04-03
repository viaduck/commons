import re
from enum import Enum
from os.path import basename, splitext
from common import CogBase, read_definition

from generators.gen_enum import enum_import

# matches "foreign Type name ON CASCADE SET NULL"
fk_matcher = re.compile(r"foreign (?P<type>[a-zA-Z0-9_/]+)\s+(?P<name>[a-z_0-9A-Z]+)\s*(?P<constraints>[A-Z ]+)#?.*")
# matches "Type name"
line_matcher = re.compile(r"(?P<type>[a-zA-Z0-9_:]*)\s*(?P<name>[a-z_0-9A-Z]*)\s*#?.*")


class SQXValueType(CogBase):
    def __init__(self, cpp_t, sql_t=None, sql_ref_t=None):
        self.cpp_t = cpp_t
        # pass integral types by value
        self.cpp_ref_t = cpp_t
        self.cpp_const_ref_t = cpp_t
        # how the type is stored in SQL
        self.sql_t = "INTEGER" if sql_t is None else sql_t
        # type used by sqlite
        self.sql_ref_t = self.cpp_ref_t if sql_ref_t is None else sql_ref_t

        # when loading from sqlite, copy by value
        self.load = '{member_name} = {name};'
        # loading from setter argument
        self.load_setter = self.load
        # when storing into sqlite, store by value
        self.store = '{member_name}'

        # basic member
        self.member = '{type.cpp_t} {member_name};'
        # basic getter
        self.getter = ['basic']
        # basic setter
        self.setter = ['basic']


class SQXReferenceType(SQXValueType):
    def __init__(self, cpp_t, sql_t, sql_ref_t=None):
        super().__init__(cpp_t, sql_t, sql_ref_t)
        # pass reference types by reference
        self.cpp_ref_t = cpp_t + " &"
        self.cpp_const_ref_t = "const " + cpp_t + " &"
        # type used by sqlite
        self.sql_ref_t = self.cpp_const_ref_t if sql_ref_t is None else sql_ref_t

        # add reference setter
        self.setter.append('ref')


class SQXBoolType(SQXValueType):
    def __init__(self):
        super().__init__("bool", "INTEGER", "uint8_t")

        # when loading bool, convert from uint8_t to bool
        self.load = '{member_name} = static_cast<bool>({name});'
        # when storing bool, convert from bool to uint8_t
        self.store = 'static_cast<uint8_t>({member_name})'


class SQXBlobType(SQXReferenceType):
    def __init__(self, cpp_t):
        super().__init__(cpp_t, "BLOB", "const sqlite::blob_t &")

        # when loading from sqlite, convert sqlite::blob (pair of ptr and size) to underlying type
        self.load = '{member_name}.clear(); {member_name}.write({name}.first, {name}.second, 0);'
        # when loading from setter argument, no need to convert
        self.load_setter = '{member_name}.clear(); {member_name}.write({name}, 0);'
        # when storing to sqlite, convert underlying type to sqlite::blob
        self.store = 'sqlite::blob_t({member_name}.const_data(), {member_name}.size())'

        # add range setter
        self.setter.append('range')


class SQXForeignType(SQXReferenceType):
    def __init__(self, cpp_t):
        super().__init__(cpp_t, "INTEGER", "const std::unique_ptr<int64_t> &")

        # when loading from sqlite, convert "null" values to -1
        self.load = '{member_name}_id = {name} ? *{name} : -1;'
        # when storing to sqlite, convert -1 to "null"
        self.store = '({member_name}_id >=0 ? std::make_unqiue<int64_t>({member_name}_id) : std::unqiue_ptr<int64_t>())'

        # one member for foreign id, one for the type (using unique_ptr to allow null)
        self.member = 'int64_t {member_name}_id = -1;\n' \
                      'std::unique_ptr<{type.cpp_t}> {member_name};'

        # replace basic getters and setters with foreign setters
        self.getter = ['foreign']
        self.setter = ['foreign']


class SQXEnumType(SQXValueType):
    def __init__(self, cpp_t, enum_t):
        super().__init__(cpp_t, "INTEGER", enum_t)

        # when loading from sqlite, convert from enum_t to cpp_t
        self.load = '{member_name} = to{type.cpp_t}({name});'
        # when storing to sqlite, convert from cpp_t to enum_t
        self.store = 'toInt({member_name})'


SQLiteTypes = {
    'bool': SQXBoolType(),
    'uint8_t': SQXValueType('uint8_t'),
    'uint16_t': SQXValueType('uint16_t'),
    'uint32_t': SQXValueType('uint32_t'),
    'uint64_t': SQXValueType('uint64_t'),
    'int8_t': SQXValueType('int8_t'),
    'int16_t': SQXValueType('int16_t'),
    'int32_t': SQXValueType('int32_t'),
    'int64_t': SQXValueType('int64_t'),
    'String': SQXBlobType('String'),
    'Buffer': SQXBlobType('Buffer'),
    'std::string': SQXReferenceType('std::string', 'TEXT')
}


# various types of protocol elements
class ElemType(Enum):
    Column = 1,
    Foreign = 2,


# a single sqx element
class SQXElem(CogBase):
    def __init__(self, line):
        # foreign key vs table column
        m = fk_matcher.match(line)
        if m is not None:
            self.sqx_type = ElemType.Foreign
            self.path = m.group('type').strip()
            self.constraints = m.group('constraints').strip()

            self.name = m.group('name').strip()
            self.type = SQXForeignType(basename(self.path))

        else:
            m = line_matcher.match(line)
            self.sqx_type = ElemType.Column

            self.name = m.group('name').strip()
            self.type = SQLiteTypes[m.group('type').strip()]

        # mSQXFoo
        self.member_name = 'mSQX' + self.name[0].upper() + self.name[1:]


# all sqx definitions
class SQXDef(CogBase):
    def __init__(self, filename):
        # split input
        includes, body = read_definition(filename)

        # collect enum includes
        self.include_enums = [enum_import(i) for i in includes]
        # add each enum as a custom type
        for e_def in self.include_enums:
            SQLiteTypes.update({e_def.name: SQXEnumType(e_def.name, e_def.type)})

        # root variables
        self.elements = [SQXElem(b) for b in body]
        self.name = splitext(basename(filename))[0]