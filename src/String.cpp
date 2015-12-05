//
// Created by steffen on 11.11.15.
//

#include <string>
#include <cstring>
#include <cmath>
#include "libCom/helper.h"
#include "libCom/String.h"
#include "libCom/BufferRange.h"


String::String() : Buffer() { }

String::String(const char *cstring) : String(cstring, static_cast<uint32_t>(strlen_s(cstring))) { }       // FIXME integer is truncated if strlen(cstring) > MAX_UINT32

String::String(const char *cstring, uint32_t size) : Buffer(size) {
    append(cstring, size);
}

String::String(const uint8_t *bytes, uint32_t size) : Buffer(size) {
    append(bytes, size);
}

String::String(const String &other) : String(static_cast<const char*>(other.const_data()), other.size()) { }

String::String(const std::string &stlstring) : Buffer(static_cast<uint32_t>(stlstring.size())) {        // FIXME integer is truncated if stlstring.size() > MAX_UINT32
    append(stlstring.c_str(), static_cast<uint32_t>(stlstring.size()));        // FIXME integer is truncated if stlstring.size() > MAX_UINT32
}

String String::operator+(const String &other) const {
    return concatHelper(static_cast<const char*>(other.const_data()), other.size());
}

String String::operator+(const char *cstring) const {
    return concatHelper(cstring, static_cast<uint32_t>(strlen_s(cstring)));             // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
}

String String::operator+(const std::string &stlstring) const {
    return concatHelper(stlstring.c_str(), static_cast<uint32_t>(stlstring.size()));   // FIXME integer is truncated if stlstring.size() > MAX_UINT32
}

String &String::operator+=(const String &other) {
    append(other.const_data(), other.size());
    return *this;
}

String &String::operator+=(const char *cstring) {
    append(cstring, static_cast<uint32_t>(strlen_s(cstring)));        // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
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

const bool String::operator==(const char *other) const {
    if (other == nullptr)       // without this check, there may occur crashes if == is wrongly used
        return false;

    uint32_t cSize = static_cast<uint32_t>(strlen_s(other));       // FIXME integer is truncated if strlen(cstring) > MAX_UINT32
    if (size() != cSize)
        return false;
    return comparisonHelper(const_data(), other, cSize);
}

const bool String::operator==(const std::string &other) const {
    if (size() != other.size())
        return false;
    return comparisonHelper(const_data(), other.c_str(), size());
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

bool String::toInt(uint8_t base, uint32_t &result) const {
    result = 0;

    if (size() == 0)
        return false;

    constexpr const static char *alphabet = "0123456789abcdefghijklmnopqrstuvwxyz";
    constexpr const static uint8_t gap = 'a' - '9' - 1;     // difference between 9 and a since they are not siblings in ascii
    uint32_t exp = 0;

    // traverse array from back to front
    for (uint32_t i = size(); i != 0; --i) {
        uint8_t val = *(static_cast<const uint8_t*>(const_data(i-1)));
        if (val >= alphabet[0] && val < alphabet[base]) {        // within base bounds

            // need to apply an offset for alphabetic values
            uint32_t diff;
            if (val >= alphabet[10])
                diff = val - gap - alphabet[0];
            else
                diff = val - alphabet[0];

            result += diff * pow(base, exp);
            exp++;
        }
    }

    return exp != 0;
}

String String::toHex(const uint8_t *data, uint32_t size) {
    if (data == nullptr || size == 0)
        return String();

    constexpr const static char *alphabet = "0123456789abcdef";
    constexpr const uint8_t sixteen = static_cast<uint8_t>(16);

    SecureUniquePtr<char[]> final(size*2+1);
    final()[size*2] = '\0';

    String s;
    uint8_t div, rem;

    for (uint32_t i = 0; i < size; ++i) {
        rem = data[i] % sixteen;
        final()[i*2+1] = alphabet[rem];

        div = data[i] / sixteen;
        final()[i*2] = alphabet[div];
    }
    s += final().get();

    return s;
}

/* PRIVATE */
String String::concatHelper(const char *cstring, uint32_t size) const {
    String newString(*this);
    newString.append(cstring, size);
    return newString;
}
