#ifndef LIBCOM_SERIALIZABLE_H
#define LIBCOM_SERIALIZABLE_H

/**
 * Class indicating presence of some methods for data serialization. We don't use a pure virtual class because of
 * performance (vtable lookup).
 * TODO: Replace with C++ concepts once available (scheduled for c++17)
 */
class Serializable {
    // uint32_t size() const;
    // void serialize(Buffer&) const;
    // bool deserialize(const Buffer&);
};

#endif //LIBCOM_SERIALIZABLE_H
