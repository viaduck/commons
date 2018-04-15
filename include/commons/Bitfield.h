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

#ifndef COMMONS_BITFIELD_H
#define COMMONS_BITFIELD_H

#include <cstdint>
#include <type_traits>

/**
 * This class provides static methods for Bitfield access to integral variables
 */
class Bitfield {
public:
    /**
     * Sets/unsets bits in a Bitfield with an offset in a given width.
     * @param T Bits value type
     * @param U Field type
     * @param offset Offset within the field
     * @param width Number of bits to set
     * @param val Bits value to set
     * @param field Bitfield
     */
    template<typename T, typename U>
    inline static void set(uint8_t offset, uint8_t width, T val, U &field) {
        static_assert(std::is_integral<T>::value && std::is_integral<U>::value, "Bitfield operations only supported for integral types!");
        // Idea: get all bits EXCEPT those, that are going to be set/unset. To this value OR the value.
        // Example: offset = 2, width=3, U is 8bit, field = 10011101, val = 110
        //  - mask: XXX000XX (computed from XXXXXXXX ^ 000XXX00)
        //  - applied mask to field: 10000001
        //  - ORed with val: 10011001
        field = (
                        (
                                (~static_cast<U>(0))        // build as many 1s as needed for the field's size
                                ^ (((1<<static_cast<U>(width))-1) << offset)        // remove 1s at the position of the bits going to be replaced
                        ) & field       // apply the mask
                ) |             // OR the value to this masked value
                ((static_cast<U>(val) & ((1<<static_cast<U>(width))-1))     // clamp the value to guarantee, at most "width" bits are set
                        << offset);                  // move value by offset
    }

    /**
     * Gets bits from a Bitfield with an offset in a given width.
     * @param T Bits value type
     * @param U Field type
     * @param offset Offset within the field
     * @param width Number of bits to set
     * @param field Bitfield
     */
    template<typename T, typename U>
    inline static T get(uint8_t offset, uint8_t width, U field) {
        static_assert(std::is_integral<T>::value && std::is_integral<U>::value, "Bitfield operations only supported for integral types!");
        return (
                (static_cast<U>((1<<width)-1)<<offset)  // build a mask for the bits to be returned
                & field) >> offset;                     // apply the mask and normalize the bits (move them to pos 0)
    }
};

#endif //COMMONS_BITFIELD_H
