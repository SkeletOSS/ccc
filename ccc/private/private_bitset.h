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
#ifndef CCC_PRIVATE_BITSET
#define CCC_PRIVATE_BITSET

/** @cond */
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
/** @endcond */

#include "../types.h"
#include "configuration.h"

/** @internal A Bitset is a contiguous array of fixed size integers. These aid
in cache friendly storage and operations.

By default a bit set is initialized with size equal to capacity but the user may
select to initialize a 0 sized bit set with non-zero capacity for pushing bits
back dynamically. */
struct CCC_Bitset {
    /** The array of bit blocks, a platform defined standard bit width. */
    unsigned *blocks;
    /** The number of active bits in the set available for reads and writes. */
    size_t count;
    /** The number of bits capable of being tracked in the bit block array. */
    size_t capacity;
};

enum : size_t {
    /** @internal The number of bits in a bit block. In sync with set type. */
    CCC_PRIVATE_BITSET_BLOCK_BITS
        = (sizeof(*(struct CCC_Bitset){}.blocks) * CHAR_BIT),
};

/*=========================     Private Interface   =========================*/

CCC_Result
CCC_private_bitset_reserve(struct CCC_Bitset *, size_t, CCC_Allocator const *);
CCC_Tribool CCC_private_bitset_set(struct CCC_Bitset *, size_t, CCC_Tribool);

/*================================     Macros     ===========================*/

/** @internal Returns the number of blocks needed to support a given capacity
of bits. Assumes the given capacity is greater than 0. Classic div round up. */
#define CCC_private_bitset_block_count(private_bit_capacity)                   \
    (((private_bit_capacity) + (CCC_PRIVATE_BITSET_BLOCK_BITS - 1))            \
     / CCC_PRIVATE_BITSET_BLOCK_BITS)

/** @internal Returns the number of bytes needed for the required blocks. */
#define CCC_private_bitset_block_bytes(private_bit_capacity)                   \
    (sizeof(*(struct CCC_Bitset){}.blocks)                                     \
     * CCC_private_bitset_block_count(private_bit_capacity))

/** @internal */
#define CCC_private_bitset_default()                                           \
    (struct CCC_Bitset) {                                                      \
    }

/** @internal NOLINTNEXTLINE */
#define CCC_private_bitset_non_CCC_private_bitset_default_size(                \
    private_cap, ...                                                           \
)                                                                              \
    __VA_ARGS__
/** @internal */
#define CCC_private_bitset_default_size(private_cap, ...) private_cap
/** @internal */
#define CCC_private_bitset_optional_size(private_cap, ...)                     \
    __VA_OPT__(CCC_private_bitset_non_)                                        \
    ##CCC_private_bitset_default_size(private_cap, __VA_ARGS__)

/** @internal This initializer must be runtime only because the possibility the
bitset comes from an uninitialized heap. */
#define CCC_private_bitset_for(                                                \
    private_cap, private_count, private_bitblock_pointer                       \
)                                                                              \
    (struct { struct CCC_Bitset private; }){(__extension__({                   \
        struct CCC_Bitset private_bitset = {                                   \
            .blocks = memset(                                                  \
                (private_bitblock_pointer),                                    \
                0,                                                             \
                CCC_private_bitset_block_bytes(private_cap)                    \
            ),                                                                 \
            .count = (private_count),                                          \
            .capacity = (private_cap),                                         \
        };                                                                     \
        private_bitset;                                                        \
    }))}.private

/** @internal Determine if user wants capacity different than count. Then pass
to inline function for bit set construction. */
#define CCC_private_bitset_from(                                               \
    private_allocator,                                                         \
    private_start_index,                                                       \
    private_count,                                                             \
    private_on_char,                                                           \
    private_string,                                                            \
    ...                                                                        \
)                                                                              \
    (struct { struct CCC_Bitset private; }){(__extension__({                   \
        struct CCC_Bitset private_bitset = CCC_private_bitset_default();       \
        size_t const private_cap                                               \
            = CCC_private_bitset_optional_size((private_count), __VA_ARGS__);  \
        size_t private_index = (private_start_index);                          \
        if (CCC_private_bitset_reserve(                                        \
                &private_bitset,                                               \
                private_cap < private_count ? private_count : private_cap,     \
                &private_allocator                                             \
            )                                                                  \
            == CCC_RESULT_OK) {                                                \
            private_bitset.count = private_count;                              \
            while (private_index < private_count                               \
                   && private_string[private_index]) {                         \
                (void)CCC_private_bitset_set(                                  \
                    &private_bitset,                                           \
                    private_index,                                             \
                    private_string[private_index] == private_on_char           \
                );                                                             \
                ++private_index;                                               \
            }                                                                  \
            private_bitset.count = private_index;                              \
        }                                                                      \
        private_bitset;                                                        \
    }))}.private

/** @internal. */
#define CCC_private_bitset_with_capacity(private_allocate, private_cap, ...)   \
    (struct { struct CCC_Bitset private; }){(__extension__({                   \
        struct CCC_Bitset private_bitset = CCC_private_bitset_default();       \
        size_t const private_count                                             \
            = CCC_private_bitset_optional_size((private_cap), __VA_ARGS__);    \
        if (CCC_private_bitset_reserve(                                        \
                &private_bitset, private_cap, &private_allocate                \
            )                                                                  \
            == CCC_RESULT_OK) {                                                \
            private_bitset.count = private_count;                              \
        }                                                                      \
        private_bitset;                                                        \
    }))}.private

/** @internal Clang is more forgiving with what qualifies as a constant
expression for both constructing compound literals and using static asserts.
The static asserts are so helpful that it is worth every effort to give them
to the user. GCC is not so forgiving. */
#if defined(__clang__) || defined(__llvm__)
/** @internal Allocates a compound literal bit block array in the scope at which
the macro is used. However, the optional parameter supports storage duration
specifiers which is a feature of C23. Not all compilers support this yet. */
#    define CCC_private_bitset_count_check_storage_for(                        \
        private_count,                                                         \
        private_bit_compound_literal,                                          \
        private_optional_storage_specifier...                                  \
    )                                                                          \
        (private_optional_storage_specifier struct {                           \
            static_assert(                                                     \
                sizeof(private_bit_compound_literal),                          \
                "Specify non-zero capacity of bits."                           \
            );                                                                 \
            static_assert(                                                     \
                sizeof(*(private_bit_compound_literal)) == sizeof(CCC_Bit),    \
                "CCC_bitset_storage_for and CCC_bitset_with_storage only "     \
                "accept "                                                      \
                "a (CCC_Bit[N]){} compound literal array as an argument. Do "  \
                "not "                                                         \
                "use CCC_bitset_storage_for as an argument to "                \
                "CCC_bitset_with_storage."                                     \
            );                                                                 \
            static_assert(                                                     \
                (private_count) <= sizeof(private_bit_compound_literal),       \
                "Bit count is less than or equal to capacity."                 \
            );                                                                 \
            typeof(*(struct CCC_Bitset){}.blocks) private                      \
                [CCC_private_bitset_block_count(                               \
                    sizeof(private_bit_compound_literal)                       \
                )];                                                            \
        }){}                                                                   \
            .private

/** @internal */
#    define CCC_private_bitset_storage_for(                                    \
        private_bit_compound_literal, private_optional_storage_specifier...    \
    )                                                                          \
        CCC_private_bitset_count_check_storage_for(                            \
            0,                                                                 \
            private_bit_compound_literal,                                      \
            private_optional_storage_specifier                                 \
        )

#else
/** @internal */
#    define CCC_private_bitset_count_check_storage_for(                        \
        private_count,                                                         \
        private_bit_compound_literal,                                          \
        private_optional_storage_specifier...                                  \
    )                                                                          \
        (typeof (                                                              \
            *(struct CCC_Bitset){}.blocks                                      \
        )[CCC_private_bitset_block_count(                                      \
            sizeof(private_bit_compound_literal)                               \
        )]) {                                                                  \
        }

/** @internal */
#    define CCC_private_bitset_storage_for(                                    \
        private_bit_compound_literal, private_optional_storage_specifier...    \
    )                                                                          \
        CCC_private_bitset_count_check_storage_for(                            \
            0,                                                                 \
            private_bit_compound_literal,                                      \
            private_optional_storage_specifier                                 \
        )
#endif

/** @internal */
#define CCC_private_bitset_with_storage(                                       \
    private_count,                                                             \
    private_compound_literal_array,                                            \
    private_optional_storage_specifier...                                      \
)                                                                              \
    (struct CCC_Bitset) {                                                      \
        .blocks = CCC_private_bitset_count_check_storage_for(                  \
            private_count,                                                     \
            private_compound_literal_array,                                    \
            private_optional_storage_specifier                                 \
        ),                                                                     \
        .count = (private_count),                                              \
        .capacity = CCC_private_bitset_block_count(                            \
                        sizeof(private_compound_literal_array)                 \
                    )                                                          \
                  * sizeof(*(struct CCC_Bitset){}.blocks) * CHAR_BIT,          \
    }

#endif /* CCC_PRIVATE_BITSET */
