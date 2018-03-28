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

#ifndef COMMONS_CUSTOM_ASSERT_H
#define COMMONS_CUSTOM_ASSERT_H

#define EXPECT_ARRAY_EQ(TARTYPE, reference, actual, element_count) \
    {\
    TARTYPE* reference_ = reinterpret_cast<TARTYPE *> (reference); \
    TARTYPE* actual_ = reinterpret_cast<TARTYPE *> (actual); \
    for(decltype(element_count) cmp_i = 0; cmp_i < element_count; cmp_i++ ){\
      EXPECT_EQ(reference_[cmp_i], actual_[cmp_i]);\
    }\
    }
#define EXPECT_ARRAY_NEQ(TARTYPE, reference, actual, element_count) \
    {\
    TARTYPE* reference_ = reinterpret_cast<TARTYPE *> (reference); \
    TARTYPE* actual_ = reinterpret_cast<TARTYPE *> (actual); \
    decltype(element_count) cmp_i; \
    for(cmp_i = 0; cmp_i < element_count; cmp_i++ ){\
      if (reference_[cmp_i] != actual_[cmp_i]) break;\
    }\
    ASSERT_NE(element_count, cmp_i) << "All array values are identical";\
    }

#define EXPECT_ANY_OF(expected, actual) \
    {                                                                   \
        bool found = false;                                             \
        std::stringstream str;                                          \
        for (const auto &anyVal: expected) {                            \
            if (anyVal == actual)                                       \
                found = true ;                                          \
            str << "- " << anyVal<< " -";                               \
        }                                                               \
        EXPECT_TRUE(found) << actual << " must be any of " << str.str();\
    }

#endif //COMMONS_CUSTOM_ASSERT_H
