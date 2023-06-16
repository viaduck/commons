/*
 * Copyright (C) 2015-2018 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMONS_CONSTEXPRSTRING_H
#define COMMONS_CONSTEXPRSTRING_H

using size_type = std::size_t;
/**
 * String implementation using only constexpr. Supports concatenation and C conversion.
 * @tparam Size String's size.
 */
template <size_type Size>
class ConstexprString {
public:
    using Type = char const (&) [Size+1];

    /**
     * Constructs a ConstexprString from a C string.
     * @param src Source C String.
     */
    explicit constexpr ConstexprString(Type src) : ConstexprString(src, std::make_index_sequence<Size>()){}

    /**
     * @return String size excluding terminating 0 character.
     */
    constexpr size_type size() const {
        return Size;
    }

    /**
     * @return Pointer to C string representation.
     */
    constexpr Type c_str() const {
        return data;
    }

    /**
     * Concatenates two ConstexprString instances by creating a new one.
     * @tparam OtherSize The other ConstexprString size.
     * @param other The other ConstexprString.
     * @return New ConstexprString of size Size+OtherSize and data: ThisData+OtherData.
     */
    template <size_type OtherSize>
    constexpr ConstexprString<Size+OtherSize> operator+(ConstexprString<OtherSize> other) const {
        return combine(*this, std::make_index_sequence<Size>(), other, std::make_index_sequence<OtherSize>());
    }

    /**
     * Compares two ConstexprString instances. This only compares the strings' sizes because this is enough for our use
     * cases!
     * @tparam OtherSize The other ConstexprString's size.
     * @param other The other ConstexprString.
     * @return True if strings' sizes are equal, false if not.
     */
    template <size_type OtherSize>
    constexpr bool operator==(ConstexprString<OtherSize> other) const {
        return size() == other.size();
    }

    /**
     * Compares two ConstexprString instances. This only compares the strings' sizes because this is enough for our use
     * cases!
     * @tparam OtherSize The other ConstexprString's size.
     * @param other The other ConstexprString.
     * @return True if strings' sizes are not equal, false if they are.
     */
    template <size_type OtherSize>
    constexpr bool operator!=(ConstexprString<OtherSize> other) const {
        return !operator==(other);
    }

private:
    template <size_type ... Indices>
    constexpr ConstexprString(Type src, std::index_sequence<Indices...>) : data{src[Indices]..., '\0'} {}

    template <size_type ... Indices2, size_type ... Indices1, size_type OtherSize>
    constexpr ConstexprString<Size+OtherSize> combine(ConstexprString<Size> src1, std::index_sequence<Indices1...>, ConstexprString<OtherSize> src2, std::index_sequence<Indices2...>) const {
        constexpr size_type newSize = Size+OtherSize;
        // Create a char array that contains the first string's data followed by the second string's and a 0 terminator
        // by unpacking the index sequences.
        // This compiles to: src1.data[0], ..., src1.data[Size-1], src2.data[0], ..., src2.data[OtherSize-1], '\0'
        const char newData[newSize+1] = {src1.data[Indices1]..., src2.data[Indices2]..., '\0'};
        return ConstexprString<newSize>(newData);
    }

public:
    char data[Size+1];      // must contain \0 terminator
};

/**
 * Creates a ConstexprString from a C string.
 * @tparam Size String's size.
 * @param src Source C string to be converted.
 * @return ConstexprString with src as data.
 */
template <size_type Size>
constexpr ConstexprString<Size-1> MakeConstexprString(char const (&src) [Size]) {
    return ConstexprString<Size-1>(src);
}

template<size_type Sep, size_type Size>
constexpr auto JoinConstexprStrings(const char (&) [Sep], const char (&src) [Size]) {
    return MakeConstexprString(src);
}
template<size_type Sep, size_type Size>
constexpr auto JoinConstexprStrings(const char (&) [Sep], ConstexprString<Size> src) {
    return src;
}
template<size_type Sep, size_type Size, typename... Args>
constexpr auto JoinConstexprStrings(const char (&sep) [Sep], const char (&src) [Size], Args&& ...args) {
    return MakeConstexprString(src) + MakeConstexprString(sep) + JoinConstexprStrings(sep, args...);
}
template<size_type Sep, size_type Size, typename... Args>
constexpr auto JoinConstexprStrings(const char (&sep) [Sep], ConstexprString<Size> src, Args&& ...args) {
    return src + MakeConstexprString(sep) + JoinConstexprStrings(sep, args...);
}

#endif //COMMONS_CONSTEXPRSTRING_H
