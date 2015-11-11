//
// Created by steffen on 11.11.15.
//

#ifndef LIBCOM_STRING_H
#define LIBCOM_STRING_H

#include "Buffer.h"

class String : protected Buffer {

public:
    /*
     * Creates a String object from a c-style string or byte sequence, copying it's contents.
     *
     * Warning: The string must be 0-terminated!
     * @param cstring C-style string
     */
    String(const char *cstring);

    /*
     * Creates a String object from a c-style string or byte sequence, copying it's contents
     * @param cstring C-style string
     * @param size The c-style string's size in characters (excluding 0-termination)
     */
    String(const char *cstring, uint32_t size);

    /*
     * Creates a String object from an STL string (std::string), copying it's contents
     * @param stlstring The std::string object
     */
    String(const std::string &stlstring);

    /*
     * Creates a String object from another String, copying it's contents
     * @param other The other String object
     */
    String(const String &other);

    /**
     * Concatenates this and another String
     * @param other Other String to concatenate
     * @return A new String object containing the concatenated strings
     */
    String operator+(const String &other) const;
    /**
     * Concatenates this and a c-style string
     * @param other Other c-style string to concatenate
     * @return A new String object containing the concatenated strings
     */
    String operator+(const char *cstring) const;
    /**
     * Concatenates this and an STL string (std::string)
     * @param other Other std::string to concatenate
     * @return A new String object containing the concatenated strings
     */
    String operator+(const std::string &stlstring) const;

    /**
     * Concatenates this and another String
     * @param other Other String to concatenate
     * @return A new String object containing the concatenated strings
     */
    String &operator+=(const String &other);
    /**
     * Concatenates this and a c-style string
     * @param other Other c-style string to concatenate
     * @return A new String object containing the concatenated strings
     */
    String &operator+=(const char *cstring);
    /**
     * Concatenates this and an STL string (std::string)
     * @param other Other std::string to concatenate
     * @return A new String object containing the concatenated strings
     */
    String &operator+=(const std::string &stlstring);


    /**
     * Compares two Strings (byte-comparison)
     * @param other String to compare to this String
     * @return True if they are equal, false if not
     */
    const bool operator==(const String &other) const;

    /**
     * Assigns a new String, copying it's contents
     * @param other The other String object
     * @return Reference to this
     */
    String &operator=(const String &other);

    /**
     * Returns a pointer to a c-style representation (0-terminated) of this String.
     * The returned pointer will always hold the string data at invocation time. Modifications to String later on will NOT be reflected to pointer.
     *
     * It's lifetime is bound to String's lifetime.
     *
     * Warning: Do not alter the returned pointer's memory!
     * @return C-style string
     */
    const char *c_str();

    /**
     * Returns an STL string representation (std::string) of this String
     * @return STL string
     */
    std::string stl_str() const;

    // REDIRECTIONS
    virtual const uint32_t size() const override;
    virtual void clear() override;

protected:
    Buffer mCStrings;       // FIXME holds all returned cstrings

private:
    String concatHelper(const char *cstring, uint32_t size) const;
};


#endif //LIBCOM_STRING_H
