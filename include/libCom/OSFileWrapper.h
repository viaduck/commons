#ifndef LIBCOM_OSFILEWRAPPER_H
#define LIBCOM_OSFILEWRAPPER_H

#include <cstdio>
#include <string>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif


// TODO: add tests
/**
 * Cross platform (POSIX, Windows) wrapper for common filesystem operations.
 *
 * Scheduled for removal once std::filesystem is standardized, released and well-supported.
 */
class OSFileWrapper {
public:
    /**
     * Resizes a file described by stream to newSize. This both enlarges and shrinks files.
     * @param stream File stream. This parameter is NOT checked for validness.
     * @param newSize File's new size
     * @return True on success, false on failure
     */
    static bool resizeFile(FILE *stream, size_t newSize) {
        bool ret = false;
#ifdef WIN32
        fseek(stream, SEEK_SET, newSize);
        ret = SetEndOfFile((HANDLE)_fileno(stream)) != 0;
#else
        ret = ftruncate(fileno(stream), newSize) == 0;
#endif
        return ret;
    }

    /**
     * Gets the size of an opened file stream.
     * @param stream File stream. This parameter is NOT checked for validness.
     * @return File size or 0 on failure.
     */
    static uint64_t size(FILE *stream) {
        uint64_t ret = 0;

#ifdef WIN32
        LARGE_INTEGER outSize;
        if (GetFileSizeEx((HANDLE)_fileno(stream), &outSize) != 0)
            ret = outSize.QuadPart;
#else
        struct stat st;
        if (fstat(fileno(stream), &st) == 0)
            ret = static_cast<uint64_t>(st.st_size);
#endif
        return ret;
    }
};

#endif //LIBCOM_OSFILEWRAPPER_H
