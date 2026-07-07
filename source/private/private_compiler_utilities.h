/** @cond
Copyright 2025 Alexander G. Lopez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
@endcond */
/** @internal
@file
@brief The Private Compiler Utility Implementations

This file implements the internal private header that allows containers to use
low level compiler built-in functionality or extremely common helper functions.
It is not user facing and will not be exported as an interface. This file
includes detection of compiler built-ins such as bit utilities. It attempts to
use bit utilities from the new `__builtin_stdc_*` family when available as these
are at the cutting edge of C23's new `stdbit.h` standard. However, sane fallback
implementations are always provided. This file should be updated frequently as
new C23 specifications are finalized by compiler maintainers. */
#ifndef CCC_PRIVATE_COMPILER_UTILITIES
#define CCC_PRIVATE_COMPILER_UTILITIES

#ifndef __has_builtin
/* @internal */
#    define __has_builtin(x) 0
#endif /* __has_builtin */

/*=============================   Inlining   ================================*/

#if defined(__GNUC__) || defined(__clang__)
/* @internal */
#    define CCC_PRIVATE_INLINE [[gnu::always_inline]] static inline
#else
/* @internal */
#    define CCC_PRIVATE_INLINE static inline
#endif /* defined(__GNUC__) || defined(__clang__) */

/*=======================   Branch Annotation   =============================*/

#if __has_builtin(__builtin_expect)
/* @internal */
#    define CCC_private_unlikely(expr) __builtin_expect(!!(expr), 0)
/* @internal */
#    define CCC_private_likely(expr) __builtin_expect(!!(expr), 1)
#else /* !__has_builtin(__builtin_expect) */
/* @internal */
#    define CCC_private_unlikely(expr) expr
/* @internal */
#    define CCC_private_likely(expr) expr
#endif /* __has_builtin(__builtin_expect) */

/*========================   Integer Rounding   =============================*/

/* @internal */
#define CCC_private_roundup(integer, alignment)                                         \
    ((typeof((integer) + (alignment)))(((typeof((integer) + (alignment)))(integer)      \
                                        + ((typeof((integer) + (alignment)))(alignment) \
                                           - 1))                                        \
                                       & ~((typeof((integer) + (alignment)))(alignment) \
                                           - 1)))

/* @internal */
#define CCC_private_checked_roundup(result_pointer, integer, alignment)        \
    (__extension__({                                                           \
        /*NOLINTBEGIN(bugprone-assignment-in-if-condition)*/                   \
        typedef typeof((integer) + (alignment)) ccc_private_max_width_type;    \
        ccc_private_max_width_type ccc_private_integer = (integer);            \
        ccc_private_max_width_type ccc_private_alignment                       \
            = (ccc_private_max_width_type)(alignment);                         \
        typeof(result_pointer) ccc_private_result_pointer = (result_pointer);  \
        bool ccc_private_has_overflow = false;                                 \
        if (ckd_add(                                                           \
                ccc_private_result_pointer,                                    \
                ccc_private_integer,                                           \
                ccc_private_alignment - 1                                      \
            )) {                                                               \
            ccc_private_has_overflow = true;                                   \
        } else {                                                               \
            *ccc_private_result_pointer &= ~(ccc_private_alignment - 1);       \
        }                                                                      \
        ccc_private_has_overflow;                                              \
        /*NOLINTEND(bugprone-assignment-in-if-condition)*/                     \
    }))

/*======================   Integer Log Base 2  ==============================*/

#if __has_builtin(__builtin_stdc_leading_zeros)

/* @internal */
#    define CCC_private_log2(x)                                                \
        ((int)(((sizeof(x) * CHAR_BIT) - 1)                                    \
               - __builtin_stdc_leading_zeros((typeof(x))((x)                  \
                                                          | (typeof(x))1))))

#elif __has_builtin(__builtin_clzg)

/* @internal */
#    define CCC_private_log2(x)                                                \
        ((int)((sizeof(x) * CHAR_BIT) - 1)                                     \
         - __builtin_clzg((typeof(x))((x) | (typeof(x))1)))

#elif __has_builtin(__builtin_clzll)

/* @internal */
#    define CCC_private_log2(x)                                                \
        ((int)((sizeof(unsigned long long) * CHAR_BIT) - 1)                    \
         - __builtin_clzll((unsigned long long)((x) | (typeof(x))1)))

#else /* PORTABLE FALLBACK */

/* @internal */
#    define CCC_private_log2_1(x) ((x) >= 0x2ULL ? 1 : 0)
/* @internal */
#    define CCC_private_log2_2(x)                                              \
        ((x) >= 0x4ULL ? 2 + CCC_private_log2_1((x) >> 2)                      \
                       : CCC_private_log2_1(x))
/* @internal */
#    define CCC_private_log2_4(x)                                              \
        ((x) >= 0x10ULL ? 4 + CCC_private_log2_2((x) >> 4)                     \
                        : CCC_private_log2_2(x))
/* @internal */
#    define CCC_private_log2_8(x)                                              \
        ((x) >= 0x100ULL ? 8 + CCC_private_log2_4((x) >> 8)                    \
                         : CCC_private_log2_4(x))
/* @internal */
#    define CCC_private_log2_16(x)                                             \
        ((x) >= 0x10000ULL ? 16 + CCC_private_log2_8((x) >> 16)                \
                           : CCC_private_log2_8(x))
/* @internal */
#    define CCC_private_log2_32(x)                                             \
        ((x) >= 0x100000000ULL ? 32 + CCC_private_log2_16((x) >> 32)           \
                               : CCC_private_log2_16(x))
/* @internal */
#    define CCC_private_log2(x)                                                \
        ((x) == 0 ? 0 : CCC_private_log2_32((unsigned long long)(x)))

#endif /* __has_builtin(__builtin_stdc_leading_zeros) */

/*======================     Minimum Maximum   ==============================*/

/* @internal */
#define CCC_private_min(a, b)                                                  \
    (__extension__({                                                           \
        typeof(a) ccc_private_a = (a);                                         \
        typeof(b) ccc_private_b = (b);                                         \
        static_assert(                                                         \
            ((typeof(a))-1 < 0) == ((typeof(b))-1 < 0),                        \
            "CCC_min: Mixed signed/unsigned comparison is unsafe. Cast "       \
            "explicitly."                                                      \
        );                                                                     \
        ccc_private_a < ccc_private_b ? ccc_private_a : ccc_private_b;         \
    }))

/* @internal */
#define CCC_private_max(a, b)                                                  \
    (__extension__({                                                           \
        typeof(a) ccc_private_a = (a);                                         \
        typeof(b) ccc_private_b = (b);                                         \
        static_assert(                                                         \
            ((typeof(a))-1 < 0) == ((typeof(b))-1 < 0),                        \
            "CCC_max: Mixed signed/unsigned comparison is unsafe. Cast "       \
            "explicitly."                                                      \
        );                                                                     \
        ccc_private_a > ccc_private_b ? ccc_private_a : ccc_private_b;         \
    }))

/*======================     Leading Zeros     ==============================*/

#if __has_builtin(__builtin_stdc_leading_zeros)

/* @internal */
#    define CCC_private_count_leading_zeros(x)                                 \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0                                                 \
                ? (int)(sizeof(ccc_private_x) * CHAR_BIT)                      \
                : (int)__builtin_stdc_leading_zeros(ccc_private_x);            \
        }))

#elif __has_builtin(__builtin_clzg)

/* @internal */
#    define CCC_private_count_leading_zeros(x)                                 \
        __builtin_clzg(x, (int)(sizeof(x) * CHAR_BIT))

#elif __has_builtin(__builtin_clz) && __has_builtin(__builtin_clzl)            \
    && __has_builtin(__builtin_clzll)

/* @internal */
#    define CCC_private_count_leading_zeros(x)                                 \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * CHAR_BIT)       \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: __builtin_clz((unsigned int)ccc_private_x)  \
                        - (int)((sizeof(unsigned int) - sizeof(unsigned char)) \
                                * CHAR_BIT),                                   \
                    unsigned short: __builtin_clz((unsigned int)ccc_private_x) \
                        - (int)((sizeof(unsigned int)                          \
                                 - sizeof(unsigned short))                     \
                                * CHAR_BIT),                                   \
                    unsigned int: __builtin_clz((unsigned int)ccc_private_x),  \
                    unsigned long: __builtin_clzl(                             \
                                         (unsigned long)ccc_private_x          \
                    ),                                                         \
                    unsigned long long: __builtin_clzll(                       \
                                         (unsigned long long)ccc_private_x     \
                    )                                                          \
                                 );                                            \
        }))

#else /* PORTABLE FALLBACK */

/* @internal */
CCC_PRIVATE_INLINE int
CCC_private_count_leading_zeros_u32(uint32_t x) {
    if (x == 0) {
        return 32;
    }
    int n = 0;
    if ((x & 0xFFFF0000U) == 0) {
        n += 16;
        x <<= 16;
    }
    if ((x & 0xFF000000U) == 0) {
        n += 8;
        x <<= 8;
    }
    if ((x & 0xF0000000U) == 0) {
        n += 4;
        x <<= 4;
    }
    if ((x & 0xC0000000U) == 0) {
        n += 2;
        x <<= 2;
    }
    if ((x & 0x80000000U) == 0) {
        n += 1;
    }
    return n;
}

/* @internal */
CCC_PRIVATE_INLINE int
CCC_private_count_leading_zeros_u64(uint64_t x) {
    if (x == 0) {
        return 64;
    }
    int n = 0;
    if ((x & 0xFFFFFFFF00000000ULL) == 0) {
        n += 32;
        x <<= 32;
    }
    if ((x & 0xFFFF000000000000ULL) == 0) {
        n += 16;
        x <<= 16;
    }
    if ((x & 0xFF00000000000000ULL) == 0) {
        n += 8;
        x <<= 8;
    }
    if ((x & 0xF000000000000000ULL) == 0) {
        n += 4;
        x <<= 4;
    }
    if ((x & 0xC000000000000000ULL) == 0) {
        n += 2;
        x <<= 2;
    }
    if ((x & 0x8000000000000000ULL) == 0) {
        n += 1;
    }
    return n;
}

/* @internal */
#    define CCC_private_count_leading_zeros(x)                                 \
        _Generic(                                                              \
            (x),                                                               \
            unsigned char: CCC_private_count_leading_zeros_u32(x)              \
                - (int)((sizeof(uint32_t) - sizeof(unsigned char))             \
                        * CHAR_BIT),                                           \
            unsigned short: CCC_private_count_leading_zeros_u32(x)             \
                - (int)((sizeof(uint32_t) - sizeof(unsigned short))            \
                        * CHAR_BIT),                                           \
            unsigned int: sizeof(int) == sizeof(uint64_t)                      \
                ? CCC_private_count_leading_zeros_u64(x)                       \
                : CCC_private_count_leading_zeros_u32(x),                      \
            unsigned long: sizeof(long) == sizeof(uint64_t)                    \
                ? CCC_private_count_leading_zeros_u64(x)                       \
                : CCC_private_count_leading_zeros_u32(x),                      \
            unsigned long long: CCC_private_count_leading_zeros_u64(x)         \
        )

#endif /* __has_builtin(__builtin_stdc_leading_zeros) */

/*======================     Trailing Zeros     =============================*/

#if __has_builtin(__builtin_stdc_trailing_zeros)

/* @internal */
#    define CCC_private_count_trailing_zeros(x)                                \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0                                                 \
                ? (int)(sizeof(ccc_private_x) * CHAR_BIT)                      \
                : (int)__builtin_stdc_trailing_zeros(ccc_private_x);           \
        }))

#elif __has_builtin(__builtin_ctzg)

/* @internal */
#    define CCC_private_count_trailing_zeros(x)                                \
        __builtin_ctzg(x, (int)(sizeof(x) * CHAR_BIT))

#elif __has_builtin(__builtin_ctz) && __has_builtin(__builtin_ctzl)            \
    && __has_builtin(__builtin_ctzll)

/* @internal */
#    define CCC_private_count_trailing_zeros(x)                                \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * CHAR_BIT)       \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: __builtin_ctz((unsigned int)ccc_private_x), \
                    unsigned short: __builtin_ctz(                             \
                                         (unsigned int)ccc_private_x           \
                    ),                                                         \
                    unsigned int: __builtin_ctz((unsigned int)ccc_private_x),  \
                    unsigned long: __builtin_ctzl(                             \
                                         (unsigned long)ccc_private_x          \
                    ),                                                         \
                    unsigned long long: __builtin_ctzll(                       \
                                         (unsigned long long)ccc_private_x     \
                    )                                                          \
                                 );                                            \
        }))

#else /* PORTABLE FALLBACK */

/* @internal */
CCC_PRIVATE_INLINE int
CCC_private_count_trailing_zeros_u32(uint32_t x) {
    if (x == 0) {
        return 32;
    }
    int n = 0;
    if ((x & 0x0000FFFFU) == 0) {
        n += 16;
        x >>= 16;
    }
    if ((x & 0x000000FFU) == 0) {
        n += 8;
        x >>= 8;
    }
    if ((x & 0x0000000FU) == 0) {
        n += 4;
        x >>= 4;
    }
    if ((x & 0x00000003U) == 0) {
        n += 2;
        x >>= 2;
    }
    if ((x & 0x00000001U) == 0) {
        n += 1;
    }
    return n;
}

/* @internal */
CCC_PRIVATE_INLINE int
CCC_private_count_trailing_zeros_u64(uint64_t x) {
    if (x == 0) {
        return 64;
    }
    int n = 0;
    if ((x & 0x00000000FFFFFFFFULL) == 0) {
        n += 32;
        x >>= 32;
    }
    if ((x & 0x000000000000FFFFULL) == 0) {
        n += 16;
        x >>= 16;
    }
    if ((x & 0x00000000000000FFULL) == 0) {
        n += 8;
        x >>= 8;
    }
    if ((x & 0x000000000000000FULL) == 0) {
        n += 4;
        x >>= 4;
    }
    if ((x & 0x0000000000000003ULL) == 0) {
        n += 2;
        x >>= 2;
    }
    if ((x & 0x0000000000000001ULL) == 0) {
        n += 1;
    }
    return n;
}

/* @internal */
#    define CCC_private_count_trailing_zeros(x)                                \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * CHAR_BIT)       \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: CCC_private_count_trailing_zeros_u32(       \
                                         ccc_private_x                         \
                    ),                                                         \
                    unsigned short: CCC_private_count_trailing_zeros_u32(      \
                                         ccc_private_x                         \
                    ),                                                         \
                    unsigned int: sizeof(int) == sizeof(uint64_t)              \
                        ? CCC_private_count_trailing_zeros_u64(ccc_private_x)  \
                        : CCC_private_count_trailing_zeros_u32(ccc_private_x), \
                    unsigned long: sizeof(long) == sizeof(uint64_t)            \
                        ? CCC_private_count_trailing_zeros_u64(ccc_private_x)  \
                        : CCC_private_count_trailing_zeros_u32(ccc_private_x), \
                    unsigned long long: CCC_private_count_trailing_zeros_u64(  \
                                         ccc_private_x                         \
                    )                                                          \
                                 );                                            \
        }))

#endif /* __has_builtin(__builtin_stdc_trailing_zeros) */

/*==========================     Popcount      ==============================*/

#if __has_builtin(__builtin_stdc_count_ones)

/* @internal */
#    define CCC_private_popcount(x) ((int)__builtin_stdc_count_ones(x))

#elif __has_builtin(__builtin_popcountg)

/* @internal */
#    define CCC_private_popcount(x) ((int)__builtin_popcountg(x))

#elif __has_builtin(__builtin_popcount) && __has_builtin(__builtin_popcountl)  \
    && __has_builtin(__builtin_popcountll)

/* @internal */
#    define CCC_private_popcount(x)                                            \
        _Generic(                                                              \
            (x),                                                               \
            unsigned char: __builtin_popcount(x),                              \
            unsigned short: __builtin_popcount(x),                             \
            unsigned int: __builtin_popcount(x),                               \
            unsigned long: __builtin_popcountl(x),                             \
            unsigned long long: __builtin_popcountll(x)                        \
        )

#else /* PORTABLE FALLBACK */

/* @internal */
CCC_PRIVATE_INLINE int
CCC_private_popcount_u32(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
    x = (x + (x >> 4)) & 0x0F0F0F0FU;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return (int)(x & 0x0000003FU);
}

/* @internal */
CCC_PRIVATE_INLINE int
CCC_private_popcount_u64(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32);
    return (int)(x & 0x0000007FULL);
}

/* @internal */
#    define CCC_private_popcount(x)                                            \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            _Generic(                                                          \
                (ccc_private_x),                                               \
                unsigned char: CCC_private_popcount_u32(ccc_private_x),        \
                unsigned short: CCC_private_popcount_u32(ccc_private_x),       \
                unsigned int: sizeof(int) == sizeof(uint64_t)                  \
                    ? CCC_private_popcount_u64(ccc_private_x)                  \
                    : CCC_private_popcount_u32(ccc_private_x),                 \
                unsigned long: sizeof(long) == sizeof(uint64_t)                \
                    ? CCC_private_popcount_u64(ccc_private_x)                  \
                    : CCC_private_popcount_u32(ccc_private_x),                 \
                unsigned long long: CCC_private_popcount_u64(ccc_private_x)    \
            );                                                                 \
        }))
#endif /* __has_builtin(__builtin_stdc_count_ones) */

/*==========================     Bit Ceiling   ==============================*/

#if __has_builtin(__builtin_stdc_bit_ceil)

/* @internal */
#    define CCC_private_bit_ceiling(x)                                         \
        (__extension__({                                                       \
            typeof(x) const ccc_private_ceiling_x = (x);                       \
            ccc_private_ceiling_x                                              \
                    > ((typeof(x))1 << ((sizeof(x) * CHAR_BIT) - 1))           \
                ? (typeof(x))0                                                 \
                : (typeof(x))__builtin_stdc_bit_ceil(ccc_private_ceiling_x);   \
        }))

#else

/* @internal */
#    define CCC_private_bit_ceiling(x)                                         \
        (__extension__({                                                       \
            typeof(x) const ccc_private_ceiling_x = (x);                       \
            ccc_private_ceiling_x <= 1 ? (typeof(x))1 : (__extension__({       \
                int const ccc_private_shifts                                   \
                    = CCC_private_count_leading_zeros(                         \
                        (typeof(x))(ccc_private_ceiling_x - 1)                 \
                    );                                                         \
                ccc_private_shifts == 0                                        \
                    ? (typeof(x))0                                             \
                    : (typeof(x))(((typeof(x))~((typeof(x))0)                  \
                                   >> ccc_private_shifts)                      \
                                  + 1);                                        \
            }));                                                               \
        }))

#endif /* __has_builtin(__builtin_stdc_bit_ceil) */

#endif /* CCC_PRIVATE_COMPILER_UTILITIES */
