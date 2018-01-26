#ifndef VDCOMMONS_CUSTOM_ASSERT_H
#define VDCOMMONS_CUSTOM_ASSERT_H

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

#endif //VDCOMMONS_CUSTOM_ASSERT_H
