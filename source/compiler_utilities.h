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
@brief The Compiler Utilities Available to All Containers

This file is an internal private header that allows containers to use low level
compiler built-in functionality or extremely common helper functions. It is not
user facing and will not be exported as an interface. This file includes
detection of compiler built-ins such as bit utilities. It attempts to use bit
utilities from the new __builtin_stdc_* family when available as these are at
the cutting edge of c23's new stdbit.h standard. However, sane fallbacks are
always implemented as well. See the documented interface for all available
macros and functions. */
#ifndef CCC_COMPILER_UTILITIES_H
#define CCC_COMPILER_UTILITIES_H

/** @cond */
#include <limits.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
/** @endcond */

/** @internal
@name Macro Preprocessor Definitions
Compiler built ins for code manipulation. */
/**@{*/

/** @internal
@brief Force a function to be static inline, if available via compiler
directives. */
#define CCC_INLINE CCC_PRIVATE_INLINE

/** @internal
@brief Mark an expression as likely for compiler optimizations.
@param[in] expression the expression in code to mark. */
#define CCC_likely(expression) CCC_private_likely(expression)

/** @internal
@brief Mark an expression as unlikely for compiler optimizations.
@param[in] expression the expression in code to mark. */
#define CCC_unlikely(expression) CCC_private_unlikely(expression)

/**@}*/

/** @name Arithmetic
Type safe arithmetic functions. */
/**@{*/

/** @internal
@brief Round an integer up to a specified alignment boundary at compile time.
@param[in] integer the integer value to round up.
@param[in] alignment the alignment boundary that must be a power of two.
@return The unchecked rounded up value.
@warning Prefer CCC_roundup(integer, alignment), if possible. This version is
intended to be used at compile time but lacks the safety and widening guarantees
of the other versions.

This is intended for use in compile time definitions and static asserts. */
#define CCC_comptime_roundup(integer, alignment)                               \
    CCC_private_comptime_roundup(integer, alignment)

/** @internal
@brief Round an integer up to a specified alignment boundary safely
evaluating arguments once.
@param[in] integer the integer value to round up.
@param[in] alignment the alignment boundary that must be a power of two.
@return The rounded-up value, evaluated in the wider type of the two input
arguments.
@note Argument promotion to the wider type ensures that smaller integer types
are not accidentally truncated during alignment arithmetic. */
#define CCC_roundup(integer, alignment) CCC_private_roundup(integer, alignment)

/** @internal
@brief Check for overflow while rounding an integer up to an alignment
boundary using C23 checked arithmetic.
@param[out] result_pointer pointer to a variable where the rounded result will
be stored.
@param[in] integer the integer value to round up.
@param[in] alignment the alignment boundary (must be a power of two).
@return true if an overflow occurred during rounding or if the result cannot
fit into the type pointed to by result_pointer; false otherwise. */
#define CCC_checked_roundup(result_pointer, integer, alignment)                \
    CCC_private_checked_roundup(result_pointer, integer, alignment)

/**@}*/

/** @name Bit Manipulation
Type safe bit manipulation functions. */
/**@{*/

/** @internal
@brief Count the number of leading zero bits in an unsigned integer.
@param[in] integer the unsigned integer value to inspect.
@return The int count of leading zeros. If the input is zero, the bit width
count of the input type is returned.
@note Behavior is well-defined for input 0. */
#define CCC_count_leading_zeros(integer)                                       \
    CCC_private_count_leading_zeros(integer)

/** @internal
@brief Count the number of trailing zero bits in an unsigned integer.
@param[in] integer the unsigned integer value to inspect.
@return The int count of trailing zeros. If the input is zero, the bit width
count of the input type is returned.
@note Behavior is well-defined for input 0. */
#define CCC_count_trailing_zeros(integer)                                      \
    CCC_private_count_trailing_zeros(integer)

/** @internal
@brief Count the total number of set 1 bits in an unsigned integer.
@param[in] integer the unsigned integer value to inspect.
@return The int count of bits set to the value 1. */
#define CCC_popcount(integer) CCC_private_popcount(integer)

/** @internal
@brief Find the next closest power of 2 ceiling for integer.
@param[in] integer the unsigned integer value to raise to its ceiling.
@return The smallest power of two greater than or equal to the given value, of
the same type as the input argument. If the next power of two overflows the
maximum representable value of the input type, 0 is returned to indicate
failure.
@note Inputting 0 returns 1, as 1 (2^0) is the first power of two. Because 0
is not a valid power of two, it safely serves as an overflow sentinel. */
#define CCC_bit_ceiling(integer) CCC_private_bit_ceiling(integer)

/**@}*/

/** @name Comparison
Type safe comparison functions. */
/**@{*/

/** @internal
@brief Evaluate the minimum of two values, safely ensuring that both types
match in signedness. Each argument is evaluated once to avoid side-effects.
@param[in] left the first value to compare.
@param[in] right the second value to compare.
@return The minimum between left and right. */
#define CCC_min(left, right) CCC_private_min(left, right)

/** @internal
@brief Evaluate the maximum of two values, safely ensuring that both types
match in signedness. Each argument is evaluated once to avoid side-effects.
@param[in] left the first value to compare.
@param[in] right the second value to compare.
@return The maximum between left and right. */
#define CCC_max(left, right) CCC_private_max(left, right)

/**@}*/

/*==========================  Implementations   =============================*/

#ifndef __has_builtin
#    define __has_builtin(x) 0
#endif /* __has_builtin */

#if defined(__GNUC__) || defined(__clang__)
#    define CCC_PRIVATE_INLINE [[gnu::always_inline]] static inline
#else
#    define CCC_PRIVATE_INLINE static inline
#endif /* defined(__GNUC__) || defined(__clang__) */

/** Maybe the compiler can give us better performance in key paths. */
#if __has_builtin(__builtin_expect)
#    define CCC_private_unlikely(expr) __builtin_expect(!!(expr), 0)
#    define CCC_private_likely(expr) __builtin_expect(!!(expr), 1)
#else /* !__has_builtin(__builtin_expect) */
#    define CCC_private_unlikely(expr) expr
#    define CCC_private_likely(expr) expr
#endif /* __has_builtin(__builtin_expect) */

#define CCC_private_comptime_roundup(integer, alignment)                       \
    ((typeof(integer))(((typeof(integer))(integer)                             \
                        + ((typeof(integer))(alignment) - 1))                  \
                       & ~((typeof(integer))(alignment) - 1)))

#define CCC_private_roundup(integer, alignment)                                        \
    (__extension__({                                                                   \
        typedef typeof(sizeof(integer) >= sizeof(alignment) ? (integer) : (alignment)) \
            ccc_private_max_width_type;                                                \
        ccc_private_max_width_type ccc_private_integer = (integer);                    \
        ccc_private_max_width_type ccc_private_alignment                               \
            = (ccc_private_max_width_type)(alignment);                                 \
        (ccc_private_max_width_type)(                                                  \
            (ccc_private_integer + (ccc_private_alignment - 1))                        \
            & ~(ccc_private_alignment - 1)                                             \
        );                                                                             \
    }))

#define CCC_private_checked_roundup(result_pointer, integer, alignment)                \
    (__extension__({                                                                   \
        /*NOLINTBEGIN(bugprone-assignment-in-if-condition)*/                           \
        typedef typeof(sizeof(integer) >= sizeof(alignment) ? (integer) : (alignment)) \
            ccc_private_max_width_type;                                                \
        ccc_private_max_width_type ccc_private_integer = (integer);                    \
        ccc_private_max_width_type ccc_private_alignment                               \
            = (ccc_private_max_width_type)(alignment);                                 \
        typeof(result_pointer) ccc_private_result_pointer = (result_pointer);          \
        bool ccc_private_has_overflow = false;                                         \
        if (ckd_add(                                                                   \
                ccc_private_result_pointer,                                            \
                ccc_private_integer,                                                   \
                ccc_private_alignment - 1                                              \
            )) {                                                                       \
            ccc_private_has_overflow = true;                                           \
        } else {                                                                       \
            *ccc_private_result_pointer &= ~(ccc_private_alignment - 1);               \
        }                                                                              \
        ccc_private_has_overflow;                                                      \
        /*NOLINTEND(bugprone-assignment-in-if-condition)*/                             \
    }))

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

#if __has_builtin(__builtin_stdc_leading_zeros)

#    define CCC_private_count_leading_zeros(x)                                 \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0                                                 \
                ? (int)(sizeof(ccc_private_x) * CHAR_BIT)                      \
                : (int)__builtin_stdc_leading_zeros(ccc_private_x);            \
        }))

#elif __has_builtin(__builtin_clzg)

#    define CCC_private_count_leading_zeros(x)                                 \
        __builtin_clzg(x, (int)(sizeof(x) * CHAR_BIT))

#elif __has_builtin(__builtin_clz) && __has_builtin(__builtin_clzl)            \
    && __has_builtin(__builtin_clzll)

#    define CCC_private_count_leading_zeros(x)                                 \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * CHAR_BIT)       \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: __builtin_clz(ccc_private_x)                \
                        - (int)((sizeof(unsigned int) - sizeof(unsigned char)) \
                                * CHAR_BIT),                                   \
                    unsigned short: __builtin_clz(ccc_private_x)               \
                        - (int)((sizeof(unsigned int)                          \
                                 - sizeof(unsigned short))                     \
                                * CHAR_BIT),                                   \
                    unsigned int: __builtin_clz(ccc_private_x),                \
                    unsigned long: __builtin_clzl(ccc_private_x),              \
                    unsigned long long: __builtin_clzll(ccc_private_x)         \
                                 );                                            \
        }))

#else /* PORTABLE FALLBACK COUNTING */

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

#if __has_builtin(__builtin_stdc_trailing_zeros)

#    define CCC_private_count_trailing_zeros(x)                                \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0                                                 \
                ? (int)(sizeof(ccc_private_x) * CHAR_BIT)                      \
                : (int)__builtin_stdc_trailing_zeros(ccc_private_x);           \
        }))

#elif __has_builtin(__builtin_ctzg)

#    define CCC_private_count_trailing_zeros(x)                                \
        __builtin_ctzg(x, (int)(sizeof(x) * CHAR_BIT))

#elif __has_builtin(__builtin_ctz) && __has_builtin(__builtin_ctzl)            \
    && __has_builtin(__builtin_ctzll)

#    define CCC_private_count_trailing_zeros(x)                                \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * CHAR_BIT)       \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: __builtin_ctz(ccc_private_x),               \
                    unsigned short: __builtin_ctz(ccc_private_x),              \
                    unsigned int: __builtin_ctz(ccc_private_x),                \
                    unsigned long: __builtin_ctzl(ccc_private_x),              \
                    unsigned long long: __builtin_ctzll(ccc_private_x)         \
                                 );                                            \
        }))

#else /* PORTABLE FALLBACK COUNTING */

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

#if __has_builtin(__builtin_stdc_count_ones)

#    define CCC_private_popcount(x) ((int)__builtin_stdc_count_ones(x))

#elif __has_builtin(__builtin_popcountg)

#    define CCC_private_popcount(x) ((int)__builtin_popcountg(x))

#elif __has_builtin(__builtin_popcount) && __has_builtin(__builtin_popcountl)  \
    && __has_builtin(__builtin_popcountll)

#    define CCC_private_popcount(x)                                            \
        _Generic(                                                              \
            (x),                                                               \
            unsigned char: __builtin_popcount(x),                              \
            unsigned short: __builtin_popcount(x),                             \
            unsigned int: __builtin_popcount(x),                               \
            unsigned long: __builtin_popcountl(x),                             \
            unsigned long long: __builtin_popcountll(x)                        \
        )

#else /* PORTABLE FALLBACK COUNTING */

CCC_PRIVATE_INLINE int
CCC_private_popcount_u32(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
    x = (x + (x >> 4)) & 0x0F0F0F0FU;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return (int)(x & 0x0000003FU);
}

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

#if __has_builtin(__builtin_stdc_bit_ceil)

#    define CCC_private_bit_ceiling(x)                                         \
        (__extension__({                                                       \
            typeof(x) const ccc_private_ceiling_x = (x);                       \
            ccc_private_ceiling_x                                              \
                    > ((typeof(x))1 << ((sizeof(x) * CHAR_BIT) - 1))           \
                ? (typeof(x))0                                                 \
                : (typeof(x))__builtin_stdc_bit_ceil(ccc_private_ceiling_x);   \
        }))

#else

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

#endif

#endif /* CCC_COMPILER_UTILITIES_H */
