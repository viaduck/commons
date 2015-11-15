//
// Created by steffen on 11.11.15.
//

#ifndef LIBCOM_STRING_H
#define LIBCOM_STRING_H

#include <istream>

#include "Buffer.h"

class String : protected Buffer {

public:
    /**
     * Creates an empty String
     */
    String();

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
     * Creates a String object from a byte sequence, copying it's contents
     * @param bytes Byte sequence
     * @param size The c-style string's size in characters (excluding 0-termination)
     */
    String(const uint8_t *bytes, uint32_t size);

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
     * Compares a String and a cstring (byte-comparison)
     * @param other Cstring to compare to this String (must be 0-terminated)
     * @return True if they are equal, false if not
     */
    const bool operator==(const char *other) const;
    /**
     * Compares a String and an stl string (byte-comparison)
     * @param other STL string (std::string) to compare to this String
     * @return True if they are equal, false if not
     */
    const bool operator==(const std::string &other) const;

    /**
     * Compares two Strings (byte-comparison)
     * @param other String to compare to this String
     * @return True if they are equal, false if not
     */
    inline const bool operator!=(const String &other) const {
        return !operator==(other);
    }
    /**
     * Compares a String and a cstring (byte-comparison)
     * @param other Cstring to compare to this String (must be 0-terminated)
     * @return True if they are equal, false if not
     */
    inline const bool operator!=(const char *other) const {
        return !operator==(other);
    }
    /**
     * Compares a String and an stl string (byte-comparison)
     * @param other STL string (std::string) to compare to this String
     * @return True if they are equal, false if not
     */
    inline const bool operator!=(const std::string &other) const {
        return !operator==(other);
    }

    /**
     * Checks if String is empty
     * @return True if String is empty, false if not
     */
    inline const bool isEmpty() const {
        return size() == 0;
    }

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

    /**
     * Casts this to Buffer, exposing the underlying Buffer methods
     * @return This casted to Buffer
     */
    inline const Buffer &toBuffer() const {
        return *dynamic_cast<const Buffer*>(this);
    }

    /**
     * Creates a Hex string from the specified binary data
     * @param data Binary data
     * @param size Data size
     * @return String instance containing the hex string
     */
    static String toHex(const uint8_t *data, uint32_t size);

    /**
     * Stream operator supporting e.g. streaming std::cin into String
     * @param is std::istream to read from
     * @param string String instance to write contents to
     * @return param is (for chaining)
     */
    friend std::istream &operator>>(std::istream &is, String &string) {
        string.increase(sizeof(char)*512);
        is.getline(static_cast<char *>(string.data()), sizeof(char)*512);
        string.use(static_cast<uint32_t>(is.gcount()-1));     // dont want the istream 0-terminator; cap is OK, since we limit it to sizeof(char)*512
        return is;
    }

    /**
     * Stream operator supporting e.g. streaming String into std::out
     * @param is std::ostream to write to
     * @param string String instance to read contents from
     * @return param os (for chaining)
     */
    friend std::ostream &operator<<(std::ostream &os, const String &string) {
        os.write(static_cast<const char *>(string.const_data()), string.size());
        return os;
    }

    // REDIRECTIONS
    inline virtual const uint32_t size() const override {
        return Buffer::size();
    }

    inline virtual void clear() override {
        Buffer::clear();
    }

protected:
    Buffer mCStrings;       // FIXME holds all returned cstrings

private:
    String concatHelper(const char *cstring, uint32_t size) const;
};


#endif //LIBCOM_STRING_H
