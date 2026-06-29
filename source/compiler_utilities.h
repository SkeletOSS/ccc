#ifndef CCC_COMPILER_UTILITIES_H
#define CCC_COMPILER_UTILITIES_H

/* C23 provided headers. */
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __has_builtin
#    define __has_builtin(x) 0
#endif /* __has_builtin */

#if defined(__GNUC__) || defined(__clang__)
#    define ccc_inline [[gnu::always_inline]] static inline
#else
#    define ccc_inline static inline
#endif /* defined(__GNUC__) || defined(__clang__) */

#define ccc_min(a, b)                                                          \
    (__extension__({                                                           \
        typeof(a) ccc_private_a = (a);                                         \
        typeof(b) ccc_private_b = (b);                                         \
        static_assert(                                                         \
            ((typeof(a))-1 < 0) == ((typeof(b))-1 < 0),                        \
            "ccc_min: Mixed signed/unsigned comparison is unsafe. Cast "       \
            "explicitly."                                                      \
        );                                                                     \
        ccc_private_a < ccc_private_b ? ccc_private_a : ccc_private_b;         \
    }))

#define ccc_max(a, b)                                                          \
    (__extension__({                                                           \
        typeof(a) ccc_private_a = (a);                                         \
        typeof(b) ccc_private_b = (b);                                         \
        static_assert(                                                         \
            ((typeof(a))-1 < 0) == ((typeof(b))-1 < 0),                        \
            "ccc_min: Mixed signed/unsigned comparison is unsafe. Cast "       \
            "explicitly."                                                      \
        );                                                                     \
        ccc_private_a > ccc_private_b ? ccc_private_a : ccc_private_b;         \
    }))

#if __has_builtin(__builtin_stdc_leading_zeros)

#    define ccc_count_leading_zeros(x)                                         \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0                                                 \
                ? (int)(sizeof(ccc_private_x) * CHAR_BIT)                      \
                : (int)__builtin_stdc_leading_zeros(ccc_private_x);            \
        }))

#elif __has_builtin(__builtin_clzg)

#    define ccc_count_leading_zeros(x)                                         \
        __builtin_clzg(x, (int)(sizeof(x) * CHAR_BIT))

#elif __has_builtin(__builtin_clz) && __has_builtin(__builtin_clzl)            \
    && __has_builtin(__builtin_clzll)
#    define ccc_count_leading_zeros(x)                                         \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * CHAR_BIT)       \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: __builtin_clz(ccc_private_x)                \
                        - (int)((sizeof(unsigned int) - sizeof(unsigned char)) \
                                * 8),                                          \
                    unsigned short: __builtin_clz(ccc_private_x)               \
                        - (int)((sizeof(unsigned int) - sizeof(unsigned char)) \
                                * 8),                                          \
                    unsigned int: __builtin_clz(ccc_private_x),                \
                    unsigned long: __builtin_clzl(ccc_private_x),              \
                    unsigned long long: __builtin_clzll(ccc_private_x)         \
                                 );                                            \
        }))

#else /* PORTABLE FALLBACK COUNTING */

ccc_inline int
ccc_count_leading_zeros_u32(uint32_t x) {
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

ccc_inline int
ccc_count_leading_zeros_u64(uint64_t x) {
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
#    define ccc_count_leading_zeros(x)                                         \
        _Generic(                                                              \
            (x),                                                               \
            unsigned char: ccc_count_leading_zeros_u32(x)                      \
                - (int)((sizeof(unsigned int) - sizeof(unsigned char)) * 8),   \
            unsigned short: ccc_count_leading_zeros_u32(x)                     \
                - (int)((sizeof(unsigned int) - sizeof(unsigned short)) * 8),  \
            unsigned int: sizeof(int) == 8 ? ccc_count_leading_zeros_u64(x)    \
                                           : ccc_count_leading_zeros_u32(x),   \
            unsigned long: sizeof(long) == 8 ? ccc_count_leading_zeros_u64(x)  \
                                             : ccc_count_leading_zeros_u32(x), \
            unsigned long long: ccc_count_leading_zeros_u64(x)                 \
        )
#endif /* __has_builtin(__builtin_stdc_leading_zeros) */

#if __has_builtin(__builtin_stdc_trailing_zeros)
#    define ccc_count_trailing_zeros(x)                                        \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0                                                 \
                ? (int)(sizeof(ccc_private_x) * 8)                             \
                : (int)__builtin_stdc_trailing_zeros(ccc_private_x);           \
        }))
#elif __has_builtin(__builtin_ctzg)
#    define ccc_count_trailing_zeros(x)                                        \
        __builtin_ctzg(x, (int)(sizeof(x) * CHAR_BIT))
#elif __has_builtin(__builtin_ctz) && __has_builtin(__builtin_ctzl)            \
    && __has_builtin(__builtin_ctzll)
#    define ccc_count_trailing_zeros(x)                                        \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * 8)              \
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

ccc_inline int
ccc_count_trailing_zeros_u32(uint32_t x) {
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

ccc_inline int
ccc_count_trailing_zeros_u64(uint64_t x) {
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
#    define ccc_count_trailing_zeros(x)                                        \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            ccc_private_x == 0 ? (int)(sizeof(ccc_private_x) * 8)              \
                               : (int)_Generic(                                \
                                     (ccc_private_x),                          \
                    unsigned char: ccc_count_trailing_zeros_u32(               \
                                         ccc_private_x                         \
                    ),                                                         \
                    unsigned short: ccc_count_trailing_zeros_u32(              \
                                         ccc_private_x                         \
                    ),                                                         \
                    unsigned int: sizeof(int) == 8                             \
                        ? ccc_count_trailing_zeros_u64(ccc_private_x)          \
                        : ccc_count_trailing_zeros_u32(ccc_private_x),         \
                    unsigned long: sizeof(long) == 8                           \
                        ? ccc_count_trailing_zeros_u64(ccc_private_x)          \
                        : ccc_count_trailing_zeros_u32(ccc_private_x),         \
                    unsigned long long: ccc_count_trailing_zeros_u64(          \
                                         ccc_private_x                         \
                    )                                                          \
                                 );                                            \
        }))
#endif /* __has_builtin(__builtin_stdc_trailing_zeros) */

#if __has_builtin(__builtin_stdc_count_ones)
#    define ccc_popcount(x) ((int)__builtin_stdc_count_ones(x))
#elif __has_builtin(__builtin_popcountg)
#    define ccc_popcount(x) ((int)__builtin_popcountg(x))
#elif __has_builtin(__builtin_popcount) && __has_builtin(__builtin_popcountl)  \
    && __has_builtin(__builtin_popcountll)

#    define ccc_popcount(x)                                                    \
        _Generic(                                                              \
            (x),                                                               \
            unsigned char: __builtin_popcount(x),                              \
            unsigned short: __builtin_popcount(x),                             \
            unsigned int: __builtin_popcount(x),                               \
            unsigned long: __builtin_popcountl(x),                             \
            unsigned long long: __builtin_popcountll(x)                        \
        )

#else /* PORTABLE FALLBACK COUNTING */

ccc_inline int
ccc_popcount_u32(uint32_t x) {
    x = x - ((x >> 1) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
    x = (x + (x >> 4)) & 0x0F0F0F0FU;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return (int)(x & 0x0000003FU);
}

ccc_inline int
ccc_popcount_u64(uint64_t x) {
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    x = x + (x >> 8);
    x = x + (x >> 16);
    x = x + (x >> 32);
    return (int)(x & 0x0000007FULL);
}

#    define ccc_popcount(x)                                                    \
        (__extension__({                                                       \
            typeof(x) ccc_private_x = (x);                                     \
            _Generic(                                                          \
                (ccc_private_x),                                               \
                unsigned char: ccc_popcount_u32(ccc_private_x),                \
                unsigned short: ccc_popcount_u32(ccc_private_x),               \
                unsigned int: sizeof(int) == 8                                 \
                    ? ccc_popcount_u64(ccc_private_x)                          \
                    : ccc_popcount_u32(ccc_private_x),                         \
                unsigned long: sizeof(long) == 8                               \
                    ? ccc_popcount_u64(ccc_private_x)                          \
                    : ccc_popcount_u32(ccc_private_x),                         \
                unsigned long long: ccc_popcount_u64(ccc_private_x)            \
            );                                                                 \
        }))
#endif /* __has_builtin(__builtin_stdc_count_ones) */

#endif /* CCC_COMPILER_UTILITIES_H */
