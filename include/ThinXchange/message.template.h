[[[cog
import cog
import generator as g
name = filename[:-4]
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


#include <Buffer.h>
#include <ThinXchange/conversions.h>


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

    const Buffer &buffer() const {
        return mBuffer;
    }

    // ++++++++ ///

    [[[cog
    vars = list(g.do(filename))

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
                     "}}\n".format(type=v[0]+"*", name=v[1], offset=offset))
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
    cog.outl("const uint32_t SIZE = {size};".format(size=offset))
    ]]]
    [[[end]]]

private:
    Buffer mBuffer;
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
