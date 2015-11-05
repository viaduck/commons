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
        cog.outl(name+"() : mBuffer(*(new Buffer(STATIC_SIZE))), mAllocated(true) {\n"
                 "     mBuffer.padd(STATIC_SIZE, 0);\n"
                 " }")
    ]]]
    [[[end]]]

    [[[cog
        cog.outl(name+"(Buffer &buffer) : mBuffer(buffer) {")
    ]]]
    [[[end]]]
        if (mBuffer.size() < STATIC_SIZE) {
            mBuffer.increase(STATIC_SIZE);     // prevent access resulting in SIGSEGV if buffer is too small
            mBuffer.use(STATIC_SIZE);
        }
    }

    // copy constructor
    [[[cog
        cog.outl(name+"(const "+name+" &other) : mBuffer(*(new Buffer(other.mBuffer))), mAllocated(true) { }")
    ]]]
    [[[end]]]

    [[[cog
    vars = list(g.do(filename))
    cstatic = sum(1 if v[3] != "var" else 0 for v in vars)
    if cstatic > 0:
        cog.out(name+"(Buffer &buffer")
        for v in vars:
            if v[3] == "var":
                continue
            if v[3] is not None:
                cog.out(", const {type} *_{name}".format(type=v[0], name=v[1]))
            else:
                cog.out(", {type} _{name}".format(type=v[0], name=v[1]))
        cog.out(") : {name}(buffer)".format(name=name)+" {\n")
        offset = 0
        for v in vars:
            if v[3] == "var":
                continue
            if v[3] is not None:
                cog.outl("    memcpy(mBuffer.data({offset}), _{name}, {size});".format(offset=offset, type=v[0], name=v[1], size=v[2]*v[3]))
                offset += v[2]*v[3]     # sizeof type * array element count
            else:
                cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
                offset += v[2]
        cog.outl("}")
    ]]]
    [[[end]]]


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
    if cstatic > 0:
        cog.out(name+"(")
        first = True
        for v in vars:
            if v[3] == "var":
                continue
            if v[3] is not None:
                cog.out((", " if not first else "")+"const {type} *_{name}".format(type=v[0], name=v[1]))
            else:
                cog.out((", " if not first else "")+"{type} _{name}".format(type=v[0], name=v[1]))
            first = False
        cog.out(") : {name}()".format(name=name)+" {\n")
        offset = 0
        for v in vars:
            if v[3] == "var":
                continue
            if v[3] is not None:
                cog.outl("    memcpy(mBuffer.data({offset}), _{name}, {size});".format(offset=offset, type=v[0], name=v[1], size=v[2]*v[3]))
                offset += v[2]*v[3]     # sizeof type * array element count
            else:
                cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
                offset += v[2]
        cog.outl("}")
    ]]]
    [[[end]]]

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
            if v[3] == "var":
                cog.outl("inline const Buffer &const_{name}() const {{\n"
                         "    return const_cast<const Buffer&>(mBuffer_{name});\n"
                         "}}\n"
                         "inline Buffer &{name}() {{\n"
                         "    return mBuffer_{name};\n"
                         "}}\n".format(name=v[1])
                        )
            else:
                cog.outl("inline const {type} const_{name}() const {{\n"
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
    void serialize(Buffer &out) const {
        out.append(mBuffer);

        [[[cog
            for v in vars:
                # variable data type
                if v[3] == "var":
                    cog.outl("// - {name} - //\n"
                             "const uint32_t {name}_size = hton_uint32_t(mBuffer_{name}.size());\n"
                             "out.append(static_cast<const void *>(&{name}_size), sizeof(uint32_t));\n"
                             "out.append(mBuffer_{name});\n".format(name=v[1]))
        ]]]
        [[[end]]]
    }

    const bool deserialize(const Buffer &in, uint32_t missing) {
        if (in.size() < STATIC_SIZE)
            return false;
        // static data
        mBuffer.clear();
        mBuffer.append(BufferRange(in, STATIC_SIZE, 0));

        // now variable data
        uint32_t offset = STATIC_SIZE;
        [[[cog
            nvars = len(vars)
            for i in range(nvars):
                v = vars[i]
                # variable data type
                if v[3] == "var":
                    cog.outl("// - {name} - //\n"
                             "if (in.size()-offset < sizeof(uint32_t)) {    // not big enough to hold size indicator for buffer\n"
                             "    missing = sizeof(uint32_t)-(in.size()-offset);\n"
                             "    return false;\n"
                             "}\n"
                             "const uint32_t mBuffer_{name}_size = ntoh_uint32_t(*static_cast<const uint32_t*>(in.const_data(offset)));\n"
                             "if (in.size()-offset < mBuffer_{name}_size) {       // not big enough to hold var buffer\n"
                             "    missing = mBuffer_{name}_size-(in.size()-offset);\n"
                             "    return false;\n"
                             "}\n"
                             "mBuffer_{name}.clear();\n"
                             "mBuffer_{name}.append(BufferRange(in, offset+sizeof(uint32_t), mBuffer_{name}_size));".format(name=v[1]))
                    if i != (nvars-1):
                        cog.outl("offset += sizeof(uint32_t) + mBuffer_{name}_size;         // go past the size indicator and the var buffer\n".format(name=v[1]))
        ]]]
        [[[end]]]
        return true;
    }
    // ++++++++ ///
    [[[cog
    cog.outl("const static uint32_t STATIC_SIZE = {size};".format(size=offset))
    ]]]
    [[[end]]]
    // ++++++++ ///
    inline const uint32_t size() const {
        return
            [[[cog
                for v in vars:
                    # variable data type
                    if v[3] == "var":
                        cog.outl("mBuffer_{name}.size()+sizeof(uint32_t)+".format(name=v[1]))
            ]]]
            [[[end]]]
            STATIC_SIZE;
    }

private:
    [[[cog
        for v in vars:
            # variable data type
            if v[3] == "var":
                cog.outl("// - "+v[1]+" - //")
                cog.outl("Buffer mBuffer_{name};\n".format(name=v[1]))
    ]]]
    [[[end]]]
    Buffer &mBuffer;
    const bool mAllocated = false;
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]