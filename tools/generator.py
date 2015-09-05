import re

i = {
    "uint64_t": 8,
    "uint32_t": 4,
    "uint16_t": 2,
    "uint8_t": 1
}

arr_supported = ('uint8_t',)


def do(filename):

    with open(filename, "r") as f:
        for line in f:
            type, id = l = line.split()
            arr_i = re.split("\(([0-9]+)\)", type)
            arr = int(arr_i[1]) if len(arr_i) > 1 else None     # array element count
            type = arr_i[0]

            # is defined as array, but not supported
            if arr is not None and type not in arr_supported:
                raise Exception(type+" is not supported as array type!")
            yield (type, id, i[type], arr)
    #return filename


#print(list(do("sometest.thx")))
#print(do(filename))
