//
// Created by steffen on 11.11.15.
//

#include <string>
#include <string.h>
#include "libCom/String.h"
#include "libCom/BufferRange.h"


String::String() : Buffer() { }

String::String(const char *cstring) : String(cstring, static_cast<uint32_t>(strlen(cstring))) { }       // FIXME integer is truncated if strlen(cstring) > MAX_UINT32

String::String(const char *cstring, uint32_t size) : Buffer(size) {
    append(cstring, size);
}

String::String(const String &other) : String(static_cast<const char*>(other.const_data()), other.size()) { }

String::String(const std::string &stlstring) : Buffer(static_cast<uint32_t>(stlstring.size())) {        // FIXME integer is truncated if stlstring.size() > MAX_UINT32
    append(stlstring.c_str(), static_cast<uint32_t>(stlstring.size()));        // FIXME integer is truncated if stlstring.size() > MAX_UINT32
}

String String::operator+(const String &other) const {
    return concatHelper(static_cast<const char*>(other.const_data()), other.size());
}

String String::operator+(const char *cstring) const {
    return concatHelper(cstring, static_cast<uint32_t>(strlen(cstring)));             // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
}

String String::operator+(const std::string &stlstring) const {
    return concatHelper(stlstring.c_str(), static_cast<uint32_t>(stlstring.size()));   // FIXME integer is truncated if stlstring.size() > MAX_UINT32
}

String &String::operator+=(const String &other) {
    append(other.const_data(), other.size());
    return *this;
}

String &String::operator+=(const char *cstring) {
    append(cstring, static_cast<uint32_t>(strlen(cstring)));        // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
    return *this;
}

String &String::operator+=(const std::string &stlstring) {
    append(stlstring.c_str(), static_cast<uint32_t>(stlstring.size()));           // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
    return *this;
}

String &String::operator=(const String &other) {
    clear();
    append(other.const_data(), other.size());

    return *this;
}

const bool String::operator==(const String &other) const {
    if (size() != other.size())     // size is different -> they are truly not equal
        return false;
    return comparisonHelper(static_cast<const char *>(other.const_data()));
}

const bool String::operator==(const char *other) const {
    uint32_t cSize = static_cast<uint32_t>(strlen(other));       // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
    if (size() != cSize)
        return false;
    return comparisonHelper(other);
}

const bool String::operator==(const std::string &other) const {
    if (size() != other.size())
        return false;
    return comparisonHelper(other.c_str());
}

const char *String::c_str() {
    // we need to append a 0-termination char to the string, since it's stored without it internally
    // TODO An extra buffer is necessary for guaranteeing validity of returned pointer's memory
    const void *p = mCStrings.append(const_data(), size()).const_data();
    mCStrings.append("", 1);

    return static_cast<const char *>(p);
}

std::string String::stl_str() const {
    return std::string(static_cast<const char*>(const_data()), size());
}

const uint32_t String::size() const {
    return Buffer::size();
}

void String::clear() {
    Buffer::clear();
}

/* PRIVATE */
String String::concatHelper(const char *cstring, uint32_t size) const {
    String newString(*this);
    newString.append(cstring, size);
    return newString;
}

const bool String::comparisonHelper(const char *other) const {
    const char *cthis = static_cast<const char *>(const_data());

    for (uint32_t len = size(); len != 0 && *cthis == *other; cthis++, other++, len--) { }

    return (cthis-static_cast<const char *>(const_data())) == size();       // if they equal, iteration count equals size()
}
