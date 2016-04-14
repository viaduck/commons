#ifndef LIBCOM_SERIALIZABLE_H
#define LIBCOM_SERIALIZABLE_H

/**
 * Class indicating presence .serializable(Buffer &) method. We don't use
 * a pure virtual class because of performance (vtable lookup).
 * TODO: Replace with C++ concepts once available (scheduled for c++17)
 */
class Serializable {

};

#endif //LIBCOM_SERIALIZABLE_H
