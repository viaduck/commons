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


#include "libCom/Serializable.h"
#include "libCom/Buffer.h"
#include "libCom/Range.h"
#include "libCom/conversions.h"
#include "libCom/Bitfield.h"
#include <cstring>


[[[cog
    cog.outl("class "+name+" : public Serializable {")
]]]
[[[end]]]
public:

    // constructor without provided buffer
    [[[cog
    vars = list(g.do(filename))
    cstatic = sum(1 if v[3] != "var" else 0 for v in vars)      # number of static elements (<-> not variable array)
    cranges = sum(1 if (v[3] != None and v[3] != "var") else 0 for v in vars)    # number of range fields

    if cstatic > 0:
        cog.out(name+"(")
        first = True
        for v in vars:
            if v[3] == "var":
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.out((", " if not first else "")+"{type} _{name}".format(type=g.bits_to_type(sub['bits']), name=sub['name']))
            elif v[3] is not None:
                cog.out((", " if not first else "")+"const {type} *_{name}".format(type=v[0], name=v[1]))
            else:
                cog.out((", " if not first else "")+"{type} _{name}".format(type=v[0], name=v[1]))
            first = False
        cog.out(") : {name}()".format(name=name)+" {\n")        # call buffer allocating constructor
        offset = 0
        for v in vars:
            if v[3] == "var":
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.outl("    {name}(_{name});".format(name=sub['name']))
            elif v[3] is not None:
                cog.outl("    memcpy(mBuffer.data({offset}), _{name}, {size});".format(offset=offset, type=v[0], name=v[1], size=v[2]*v[3]))
                offset += v[2]*v[3]     # sizeof type * array element count
            else:
                cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
                offset += v[2]
        cog.outl("}")
    ]]]
    [[[end]]]

    // GETTER & SETTER //
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
        # array types
        if v[3] is not None:
            if v[3] == "var":       # variable array
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
                         "    if (size > {size}) return false;\n"
                         "    memset(mBuffer.data({offset}+size), 0, {size}-size);        // pad with 0s\n"
                         "    memcpy(mBuffer.data({offset}), v, size);\n"
                         "    return true;\n"
                         "}}\n"
                         "inline const BufferRangeConst {name}_range() const {{\n"
                         "    return BufferRangeConst(mBuffer, {offset}, {name}_size());\n"
                         "}}\n"
                         "inline BufferRange {name}_mutable_range() {{\n"
                         "    return BufferRange(mBuffer, {offset}, {name}_size());\n"
                         "}}\n"
                         "static inline uint32_t {name}_size() {{\n"
                         "    return sizeof({type_raw})*{count};\n"
                         "}}\n".format(type=v[0]+"*", type_raw=v[0], name=v[1], offset=offset, count=v[3], size=v[2]*v[3]))
                offset += v[2]*v[3]     # sizeof type * array element count
        else:
        # non-array types
            if v[4] is not None:            # subs
                shift_offset = 0
                for sub in v[4]:
                    cog.outl("inline {sub_type} {sub_name}() const {{\n"
                    "    return Bitfield::get<{sub_type}>({shift_offset}, {sub_bits}, *static_cast<const {type}*>(mBuffer.const_data({offset})));\n"
                    "}}\n"
                    "inline void {sub_name}({sub_type} v) {{\n"
                    "    Bitfield::set({shift_offset}, {sub_bits}, v, *static_cast<{type}*>(mBuffer.data({offset})));\n"
                    "}}\n"
                    "static inline uint32_t {sub_name}_size() {{\n"
                    "    return {sub_bits};\n"
                    "}}\n".format(sub_bits=sub['bits'], type=v[0], sub_type=g.bits_to_type(sub['bits']), name=v[1], sub_name=sub['name'], offset=offset, shift_offset=shift_offset, conversion="hton_"+v[0], conversion_sub="hton_"+g.bits_to_type(sub['bits'])))
                    shift_offset += sub['bits']
            else:
                cog.outl("inline {type} {name}() const {{\n"
                         "    return {conversion}(*static_cast<const {type}*>(mBuffer.const_data({offset})));\n"
                         "}}\n"
                         "inline void {name}({type} v) {{\n"
                         "    *static_cast<{type}*>(mBuffer.data({offset})) = {conversion}(v);\n"
                         "}}\n"
                         "static inline uint32_t {name}_size() {{\n"
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
                # variable array
                if v[3] == "var":
                    cog.outl("// - {name} - //\n"
                             "const uint32_t {name}_size = hton_uint32_t(mBuffer_{name}.size());\n"
                             "out.append(static_cast<const void *>(&{name}_size), sizeof(uint32_t));\n"
                             "out.append(mBuffer_{name});\n".format(name=v[1]))
        ]]]
        [[[end]]]
    }

    bool deserialize(const Buffer &in) {
        uint32_t unused;
        return deserialize(in, unused);
    }

    bool deserialize(const Buffer &in, uint32_t &missing) {
        [[[cog
        if offset != 0:
            cog.outl("    if (in.size() < STATIC_SIZE) {\n"
                     "        missing = STATIC_SIZE - in.size();\n"
                     "        return false;\n"
                     "    }")
        ]]]
        [[[end]]]
        // static data
        mBuffer.clear();
        mBuffer.append(BufferRangeConst(in, 0, STATIC_SIZE));

        [[[cog
            nvars = len(vars)
            first = False
            for i in range(nvars):
                v = vars[i]
                # variable array
                if v[3] == "var":
                    if not first:
                        cog.outl("// now variable data")
                        cog.outl("uint32_t offset = STATIC_SIZE;")
                        first = True
                    cog.outl("// - {name} - //\n"
                             "if (in.size()-offset < sizeof(uint32_t)) {{    // not big enough to hold size indicator for buffer\n"
                             "    missing = sizeof(uint32_t)-(in.size()-offset);\n"
                             "    return false;\n"
                             "}}\n"
                             "const uint32_t mBuffer_{name}_size = ntoh_uint32_t(*static_cast<const uint32_t*>(in.const_data(offset)));\n"
                             "if (in.size()-offset < mBuffer_{name}_size+sizeof(uint32_t)) {{       // not big enough to hold var buffer\n"
                             "    missing = mBuffer_{name}_size-(in.size()-offset-sizeof(uint32_t));\n"
                             "    return false;\n"
                             "}}\n"
                             "mBuffer_{name}.clear();\n"
                             "mBuffer_{name}.append(BufferRangeConst(in, offset+sizeof(uint32_t), mBuffer_{name}_size));".format(name=v[1]))
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
    inline uint32_t size() const {
        return
            [[[cog
                for v in vars:
                    # variable array
                    if v[3] == "var":
                        cog.outl("mBuffer_{name}.size()+sizeof(uint32_t)+".format(name=v[1]))
            ]]]
            [[[end]]]
            STATIC_SIZE;
    }

    // constructors
    [[[cog
        cog.outl(name+"() : mBuffer(*(new Buffer(STATIC_SIZE))), mAllocated(true) {\n"
                 "     mBuffer.padd(STATIC_SIZE, 0);\n"
                 " }")
    ]]]
    [[[end]]]

    [[[cog
        cog.outl(name+"(Buffer &buffer) : mBuffer(buffer) {")
        if offset != 0:
            cog.outl("    if (mBuffer.size() < STATIC_SIZE) {\n"
                     "        mBuffer.increase(STATIC_SIZE);     // prevent access resulting in SIGSEGV if buffer is too small\n"
                     "        mBuffer.use(STATIC_SIZE);\n"
                     "    }")
    ]]]
    [[[end]]]
    }

    // copy constructor
    [[[cog
    vars = list(g.do(filename))
    cog.out(name+"(const "+name+" &other) : mBuffer(*(new Buffer(other.mBuffer))), mAllocated(true)")
    for v in vars:
        if v[3] == "var":           # variable type
            cog.out(", mBuffer_{name}(other.mBuffer_{name})".format(name=v[1]))
    cog.outl("\n{ }")
    ]]]
    [[[end]]]

    // constructor with provided buffer
    [[[cog
    vars = list(g.do(filename))
    if cstatic > 0:
        cog.out(name+"(Buffer &buffer")
        for v in vars:
            if v[3] == "var":           # variable array
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.out(", {type} _{name}".format(type=g.bits_to_type(sub['bits']), name=sub['name']))
            elif v[3] is not None:        # array
                cog.out(", const {type} *_{name}".format(type=v[0], name=v[1]))
            else:                       # normal type
                cog.out(", {type} _{name}".format(type=v[0], name=v[1]))
        cog.out(") : {name}(buffer)".format(name=name)+" {\n")
        offset = 0
        for v in vars:
            if v[3] == "var":           # variable type
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.outl("    {name}(_{name});".format(name=sub['name']))
            elif v[3] is not None:        # array
                cog.outl("    memcpy(mBuffer.data({offset}), _{name}, {size});".format(offset=offset, type=v[0], name=v[1], size=v[2]*v[3]))
                offset += v[2]*v[3]     # sizeof type * array element count
            else:                   # normal type
                cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
                offset += v[2]
        cog.outl("}")
    ]]]
    [[[end]]]
    
    // -- construct from ranges
    [[[cog
    vars = list(g.do(filename))
    if cstatic > 0 and cranges > 0:
        cog.out(name+"(Buffer &buffer")
        for v in vars:
            if v[3] == "var":           # variable array
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.out(", {type} _{name}".format(type=g.bits_to_type(sub['bits']), name=sub['name']))
            elif v[3] is not None:        # array
                cog.out(", const BufferRangeConst _{name}".format(type=v[0], name=v[1]))
            else:                       # normal type
                cog.out(", {type} _{name}".format(type=v[0], name=v[1]))
        cog.out(") : {name}(buffer)".format(name=name)+" {\n")
        for v in vars:
            if v[3] == "var":           # variable type
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.outl("    {name}(_{name});".format(name=sub['name']))
            elif v[3] is not None:        # array
                cog.outl("    {name}(static_cast<const {type}*>(_{name}.const_data()), _{name}.size());".format(type=v[0], name=v[1], size=v[2]*v[3]))
            else:                   # normal type
                cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
        cog.outl("}")
    ]]]
    [[[end]]]

    [[[cog
    vars = list(g.do(filename))
    if cstatic > 0 and cranges > 0:
        outVars = []
        cog.out(name+"(")
        for v in vars:
            if v[3] == "var":           # variable array
                continue
            if v[4] is not None:
                for sub in v[4]:
                    outVars += ["{type} _{name}".format(type=g.bits_to_type(sub['bits']), name=sub['name'])]
            elif v[3] is not None:        # array
                #cog.out(", const BufferRangeConst _{name}".format(type=v[0], name=v[1]))
                outVars += ["const BufferRangeConst _{name}".format(type=v[0], name=v[1])]
            else:                       # normal type
                #cog.out(", {type} _{name}".format(type=v[0], name=v[1]))
                outVars += ["{type} _{name}".format(type=v[0], name=v[1])]
        cog.out(', '.join(outVars))

        otherargs = []
        for v in vars:
            if v[3] != "var":
                otherargs += ["_{name}".format(name=v[1])]

        cog.out(") : {name}()".format(name=name)+" {\n")
        for v in vars:
            if v[3] == "var":           # variable type
                continue
            if v[4] is not None:
                for sub in v[4]:
                    cog.outl("    {name}(_{name});".format(name=sub['name']))
            elif v[3] is not None:        # array
                cog.outl("    {name}(static_cast<const {type}*>(_{name}.const_data()), _{name}.size());".format(type=v[0], name=v[1], size=v[2]*v[3]))
            else:                   # normal type
                cog.outl("    {name}(_{name});".format(type=v[0], name=v[1]))
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

private:
    Buffer &mBuffer;
    bool mAllocated = false;

    [[[cog
        for v in vars:
            # variable array
            if v[3] == "var":
                cog.outl("// - "+v[1]+" - //")
                cog.outl("Buffer mBuffer_{name};\n".format(name=v[1]))
    ]]]
    [[[end]]]
};

[[[cog
cog.outl("#endif //{name}_H".format(name=name))
]]]
[[[end]]]
