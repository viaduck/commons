import re
from os.path import basename, splitext
from common import CogBase, DefBase, suggested_type, bits_type, comment_pattern

# matches
# "squeeze_name(bits)"
line_matcher = re.compile(r"(?P<name>[a-zA-Z0-9_]*)\s?\((?P<size>\d+)\)" + comment_pattern)
# matches "import bit/path/to/BitfieldName.btx"
import_matcher = re.compile(r"^import\s(?P<path>bit.+)$")


# a single squeeze element
class BitElem(CogBase):
    def __init__(self, line):
        # match
        m = line_matcher.match(line)

        # extract
        self.name = m.group('name').strip()
        self.size = int(m.group('size').strip())
        self.type = bits_type(self.size)
        self.offset = 0

        # check fields
        if len(self.name) == 0:
            raise Exception("Parsing error in line: " + line)


# all enum definitions
class BitDef(DefBase, CogBase):
    def __init__(self, filename):
        DefBase.__init__(self, filename)

        # create elements
        self.parse()

        # count bits and assign offsets
        offset = 0
        for elem in self.elements:
            elem.offset = offset
            offset = offset + elem.size

        # name bitfield after basename of the file
        self.name = splitext(basename(filename))[0]
        # use smallest type that can fit all bits
        self.type = bits_type(offset)
        # include path for bitfield imports
        self.import_path = splitext(filename)[0] + ".h"

    def parse_line(self, line):
        # enum definitions only have enum lines
        return [BitElem(line)]


def bit_import(line):
    # match import
    m = import_matcher.match(line)
    if m is None:
        return None

    # extract path
    path = m.group('path').strip()

    # return BitDef from path
    return BitDef(path)
