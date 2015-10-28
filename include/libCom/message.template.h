[[[cog
import cog
import generator as g
from os.path import basename, splitext

name = splitext(basename(filename[:-4]))[0]
filename = base_path+filename
]]]
[[[end]]]
[[[cog
cog.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
]]]
[[[end]]]

[[[cog
cog.outl("#ifndef {name}_H".format(name=name))
cog.outl("#define {name}_H".format(name=name))
]]]
[[[end]]]


#include "libCom/Buffer.h"
#include "libCom/BufferRange.h"
#include "libCom/conversions.h"
#include <cstring>


[[[cog
    cog.outl("class "+name+" {")
]]]
[[[end]]]
public:
    [[[cog
        cog.outl(name+"() : mBuffer(*(new Buffer(SIZE))), mAllocated(true) {\n"
                 "     mBuffer.padd(SIZE, 0);\n"
                 " }")
    ]]]
    [[[end]]]

    [[[cog
        cog.outl(name+"(Buffer &buffer) : mBuffer(buffer) {")
    ]]]
    [[[end]]]
        if (mBuffer.size() < SIZE) {
            mBuffer.increase(SIZE);     // prevent access resulting in SIGSEGV if buffer is too small
            mBuffer.use(SIZE);
        }
    }

    // copy constructor
    [[[cog
        cog.outl(name+"("+name+" &other) : mBuffer(*(new Buffer(other.mBuffer))), mAllocated(true) { }")
    ]]]
    [[[end]]]

    [[[cog
    vars = list(g.do(filename))

    cog.out(name+"(Buffer &buffer")
    for v in vars:
        if v[3] is not None:
            cog.out(", const {type} *_{name}".format(type=v[0], name=v[1]))
        else:
            cog.out(", {type} _{name}".format(type=v[0], name=v[1]))
    cog.out(") : {name}(buffer)".format(name=name)+" {\n")
    offset = 0
    for v in vars:
        if v[3] is not None:
            cog.outl("    memcpy(mBuffer.data({offset}), _{name}, {size});".format(offset=offset, type=v[0], name=v[1], size=v[2]*v[3]))
            offset += v[2]*v[3]     # sizeof type * array element count
        else:
            cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
            offset += v[2]
    ]]]
    [[[end]]]
    }


    // destructor
    [[[cog
    cog.outl("~"+name+"() {")
    ]]]
    [[[end]]]
        if (mAllocated)
            delete &mBuffer;
    }


    // GETTER & SETTER //
    [[[cog
    vars = list(g.do(filename))

    cog.out(name+"(")
    first = True
    for v in vars:
        if v[3] is not None:
            cog.out((", " if not first else "")+"const {type} *_{name}".format(type=v[0], name=v[1]))
        else:
            cog.out((", " if not first else "")+"{type} _{name}".format(type=v[0], name=v[1]))
        first = False
    cog.out(") : {name}()".format(name=name)+" {\n")
    offset = 0
    for v in vars:
        if v[3] is not None:
            cog.outl("    memcpy(mBuffer.data({offset}), _{name}, {size});".format(offset=offset, type=v[0], name=v[1], size=v[2]*v[3]))
            offset += v[2]*v[3]     # sizeof type * array element count
        else:
            cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
            offset += v[2]
    ]]]
    [[[end]]]
    }

    const Buffer &const_buffer() const {
        return mBuffer;
    }

    Buffer &buffer() {
        return mBuffer;
    }

    // ++++++++ ///

    [[[cog

    offset = 0
    for v in vars:
        cog.outl("// - "+v[1]+" - //")
        # pointer types
        if v[3] is not None:
            cog.outl("inline const {type} {name}_const() const {{\n"
                     "    return static_cast<const {type}>(mBuffer.const_data({offset}));\n"
                     "}}\n"
                     "inline {type} {name}() {{\n"
                     "    return static_cast<{type}>(mBuffer.data({offset}));\n"
                     "}}\n"
                     "inline bool {name}(const {type} v, const uint32_t size) {{\n"
                     "    if (size != {size}) return false;\n"
                     "    memcpy(mBuffer.data({offset}), v, {size});\n"
                     "    return true;\n"
                     "}}\n"
                     "const BufferRange {name}_range() {{\n"
                     "    return BufferRange(mBuffer, {name}_size(), {offset});\n"
                     "}}\n"
                     "static inline const uint32_t {name}_size() {{\n"
                     "    return sizeof({type_raw})*{count};\n"
                     "}}\n".format(type=v[0]+"*", type_raw=v[0], name=v[1], offset=offset, count=v[3], size=v[2]*v[3]))
            offset += v[2]*v[3]     # sizeof type * array element count
        else:
        # non-pointer types
            cog.outl("inline const {type} {name}() const {{\n"
                     "    return {conversion}(*static_cast<const {type}*>(mBuffer.const_data({offset})));\n"
                     "}}\n"
                     "inline void {name}({type} v) {{\n"
                     "    *static_cast<{type}*>(mBuffer.data({offset})) = {conversion}(v);\n"
                     "}}\n"
                     "static inline const uint32_t {name}_size() {{\n"
                     "    return sizeof({type});\n"
                     "}}\n".format(type=v[0], name=v[1], offset=offset, conversion="hton_"+v[0]))
            offset += v[2]
    ]]]
    [[[end]]]

    // ++++++++ ///

    [[[cog
    cog.outl("const static uint32_t SIZE = {size};".format(size=offset))
    ]]]
    [[[end]]]

private:
    Buffer &mBuffer;
    const bool mAllocated = false;
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
