[[[cog
import cog
import generator as g
name = filename[:-4]
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
#include "libCom/conversions.h"
#include <cstring>


[[[cog
    cog.outl("class "+name+" {")
]]]
[[[end]]]
public:
    [[[cog
        cog.outl(name+"(const Buffer &buffer) : mBuffer(buffer) {")
    ]]]
    [[[end]]]
        if (mBuffer.size() < SIZE)
            mBuffer.increase(SIZE);     // prevent access resulting in SIGSEGV if buffer is too small
    }


    [[[cog
    vars = list(g.do(filename))

    cog.out(name+"(const Buffer &buffer")
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
                     "const BufferRange {name}_range() const {{\n"
                     "    return BufferRange(mBuffer, sizeof({type_raw})*{count}, {offset});\n"
                     "}}\n".format(type=v[0]+"*", type_raw=v[0], name=v[1], offset=offset, count=v[3]))
            offset += v[2]*v[3]     # sizeof type * array element count
        else:
        # non-pointer types
            cog.outl("inline const {type} {name}() const {{\n"
                     "    return {conversion}(*static_cast<const {type}*>(mBuffer.const_data({offset})));\n"
                     "}}\n"
                     "inline void {name}({type} v) {{\n"
                     "    *static_cast<{type}*>(mBuffer.data({offset})) = {conversion}(v);\n"
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
    Buffer mBuffer;
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
