#ifndef LIBCOM_OSFILEWRAPPER_H
#define LIBCOM_OSFILEWRAPPER_H

#include <cstdio>
#include <string>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// TODO: add tests
class OSFileWrapper {
public:
    static bool resizeFile(const std::string &path, FILE *&descriptor, const char *mode, size_t newSize) {
        bool ret = false;
#ifdef WIN32
        fseek(descriptor, SEEK_SET, newSize);
        ret = SetEndOfFile(_fileno(descriptor)) != 0;
#else
        fclose(descriptor);
        ret = truncate(path.c_str(), newSize) == 0;
        descriptor = fopen(path.c_str(), mode);
#endif
        return ret;
    }
};

#endif //LIBCOM_OSFILEWRAPPER_H
