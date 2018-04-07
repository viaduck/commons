[[[cog
    import cog
    from gen_protocol import ProtoDef
    p_def = ProtoDef(def_file)

    p_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    p_def.outl("#ifndef {name}_H")
    p_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <secure_memory/Serializable.h>
#include <secure_memory/Buffer.h>
#include <secure_memory/Range.h>
#include <secure_memory/conversions.h>
#include <commons/Bitfield.h>
#include <cstring>

[[[cog
    # imported includes
    for e_def in p_def.includes:
        e_def.outl('#include "{import_path}"')

    p_def.outl("")
    p_def.outl("{doxygen}")
    p_def.outl("class {name} : public Serializable {{")
]]]
[[[end]]]
public:
    // static constructor
    [[[cog
        # ctr with new buffer
        p_def.outl("{name}() : mBuffer(*(new Buffer(STATIC_SIZE))), mAllocated(true) {{\n"
                   "    mBuffer.padd(STATIC_SIZE, 0);\n"
                   "}}")

        # ctr with existing buffer
        p_def.outl("{name}(Buffer &buffer) : mBuffer(buffer) {{")
        if p_def.n_static > 0:
            p_def.outl("    if (mBuffer.size() < STATIC_SIZE) {{\n"
                       "        // prevent access resulting in SIGSEGV if buffer is too small\n"
                       "        mBuffer.increase(STATIC_SIZE);\n"
                       "        mBuffer.use(STATIC_SIZE);\n"
                       "    }}")
        p_def.outl("}}")

        # copy ctr
        p_def.out("{name}(const {name} &other) : mBuffer(*(new Buffer(other.mBuffer))), mAllocated(true)")
        for elem in p_def.elements:
            if len(elem.copy_extra) > 0:
                elem.out(", " + elem.copy_extra)
        p_def.outl("\n{{ }}")

        # constructor with static fields (without provided buffer)
        if p_def.n_static > 0:
            p_def.out("{name}(")

            # constructor argument list
            ProtoDef.reset_list()
            for elem in p_def.ctr_elements:
                elem.lout(elem.ctr_arg)

            p_def.outl(") : {name}() {{")

            # copy each argument over
            for elem in p_def.ctr_elements:
                elem.outl("    " + elem.ctr_load)

            p_def.outl("}}")

        # ctr with existing buffer and statics
        if p_def.n_static > 0:
            p_def.out("{name}(Buffer &buffer")

            # constructor argument list
            for elem in p_def.ctr_elements:
                elem.out(", " + elem.ctr_arg)

            p_def.outl(") : {name}(buffer) {{")

            # copy each argument over
            for elem in p_def.ctr_elements:
                elem.outl("    " + elem.ctr_load)

            p_def.outl("}}")

        # ctr with existing buffer, statics and ranges
        if p_def.n_ranges > 0:
            p_def.out("{name}(Buffer &buffer")

            # constructor argument list
            for elem in p_def.ctr_elements:
                elem.out(", " + elem.ctr_range_arg)

            p_def.outl(") : {name}(buffer) {{")

            # copy each argument over
            for elem in p_def.ctr_elements:
                elem.outl("    " + elem.ctr_range_load)

            p_def.outl("}}")


        # ctr with new buffer from statics and ranges
        if p_def.n_ranges > 0:
            p_def.out("{name}(")

            # constructor argument list
            ProtoDef.reset_list()
            for elem in p_def.ctr_elements:
                elem.lout(elem.ctr_range_arg)

            p_def.outl(") : {name}() {{")

            # copy each argument over
            for elem in p_def.ctr_elements:
                elem.outl("    " + elem.ctr_range_load)

            p_def.outl("}}")
    ]]]
    [[[end]]]

    // destructor

    [[[cog
        p_def.outl("~{name}() {{\n"
                   "    if (mAllocated)\n"
                   "        delete &mBuffer;\n"
                   "}}")
    ]]]
    [[[end]]]

    // sizes

    [[[cog
        p_def.outl("const static uint32_t STATIC_SIZE = {static_size};\n")

        p_def.outl("inline uint32_t size() const {{")
        p_def.out("    return ")
        for elem in p_def.elements:
            elem.out(elem.size_extra)
        p_def.outl("STATIC_SIZE;")
        p_def.outl("}}")
    ]]]
    [[[end]]]

    // getters and setters

    const Buffer &const_buffer() const {
        return mBuffer;
    }

    Buffer &buffer() {
        return mBuffer;
    }

    // getters (basic, var)

    [[[cog
        for elem in p_def.elements:
            if 'basic' in elem.getter:
                elem.outl("inline {ref_type} {name}() const {{\n"
                          "    " + elem.load + "\n"
                          "}}\n")

                elem.outl("static inline uint32_t {name}_size() {{\n"
                          "   " + elem.size + "\n"
                          "}}\n")

            if 'var' in elem.getter:
                elem.outl("inline const Buffer &const_{name}() const {{\n"
                          "    return const_cast<const Buffer&>(mBuffer_{name});\n"
                          "}}\n")
    ]]]
    [[[end]]]

    // setters (basic, array, enum, var)

    [[[cog
        for elem in p_def.elements:
            if 'basic' in elem.setter:
                elem.outl("inline void {name}({ref_type} v) {{\n"
                          "    " + elem.store + "\n"
                          "}}\n")

            if 'array' in elem.setter:
                elem.outl("inline bool {name}({ref_type} v, const uint32_t size) {{\n"
                          "    if (size > {byte_size}) return false;\n"
                          "    memset(mBuffer.data({offset}+size), 0, {byte_size}-size);        // pad with 0s\n"
                          "    memcpy(mBuffer.data({offset}), v, size);\n"
                          "    return true;\n"
                          "}}\n")

                elem.outl("inline bool {name}(const BufferRangeConst &v) {{\n"
                          "    return {name}(static_cast<{ref_type}>(v.const_data()), v.size());\n"
                          "}}\n")

                elem.outl("inline const BufferRangeConst {name}_range() const {{\n"
                          "    return BufferRangeConst(mBuffer, {offset}, {name}_size());\n"
                          "}}\n")

                elem.outl("inline BufferRange {name}_mutable_range() {{\n"
                           "    return BufferRange(mBuffer, {offset}, {name}_size());\n"
                           "}}\n")

            if 'enum' in elem.setter:
                elem.outl("inline void {name}({base_type} v) {{\n"
                "    {name}(to{type}(v));\n"
                "}}\n")

            if 'var' in elem.setter:
                elem.outl("inline Buffer &{name}() {{\n"
                          "    return mBuffer_{name};\n"
                          "}}")
    ]]]
    [[[end]]]

    // (de)serialization

    bool serialize(Buffer &out) const {
        BufferRange r = out.end();
        return serialize(r);
    }

    bool serialize(BufferRange &range) const {
        return range.applyPolicy(range, size()) && internalSerialize(range.object(), range.offset());
    }

    bool deserialize(const Buffer &in) {
        uint32_t unused;
        return deserialize(in, unused);
    }

    [[[cog
        # name variable only if it is used
        if len(p_def.elements) > 0:
            p_def.outl("bool deserialize(const Buffer &in, uint32_t &missing) {{")
        else:
            p_def.outl("bool deserialize(const Buffer &in, uint32_t &) {{")

        # static size check
        if p_def.n_static > 0:
             p_def.outl("    if (in.size() < STATIC_SIZE) {{\n"
                        "        missing = STATIC_SIZE - in.size();\n"
                        "        return false;\n"
                        "    }}\n")

        # copy static data
        p_def.outl("    mBuffer.clear();")
        p_def.outl("    mBuffer.append(BufferRangeConst(in, 0, STATIC_SIZE));\n")

        # variable arrays
        if p_def.n_vars > 0:
            p_def.outl("    // now variable data")
            p_def.outl("    uint32_t offset = STATIC_SIZE;\n")

            for elem in p_def.elements:
                if 'var' in elem.getter:
                    elem.outl("    // - {name} -\n"
                              "    // not big enough to hold size indicator for buffer\n"
                              "    if (in.size()-offset < sizeof({var_type})) {{\n"
                              "        missing = sizeof({var_type})-(in.size()-offset);\n"
                              "        return false;\n"
                              "    }}\n"
                              "    const {var_type} mBuffer_{name}_size = ntoh(*static_cast<const {var_type}*>(in.const_data(offset)));\n"
                              "    // not big enough to hold var buffer\n"
                              "    if (in.size()-offset < mBuffer_{name}_size+sizeof({var_type})) {{\n"
                              "        missing = mBuffer_{name}_size-(in.size()-offset-sizeof({var_type}));\n"
                              "        return false;\n"
                              "    }}\n"
                              "    mBuffer_{name}.clear();\n"
                              "    mBuffer_{name}.append(BufferRangeConst(in, offset+sizeof({var_type}), mBuffer_{name}_size));\n"
                              "    // go past the size indicator and the var buffer\n"
                              "    offset += sizeof({var_type}) + mBuffer_{name}_size;")

        p_def.outl("    return true;")
        p_def.outl("}};")
    ]]]
    [[[end]]]

private:
    bool internalSerialize(Buffer &out, uint32_t offset=0) const {
        out.write(mBuffer, offset);
        offset += STATIC_SIZE;

        [[[cog
            for elem in p_def.elements:
                if 'var' in elem.setter:
                    elem.outl("// {name}\n"
                              "if (mBuffer_{name}.size() > std::numeric_limits<{var_type}>::max()) return false;\n"
                              "const {var_type} {name}_size = hton(static_cast<{var_type}>(mBuffer_{name}.size()));\n"
                              "out.write(static_cast<const void *>(&{name}_size), sizeof({var_type}), offset);\n"
                              "offset += sizeof({var_type});\n"
                              "out.write(mBuffer_{name}, offset);\n"
                              "offset += mBuffer_{name}.size();\n")
        ]]]
        [[[end]]]

        return true;
    }

    Buffer &mBuffer;
    bool mAllocated = false;

    [[[cog
        for elem in p_def.elements:
            if 'var' in elem.getter or 'var' in elem.setter:
                elem.outl("// {name}\n"
                          "Buffer mBuffer_{name};\n")
    ]]]
    [[[end]]]

[[[cog
    p_def.outl("}};")
    p_def.outl("#endif //{name}_H")
]]]
[[[end]]]