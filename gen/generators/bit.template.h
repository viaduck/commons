[[[cog
    import cog
    from gen_bit import BitDef
    b_def = BitDef(def_file)

    b_def.outl("/*** AUTOMATICALLY GENERATED FILE - DO NOT EDIT! ***/")
    b_def.outl("#ifndef {name}_H")
    b_def.outl("#define {name}_H")
]]]
[[[end]]]

#include <secure_memory/conversions.h>
#include <commons/Bitfield.h>

[[[cog
    b_def.out("{doxygen}")
    b_def.outl("class {name} {{")
]]]
[[[end]]]
public:
    // constructors
    [[[cog
        # ctr with new field
        b_def.outl("{name}() : mField(new {type}()), mAllocated(true) {{ }}\n")

        # ctr with existing field
        b_def.outl("{name}({type} *field) : mField(field), mAllocated(false) {{ }}\n")

        # ctr with new field, set args
        b_def.out("{name}(")

        BitDef.reset_list()
        for elem in b_def.elements:
            elem.lout("{type} _{name}")

        b_def.outl(") : {name}() {{")
        for elem in b_def.elements:
            elem.outl("    {name}(_{name});")
        b_def.outl("}}")

        # ctr with existing field, set args
        b_def.out("{name}({type} *field")

        for elem in b_def.elements:
            elem.out(", {type} _{name}")

        b_def.outl(") : {name}(field) {{")
        for elem in b_def.elements:
            elem.outl("    {name}(_{name});")
        b_def.outl("}}")
    ]]]
    [[[end]]]

    // destructor

    [[[cog
        b_def.outl("~{name}() {{\n"
                   "    if (mAllocated)\n"
                   "        delete mField;\n"
                   "}}")
    ]]]
    [[[end]]]

    // getter and setter

    [[[cog
        b_def.outl("{type} value() const {{\n"
                   "    return ntoh(*mField);\n"
                   "}}\n")

        b_def.outl("void value({type} v) {{\n"
                   "    *mField = hton(v);\n"
                   "}}\n")

        for elem in b_def.elements:
            elem.outl("{type} {name}() {{\n"
                      "    return Bitfield::get<{type}>({offset}, {size}, value());\n"
                      "}}\n")

            elem.outl("uint32_t {name}_width() {{\n"
                      "    return {size};\n"
                      "}}\n")

            elem.outl("void {name}({type} v) {{\n"
                      "    auto fld = value();\n"
                      "    Bitfield::set({offset}, {size}, v, fld);\n"
                      "    value(fld);\n"
                      "}}\n")
    ]]]
    [[[end]]]

private:
    // members
    [[[cog
        b_def.outl("{type} *mField;")
    ]]]
    [[[end]]]
    bool mAllocated = false;

[[[cog
    b_def.outl("}};")
    b_def.outl("#endif //{name}_H")
]]]
[[[end]]]
