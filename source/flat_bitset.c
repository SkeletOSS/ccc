/** Copyright 2025 Alexander G. Lopez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file implements a Bit Set using blocks of platform defined integers
for speed and efficiency. The implementation aims for constant and linear time
operations at all times, specifically when implementing more complicated range
based operations over the set.

It is also important to avoid modulo and division operations whenever possible.
This is why much of the code revolves around obtaining indices by processing
entire blocks at a time, rather than using mathematical operations to
conceptually iterate over individual bits.

Finally, the code is able to unite most functions for finding zeros or ones
into a single function that accepts true or false as an additional argument.
This is because every search for a zero can be solved by bitwise inverting a
block and searching for a 1 instead. This elimination of identical functions
costs a single branch in the function and is worth it to avoid code duplication
and bug doubling. */
/** C23 provided headers. */
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/** CCC provided headers. */
#include "ccc/configuration.h" /* IWYU pragma: keep */
#include "ccc/flat_bitset.h"
#include "ccc/private/private_flat_bitset.h"
#include "ccc/types.h"
#include "source/compiler_utilities.h"

/*=========================   Type Declarations  ============================*/

/** @internal The type representing the word size used for each block of the
Flat_bitset. This type may vary depending on the platform in order to maximize
performance, so it is given a type. The implementation then should not concern
itself with the word size.

The implementation may assume that the block is a standard integer width on
the given platform that is compatible with all basic arithmetic and bitwise
operations. If SIMD is implemented then multiple blocks may be processed under
this same assumption. */
typedef typeof(*(struct CCC_Flat_bitset){}.blocks) Bit_block;

/** @internal Used frequently so call the builtin just once. */
enum : size_t {
    /** @internal Bytes of a bit block to help with byte calculations. */
    SIZEOF_BLOCK = sizeof(Bit_block),
};

/** @internal Various constants to support bit block size bit ops. */
enum : Bit_block {
    /** @internal A mask of a bit block with all bits on. */
    BLOCK_ON = (Bit_block)~0,
    /** @internal The Most Significant Bit of a bit block turned on to 1. */
    BLOCK_MSB = (Bit_block)1 << ((SIZEOF_BLOCK * CHAR_BIT) - 1),
};

/** @internal An index into the block array or count of bit blocks. The block
array is constructed by the number of blocks required to support the current bit
set capacity. Assume this index type has range [0, block count to support N
bits].

User input is given as a `size_t` so distinguish from that input with this type
to make it clear to the reader the index refers to a block not the given bit
index the user has provided. */
typedef size_t Block_count;

/** @internal An index within a block. A block is set to some number of bits
as determined by the type used for each block. This type is intended to count
bits in a block and therefore cannot count up to arbitrary indices. Assume its
range is `[0, BIT_BLOCK_BITS]`, for ease of use and clarity.

There are many types of indexing and counting that take place in a bit set so
use this type specifically for counting bits in a block so the reader is clear
on intent. */
typedef uint8_t Bit_count;

enum : Bit_count {
    /** @internal How many total bits that fit in a bit block. */
    BLOCK_BITS = SIZEOF_BLOCK * CHAR_BIT,
    /** @internal Used for static assert clarity. */
    U8_BLOCK_MAX = UINT8_MAX,
    /** @internal Hand coded log2 of block bits to avoid division. */
    BLOCK_BITS_LOG2 = 5,
};
static_assert(
    (BLOCK_BITS & (BLOCK_BITS - 1)) == 0,
    "the number of bits in a block is always a power of two, "
    "helping avoid division and modulo operations when possible"
);
static_assert(
    BLOCK_BITS >> BLOCK_BITS_LOG2 == 1,
    "hand coded log2 of bitblock bits is always correct"
);
static_assert(
    (Bit_count) ~((Bit_count)0) >= (Bit_count)0, "Bit_count must be unsigned"
);
static_assert(UINT8_MAX >= BLOCK_BITS, "Bit_count counts all block bits.");

/*=========================      Prototypes      ============================*/

static size_t block_count_index(size_t);
static Bit_block *block_at(struct CCC_Flat_bitset const *, size_t);
static void set(Bit_block *, size_t, CCC_Tribool);
static Bit_block on(size_t);
static void fix_end(struct CCC_Flat_bitset *);
static CCC_Tribool status(Bit_block const *, size_t);
static size_t block_count(size_t);
static CCC_Tribool
any_or_none_range(struct CCC_Flat_bitset const *, size_t, size_t, CCC_Tribool);
static CCC_Tribool all_range(struct CCC_Flat_bitset const *, size_t, size_t);
static CCC_Count first_trailing_bit_range(
    struct CCC_Flat_bitset const *, size_t, size_t, CCC_Tribool
);
static CCC_Count first_leading_bit_range(
    struct CCC_Flat_bitset const *, size_t, size_t, CCC_Tribool
);
static CCC_Count first_trailing_bits_range(
    struct CCC_Flat_bitset const *, size_t, size_t, size_t, CCC_Tribool
);
static CCC_Count first_leading_bits_range(
    struct CCC_Flat_bitset const *, size_t, size_t, size_t, CCC_Tribool
);
static CCC_Result
maybe_resize(struct CCC_Flat_bitset *, size_t, CCC_Allocator const *);
static CCC_Tribool is_mask_match(Bit_block, Bit_block);
static Bit_block trailing_ones_mask(Bit_count);
static Bit_block leading_ones_mask(Bit_count);
static void set_all(struct CCC_Flat_bitset *, CCC_Tribool);
static Bit_count bit_count_index(size_t);
static CCC_Tribool
is_subset_of(struct CCC_Flat_bitset const *, struct CCC_Flat_bitset const *);
static Bit_count popcount(Bit_block);
static Bit_count count_trailing_zeros(Bit_block);
static Bit_count count_leading_zeros(Bit_block);

/*=======================   Public Interface   ==============================*/

CCC_Tribool
CCC_flat_bitset_is_proper_subset(
    CCC_Flat_bitset const *const subset, CCC_Flat_bitset const *const set
) {
    if (!set || !subset) {
        return CCC_TRIBOOL_ERROR;
    }
    if (set->count <= subset->count) {
        return CCC_FALSE;
    }
    return is_subset_of(subset, set);
}

CCC_Tribool
CCC_flat_bitset_is_subset(
    CCC_Flat_bitset const *const subset, CCC_Flat_bitset const *const set
) {
    if (!set || !subset) {
        return CCC_TRIBOOL_ERROR;
    }
    if (set->count < subset->count) {
        return CCC_FALSE;
    }
    return is_subset_of(subset, set);
}

CCC_Result
CCC_flat_bitset_or(
    CCC_Flat_bitset *const destination, CCC_Flat_bitset const *const source
) {
    if (!destination || !source) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!destination->count || !source->count) {
        return CCC_RESULT_OK;
    }
    Block_count const end_block
        = block_count(ccc_min(destination->count, source->count));
    for (size_t b = 0; b < end_block; ++b) {
        destination->blocks[b] |= source->blocks[b];
    }
    fix_end(destination);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_bitset_xor(
    CCC_Flat_bitset *const destination, CCC_Flat_bitset const *const source
) {
    if (!destination || !source) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!destination->count || !source->count) {
        return CCC_RESULT_OK;
    }
    Block_count const end_block
        = block_count(ccc_min(destination->count, source->count));
    for (Block_count b = 0; b < end_block; ++b) {
        destination->blocks[b] ^= source->blocks[b];
    }
    fix_end(destination);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_bitset_and(
    CCC_Flat_bitset *destination, CCC_Flat_bitset const *source
) {
    if (!destination || !source) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!source->count) {
        set_all(destination, CCC_FALSE);
        return CCC_RESULT_OK;
    }
    if (!destination->count) {
        return CCC_RESULT_OK;
    }
    Block_count const end_block
        = block_count(ccc_min(destination->count, source->count));
    for (Block_count b = 0; b < end_block; ++b) {
        destination->blocks[b] &= source->blocks[b];
    }
    if (destination->count <= source->count) {
        return CCC_RESULT_OK;
    }
    /* The source widens to align with destination as integers would; same
       consequences. */
    Block_count const destination_blocks = block_count(destination->count);
    Block_count const remain = destination_blocks - end_block;
    (void)memset(
        destination->blocks + end_block, CCC_FALSE, remain * SIZEOF_BLOCK
    );
    fix_end(destination);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_bitset_shift_left(
    CCC_Flat_bitset *const bitset, size_t const left_shifts
) {
    if (!bitset) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!bitset->count || !left_shifts) {
        return CCC_RESULT_OK;
    }
    if (left_shifts >= bitset->count) {
        set_all(bitset, CCC_FALSE);
        return CCC_RESULT_OK;
    }
    Block_count const end = block_count_index(bitset->count - 1);
    Block_count const blocks = block_count_index(left_shifts);
    Bit_count const split = bit_count_index(left_shifts);
    if (!split) {
        for (Block_count shift = end - blocks + 1, write = end; shift--;
             --write) {
            bitset->blocks[write] = bitset->blocks[shift];
        }
    } else {
        Bit_count const remain = BLOCK_BITS - split;
        for (Block_count shift = end - blocks, write = end; shift > 0;
             --shift, --write) {
            bitset->blocks[write] = (bitset->blocks[shift] << split)
                                  | (bitset->blocks[shift - 1] >> remain);
        }
        bitset->blocks[blocks] = bitset->blocks[0] << split;
    }
    /* Zero fills in lower bits just as an integer shift would. */
    for (Block_count i = 0; i < blocks; ++i) {
        bitset->blocks[i] = 0;
    }
    fix_end(bitset);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_bitset_shift_right(
    CCC_Flat_bitset *const bitset, size_t const right_shifts
) {
    if (!bitset) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!bitset->count || !right_shifts) {
        return CCC_RESULT_OK;
    }
    if (right_shifts >= bitset->count) {
        set_all(bitset, CCC_FALSE);
        return CCC_RESULT_OK;
    }
    Block_count const end = block_count_index(bitset->count - 1);
    Block_count const blocks = block_count_index(right_shifts);
    Bit_count const split = bit_count_index(right_shifts);
    if (!split) {
        for (Block_count shift = blocks, write = 0; shift < end + 1;
             ++shift, ++write) {
            bitset->blocks[write] = bitset->blocks[shift];
        }
    } else {
        Bit_count const remain = BLOCK_BITS - split;
        for (Block_count shift = blocks, write = 0; shift < end;
             ++shift, ++write) {
            bitset->blocks[write] = (bitset->blocks[shift + 1] << remain)
                                  | (bitset->blocks[shift] >> split);
        }
        bitset->blocks[end - blocks] = bitset->blocks[end] >> split;
    }
    /* This is safe for a few reasons:
       - If shifts equals count we set all to 0 and returned early.
       - If we only have one block i will be equal to end and we are done.
       - If end is the 0th block we will stop after 1. A meaningful shift
         occurred in the 0th block so zeroing would be a mistake.
       - All other cases ensure it is safe to decrease i (no underflow).
       This operation emulates the zeroing of high bits on a right shift and
       a bit set is considered unsigned so we don't sign bit fill. */
    for (Block_count i = end; i > end - blocks; --i) {
        bitset->blocks[i] = 0;
    }
    fix_end(bitset);
    return CCC_RESULT_OK;
}

CCC_Tribool
CCC_flat_bitset_test(CCC_Flat_bitset const *const bitset, size_t const i) {
    if (!bitset) {
        return CCC_TRIBOOL_ERROR;
    }
    if (i >= bitset->count) {
        return CCC_TRIBOOL_ERROR;
    }
    return status(block_at(bitset, i), i);
}

CCC_Tribool
CCC_flat_bitset_set(
    CCC_Flat_bitset *const bitset, size_t const i, CCC_Tribool const b
) {
    if (!bitset) {
        return CCC_TRIBOOL_ERROR;
    }
    if (i >= bitset->count) {
        return CCC_TRIBOOL_ERROR;
    }
    Bit_block *const block = block_at(bitset, i);
    CCC_Tribool const was = status(block, i);
    set(block, i, b);
    return was;
}

CCC_Result
CCC_flat_bitset_set_all(CCC_Flat_bitset *const bitset, CCC_Tribool const b) {
    if (!bitset) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (bitset->count) {
        set_all(bitset, b);
    }
    return CCC_RESULT_OK;
}

/** A naive implementation might just call set for every index between the
start and start + count. However, calculating the block and index within each
block for every call to set costs a division and a modulo operation. This also
loads and stores a block multiple times just to set each bit within a block to
the same value. We can avoid this by handling the first and last block with one
operations and then handling everything in between with a bulk memset. */
CCC_Result
CCC_flat_bitset_set_range(
    CCC_Flat_bitset *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count,
    CCC_Tribool const b
) {
    size_t const range_end = range_start_index + range_bit_count;
    if (!bitset || !range_bit_count || range_start_index >= bitset->count
        || range_end < range_start_index || range_end > bitset->count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    Block_count start_block = block_count_index(range_start_index);
    Bit_count const start_bit = bit_count_index(range_start_index);
    Bit_block range_mask = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + range_bit_count < BLOCK_BITS) {
        range_mask
            &= trailing_ones_mask((Bit_count)(start_bit + range_bit_count));
    }

    /* Logic is uniform except for key lines to turn bits on or off. */
    b ? (bitset->blocks[start_block] |= range_mask)
      : (bitset->blocks[start_block] &= ~range_mask);

    Block_count const end_block = block_count_index(range_end - 1);
    if (end_block == start_block) {
        fix_end(bitset);
        return CCC_RESULT_OK;
    }
    if (++start_block != end_block) {
        int const v = b ? ~0 : 0;
        (void)memset(
            &bitset->blocks[start_block],
            v,
            (end_block - start_block) * SIZEOF_BLOCK
        );
    }
    /* Need the final included bit position modulo block size but then feed to
       mask creation function as a count. */
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(range_end - 1) + 1);

    b ? (bitset->blocks[end_block] |= last_block_mask)
      : (bitset->blocks[end_block] &= ~last_block_mask);

    fix_end(bitset);
    return CCC_RESULT_OK;
}

CCC_Tribool
CCC_flat_bitset_reset(CCC_Flat_bitset *const bitset, size_t const i) {
    if (!bitset) {
        return CCC_TRIBOOL_ERROR;
    }
    if (i >= bitset->count) {
        return CCC_TRIBOOL_ERROR;
    }
    Bit_block *const block = block_at(bitset, i);
    CCC_Tribool const was = status(block, i);
    *block &= ~on(i);
    fix_end(bitset);
    return was;
}

CCC_Result
CCC_flat_bitset_reset_all(CCC_Flat_bitset *const bitset) {
    if (!bitset) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (bitset->count) {
        (void)memset(
            bitset->blocks, CCC_FALSE, block_count(bitset->count) * SIZEOF_BLOCK
        );
    }
    return CCC_RESULT_OK;
}

/** Same concept as set range but easier. Handle first and last then set
everything in between to false with memset. */
CCC_Result
CCC_flat_bitset_reset_range(
    CCC_Flat_bitset *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    size_t const range_end = range_start_index + range_bit_count;
    if (!bitset || !range_bit_count || range_start_index >= bitset->count
        || range_end < range_start_index || range_end > bitset->count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    Block_count start_block = block_count_index(range_start_index);
    Bit_count const start_bit = bit_count_index(range_start_index);
    Bit_block first_block_mask = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + range_bit_count < BLOCK_BITS) {
        first_block_mask
            &= trailing_ones_mask((Bit_count)(start_bit + range_bit_count));
    }
    bitset->blocks[start_block] &= ~first_block_mask;
    Block_count const end_block = block_count_index(range_end - 1);
    if (end_block == start_block) {
        fix_end(bitset);
        return CCC_RESULT_OK;
    }
    if (++start_block != end_block) {
        (void)memset(
            &bitset->blocks[start_block],
            CCC_FALSE,
            (end_block - start_block) * SIZEOF_BLOCK
        );
    }
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(range_end - 1) + 1);
    bitset->blocks[end_block] &= ~last_block_mask;
    fix_end(bitset);
    return CCC_RESULT_OK;
}

CCC_Tribool
CCC_flat_bitset_flip(CCC_Flat_bitset *const bitset, size_t const i) {
    if (!bitset || i > bitset->count) {
        return CCC_TRIBOOL_ERROR;
    }
    Block_count const b_i = block_count_index(i);
    Bit_block *const block = &bitset->blocks[b_i];
    CCC_Tribool const was = status(block, i);
    *block ^= on(i);
    fix_end(bitset);
    return was;
}

CCC_Result
CCC_flat_bitset_flip_all(CCC_Flat_bitset *const bitset) {
    if (!bitset) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!bitset->count) {
        return CCC_RESULT_OK;
    }
    Block_count const end = block_count(bitset->count);
    for (size_t i = 0; i < end; ++i) {
        bitset->blocks[i] = ~bitset->blocks[i];
    }
    fix_end(bitset);
    return CCC_RESULT_OK;
}

/** Maybe future SIMD vectorization could speed things up here because we use
the same strat of handling first and last which just leaves a simpler bulk
operation in the middle. But we don't benefit from memset here. */
CCC_Result
CCC_flat_bitset_flip_range(
    CCC_Flat_bitset *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    size_t const range_end = range_start_index + range_bit_count;
    if (!bitset || !range_bit_count || range_start_index >= bitset->count
        || range_end < range_start_index || range_end > bitset->count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    Block_count start_block = block_count_index(range_start_index);
    Bit_count const start_bit = bit_count_index(range_start_index);
    Bit_block first_block_on = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + range_bit_count < BLOCK_BITS) {
        first_block_on
            &= trailing_ones_mask((Bit_count)(start_bit + range_bit_count));
    }
    bitset->blocks[start_block] ^= first_block_on;
    Block_count const end_block = block_count_index(range_end - 1);
    if (end_block == start_block) {
        fix_end(bitset);
        return CCC_RESULT_OK;
    }
    while (++start_block < end_block) {
        bitset->blocks[start_block] = ~bitset->blocks[start_block];
    }
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(range_end - 1) + 1);
    bitset->blocks[end_block] ^= last_block_mask;
    fix_end(bitset);
    return CCC_RESULT_OK;
}

CCC_Count
CCC_flat_bitset_capacity(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return (CCC_Count){.count = bitset->capacity};
}

CCC_Count
CCC_flat_bitset_blocks_capacity(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return (CCC_Count){
        .count = block_count(bitset->capacity),
    };
}

CCC_Count
CCC_flat_bitset_count(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return (CCC_Count){.count = bitset->count};
}

CCC_Count
CCC_flat_bitset_blocks_count(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return (CCC_Count){
        .count = block_count(bitset->count),
    };
}

CCC_Tribool
CCC_flat_bitset_is_empty(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return CCC_TRIBOOL_ERROR;
    }
    return !bitset->count;
}

CCC_Count
CCC_flat_bitset_popcount(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    if (!bitset->count) {
        return (CCC_Count){.count = 0};
    }
    Block_count const end = block_count(bitset->count);
    size_t count = 0;
    for (Block_count i = 0; i < end; ++i) {
        count += popcount(bitset->blocks[i]);
    }
    return (CCC_Count){.count = count};
}

CCC_Count
CCC_flat_bitset_popcount_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    size_t const range_end = range_start_index + range_bit_count;
    if (!bitset || !range_bit_count || range_start_index >= bitset->count
        || range_end < range_start_index || range_end > bitset->count) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    size_t popped = 0;
    Block_count start_block = block_count_index(range_start_index);
    Bit_count const start_bit = bit_count_index(range_start_index);
    Bit_block first_block_mask = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + range_bit_count < BLOCK_BITS) {
        first_block_mask
            &= trailing_ones_mask((Bit_count)(start_bit + range_bit_count));
    }
    popped += popcount(first_block_mask & bitset->blocks[start_block]);
    Block_count const end_block = block_count_index(range_end - 1);
    if (end_block == start_block) {
        return (CCC_Count){.count = popped};
    }
    while (++start_block < end_block) {
        popped += popcount(bitset->blocks[start_block]);
    }
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(range_end - 1) + 1);
    popped += popcount(last_block_mask & bitset->blocks[end_block]);
    return (CCC_Count){.count = popped};
}

CCC_Result
CCC_flat_bitset_push_back(
    CCC_Flat_bitset *const bitset,
    CCC_Tribool const b,
    CCC_Allocator const *const allocator
) {
    if (!bitset || !allocator || b > CCC_TRUE || b < CCC_FALSE) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    CCC_Result const check_resize = maybe_resize(bitset, 1, allocator);
    if (check_resize != CCC_RESULT_OK) {
        return check_resize;
    }
    ++bitset->count;
    set(block_at(bitset, bitset->count - 1), bitset->count - 1, b);
    fix_end(bitset);
    return CCC_RESULT_OK;
}

CCC_Tribool
CCC_flat_bitset_pop_back(CCC_Flat_bitset *const bitset) {
    if (!bitset || !bitset->count) {
        return CCC_TRIBOOL_ERROR;
    }
    CCC_Tribool const was
        = status(block_at(bitset, bitset->count - 1), bitset->count - 1);
    --bitset->count;
    fix_end(bitset);
    return was;
}

CCC_Tribool
CCC_flat_bitset_any_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return any_or_none_range(
        bitset, range_start_index, range_bit_count, CCC_TRUE
    );
}

CCC_Tribool
CCC_flat_bitset_any(CCC_Flat_bitset const *const bitset) {
    return any_or_none_range(bitset, 0, bitset->count, CCC_TRUE);
}

CCC_Tribool
CCC_flat_bitset_none_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return any_or_none_range(
        bitset, range_start_index, range_bit_count, CCC_FALSE
    );
}

CCC_Tribool
CCC_flat_bitset_none(CCC_Flat_bitset const *const bitset) {
    return any_or_none_range(bitset, 0, bitset->count, CCC_FALSE);
}

CCC_Tribool
CCC_flat_bitset_all_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return all_range(bitset, range_start_index, range_bit_count);
}

CCC_Tribool
CCC_flat_bitset_all(CCC_Flat_bitset const *const bitset) {
    return all_range(bitset, 0, bitset->count);
}

CCC_Count
CCC_flat_bitset_first_trailing_one_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return first_trailing_bit_range(
        bitset, range_start_index, range_bit_count, CCC_TRUE
    );
}

CCC_Count
CCC_flat_bitset_first_trailing_one(CCC_Flat_bitset const *const bitset) {
    return first_trailing_bit_range(bitset, 0, bitset->count, CCC_TRUE);
}

CCC_Count
CCC_flat_bitset_first_trailing_ones(
    CCC_Flat_bitset const *const bitset, size_t const ones_count
) {
    return first_trailing_bits_range(
        bitset, 0, bitset->count, ones_count, CCC_TRUE
    );
}

CCC_Count
CCC_flat_bitset_first_trailing_ones_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count,
    size_t const ones_count
) {
    return first_trailing_bits_range(
        bitset, range_start_index, range_bit_count, ones_count, CCC_TRUE
    );
}

CCC_Count
CCC_flat_bitset_first_trailing_zero_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return first_trailing_bit_range(
        bitset, range_start_index, range_bit_count, CCC_FALSE
    );
}

CCC_Count
CCC_flat_bitset_first_trailing_zero(CCC_Flat_bitset const *const bitset) {
    return first_trailing_bit_range(bitset, 0, bitset->count, CCC_FALSE);
}

CCC_Count
CCC_flat_bitset_first_trailing_zeros(
    CCC_Flat_bitset const *const bitset, size_t const zeros_count
) {
    return first_trailing_bits_range(
        bitset, 0, bitset->count, zeros_count, CCC_FALSE
    );
}

CCC_Count
CCC_flat_bitset_first_trailing_zeros_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count,
    size_t const zeros_count
) {
    return first_trailing_bits_range(
        bitset, range_start_index, range_bit_count, zeros_count, CCC_FALSE
    );
}

CCC_Count
CCC_flat_bitset_first_leading_one_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return first_leading_bit_range(
        bitset, range_start_index, range_bit_count, CCC_TRUE
    );
}

CCC_Count
CCC_flat_bitset_first_leading_one(CCC_Flat_bitset const *const bitset) {
    return first_leading_bit_range(bitset, 0, bitset->count, CCC_TRUE);
}

CCC_Count
CCC_flat_bitset_first_leading_ones(
    CCC_Flat_bitset const *const bitset, size_t const ones_count
) {
    return first_leading_bits_range(
        bitset, 0, bitset->count, ones_count, CCC_TRUE
    );
}

CCC_Count
CCC_flat_bitset_first_leading_ones_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count,
    size_t const ones_count
) {
    return first_leading_bits_range(
        bitset, range_start_index, range_bit_count, ones_count, CCC_TRUE
    );
}

CCC_Count
CCC_flat_bitset_first_leading_zero_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count
) {
    return first_leading_bit_range(
        bitset, range_start_index, range_bit_count, CCC_FALSE
    );
}

CCC_Count
CCC_flat_bitset_first_leading_zero(CCC_Flat_bitset const *const bitset) {
    return first_leading_bit_range(bitset, 0, bitset->count, CCC_FALSE);
}

CCC_Count
CCC_flat_bitset_first_leading_zeros(
    CCC_Flat_bitset const *const bitset, size_t const zeros_count
) {
    return first_leading_bits_range(
        bitset, 0, bitset->count, zeros_count, CCC_FALSE
    );
}

CCC_Count
CCC_flat_bitset_first_leading_zeros_range(
    CCC_Flat_bitset const *const bitset,
    size_t const range_start_index,
    size_t const range_bit_count,
    size_t const zeros_count
) {
    return first_leading_bits_range(
        bitset, range_start_index, range_bit_count, zeros_count, CCC_FALSE
    );
}

CCC_Result
CCC_flat_bitset_clear(CCC_Flat_bitset *const bitset) {
    if (!bitset) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (bitset->blocks) {
        assert(bitset->capacity);
        (void)memset(
            bitset->blocks,
            CCC_FALSE,
            block_count(bitset->capacity) * SIZEOF_BLOCK
        );
    }
    bitset->count = 0;
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_bitset_clear_and_free(
    CCC_Flat_bitset *const bitset, CCC_Allocator const *const allocator
) {
    if (!bitset || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!allocator->allocate) {
        return CCC_RESULT_NO_ALLOCATION_FUNCTION;
    }
    if (bitset->blocks) {
        (void)allocator->allocate((CCC_Allocator_arguments){
            .input = bitset->blocks,
            .bytes = 0,
            .alignment = alignof(*bitset->blocks),
            .context = allocator->context,
        });
    }
    bitset->count = 0;
    bitset->capacity = 0;
    bitset->blocks = NULL;
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_bitset_reserve(
    CCC_Flat_bitset *const bitset,
    size_t const to_add,
    CCC_Allocator const *const allocator
) {
    if (!bitset || !allocator || !allocator->allocate || !to_add) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    return maybe_resize(bitset, to_add, allocator);
}

CCC_Result
CCC_flat_bitset_copy(
    CCC_Flat_bitset *const destination,
    CCC_Flat_bitset const *const source,
    CCC_Allocator const *const allocator
) {
    if (!destination || !source || !allocator
        || (destination->capacity < source->capacity && !allocator->allocate)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!source->capacity) {
        destination->count = 0;
        return CCC_RESULT_OK;
    }
    if (destination->capacity < source->capacity) {
        Bit_block *const new_data
            = allocator->allocate((CCC_Allocator_arguments){
                .input = destination->blocks,
                .bytes = block_count(source->capacity) * SIZEOF_BLOCK,
                .alignment = alignof((*destination->blocks)),
                .context = allocator->context,
            });
        if (!new_data) {
            return CCC_RESULT_ALLOCATOR_ERROR;
        }
        destination->blocks = new_data;
        destination->capacity = source->capacity;
    }
    if (!source->blocks || !destination->blocks) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    destination->count = source->count;
    (void)memcpy(
        destination->blocks,
        source->blocks,
        block_count(source->capacity) * SIZEOF_BLOCK
    );
    fix_end(destination);
    return CCC_RESULT_OK;
}

void *
CCC_flat_bitset_data(CCC_Flat_bitset const *const bitset) {
    if (!bitset) {
        return NULL;
    }
    return bitset->blocks;
}

CCC_Tribool
CCC_flat_bitset_is_equal(
    CCC_Flat_bitset const *const left, CCC_Flat_bitset const *const right
) {
    if (!left || !right) {
        return CCC_TRIBOOL_ERROR;
    }
    if (left->count != right->count) {
        return CCC_FALSE;
    }
    if (!left->count) {
        return CCC_TRUE;
    }
    return memcmp(
               left->blocks,
               right->blocks,
               block_count(left->count) * SIZEOF_BLOCK
           )
        == 0;
}

/*=========================     Private Interface   =========================*/

CCC_Result
CCC_private_flat_bitset_reserve(
    struct CCC_Flat_bitset *const bitset,
    size_t const to_add,
    CCC_Allocator const *const allocator
) {
    return CCC_flat_bitset_reserve(bitset, to_add, allocator);
}

CCC_Tribool
CCC_private_flat_bitset_set(
    struct CCC_Flat_bitset *const bitset,
    size_t const index,
    CCC_Tribool const bit
) {
    return CCC_flat_bitset_set(bitset, index, bit);
}

/*=======================    Static Helpers    ==============================*/

/** Assumes set size is greater than or equal to subset size. */
static inline CCC_Tribool
is_subset_of(
    struct CCC_Flat_bitset const *const subset,
    struct CCC_Flat_bitset const *const set
) {
    assert(set->count >= subset->count);
    for (Block_count i = 0, end = block_count(subset->count); i < end; ++i) {
        /* Invariant: the last N unused bits in a set zero so this works. */
        if (!is_mask_match(set->blocks[i], subset->blocks[i])) {
            return CCC_FALSE;
        }
    }
    return CCC_TRUE;
}

static CCC_Result
maybe_resize(
    struct CCC_Flat_bitset *const bitset,
    size_t const to_add,
    CCC_Allocator const *const allocator
) {
    size_t bits_needed = bitset->count + to_add;
    if (bits_needed <= bitset->capacity) {
        return CCC_RESULT_OK;
    }
    if (!allocator->allocate) {
        return CCC_RESULT_NO_ALLOCATION_FUNCTION;
    }
    if (to_add == 1) {
        bits_needed <<= 1;
    }
    static_assert(
        (BLOCK_BITS & (BLOCK_BITS - 1)) == 0,
        "rounding trick only works for powers of 2"
    );
    size_t const new_capacity
        = (bits_needed + (BLOCK_BITS - 1)) & ~((size_t)BLOCK_BITS - 1);
    if (new_capacity <= bitset->capacity) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    size_t const new_bytes
        = block_count(new_capacity - bitset->count) * SIZEOF_BLOCK;
    size_t const old_bytes
        = bitset->count ? block_count(bitset->count) * SIZEOF_BLOCK : 0;
    Bit_block *const new_data = allocator->allocate((CCC_Allocator_arguments){
        .input = bitset->blocks,
        .bytes = block_count(new_capacity) * SIZEOF_BLOCK,
        .alignment = alignof(*bitset->blocks),
        .context = allocator->context,
    });
    if (!new_data) {
        return CCC_RESULT_ALLOCATOR_ERROR;
    }
    (void)memset((char *)new_data + old_bytes, 0, new_bytes);
    bitset->capacity = new_capacity;
    bitset->blocks = new_data;
    return CCC_RESULT_OK;
}

/** A trailing bit in a range is the first bit set to the specified boolean
value in the provided range. The input i gives the starting bit of the search,
meaning a bit within a block that is the inclusive start of the range. The count
gives us the end of the search for an overall range of `[i, i + count)`. This
means if the search range is greater than a single block we will iterate in
ascending order through our blocks and from least to most significant bit within
each block. */
static CCC_Count
first_trailing_bit_range(
    struct CCC_Flat_bitset const *const bitset,
    size_t const i,
    size_t const count,
    CCC_Tribool const is_one
) {
    size_t const exclusive_end = i + count;
    if (!bitset || !count || i >= bitset->count || exclusive_end < i
        || exclusive_end > bitset->count) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    Block_count start_block = block_count_index(i);
    Bit_count const start_bit = bit_count_index(i);
    Bit_block first_block_mask = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + count < BLOCK_BITS) {
        first_block_mask &= trailing_ones_mask((Bit_count)(start_bit + count));
    }
    Bit_count trailing_zeros = count_trailing_zeros(
        first_block_mask
        & (is_one ? bitset->blocks[start_block] : ~bitset->blocks[start_block])
    );
    if (trailing_zeros != BLOCK_BITS) {
        return (CCC_Count){
            .count = (start_block * BLOCK_BITS) + trailing_zeros,
        };
    }
    Block_count const end_block = block_count_index(exclusive_end - 1);
    if (end_block == start_block) {
        return (CCC_Count){.error = CCC_RESULT_FAIL};
    }
    while (++start_block < end_block) {
        trailing_zeros = count_trailing_zeros(
            is_one ? bitset->blocks[start_block] : ~bitset->blocks[start_block]
        );
        if (trailing_zeros != BLOCK_BITS) {
            return (CCC_Count){
                .count = (start_block * BLOCK_BITS) + trailing_zeros,
            };
        }
    }
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(exclusive_end - 1) + 1);
    trailing_zeros = count_trailing_zeros(
        last_block_mask
        & (is_one ? bitset->blocks[end_block] : ~bitset->blocks[end_block])
    );
    if (trailing_zeros != BLOCK_BITS) {
        return (CCC_Count){
            .count = (end_block * BLOCK_BITS) + trailing_zeros,
        };
    }
    return (CCC_Count){.error = CCC_RESULT_FAIL};
}

/** Finds the starting index of a sequence of 1's or 0's of the num_bits size in
linear time. The algorithm aims to efficiently skip as many bits as possible
while searching for the desired group. This avoids both an O(N^2) runtime and
the use of any unnecessary modulo or division operations in a hot loop. */
static CCC_Count
first_trailing_bits_range( /* NOLINT (*cognitive-complexity) */
    struct CCC_Flat_bitset const *const bitset,
    size_t const i,
    size_t const count,
    size_t const num_bits,
    CCC_Tribool const ones
) {
    size_t const exclusive_end = i + count;
    if (!bitset || !count || !num_bits || i >= bitset->count || num_bits > count
        || exclusive_end < i || exclusive_end > bitset->count) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    size_t bit_count = 0;
    size_t window_start = i;
    Block_count block_index = block_count_index(i);
    size_t window_end = (block_index * BLOCK_BITS) + BLOCK_BITS;
    Bit_count bit_index = bit_count_index(i);
    Bit_block bits
        = ones ? bitset->blocks[block_index] : ~bitset->blocks[block_index];
    bits &= leading_ones_mask(BLOCK_BITS - bit_index);
    for (;;) {
        if (window_end > exclusive_end) {
            bits &= trailing_ones_mask(bit_count_index(exclusive_end - 1) + 1);
        }
        if (!bits) {
            window_start = (block_index + 1) * BLOCK_BITS;
            bit_count = 0;
        } else {
            size_t bits_remain = num_bits - bit_count;
            /* I would rather check for a prefix that could be completing in
               this block here than check for a failure and reset to number of
               bits on every iteration of the next loop. Think of this like loop
               unrolling while also making next block simpler. */
            if (bits_remain <= BLOCK_BITS && bits_remain < num_bits) {
                assert(bit_index < BLOCK_BITS && "shifts are valid for block");
                Bit_block const shifted_bits = bits >> bit_index;
                if (is_mask_match(
                        shifted_bits, trailing_ones_mask((Bit_count)bits_remain)
                    )) {
                    return (CCC_Count){.count = window_start};
                }
                bits_remain = num_bits;
                bit_index += count_trailing_zeros(~shifted_bits);
                bit_count = 0;
                window_start = (block_index * BLOCK_BITS) + bit_index;
            }
            if (bits_remain <= BLOCK_BITS) {
                assert(bit_index < BLOCK_BITS && "shifts are valid for block");
                Bit_block shifted_bits = bits >> bit_index;
                Bit_block const bits_remain_mask
                    = trailing_ones_mask((Bit_count)bits_remain);
                /* The loop continues only while our block is numerically
                   greater than the mask. Because unsigned integers are
                   represented in base 2 we get two automatic early exits here.
                       - If the block is missing a high-order bit in the
                         required mask, it is numerically smaller than the mask
                         and cannot match with further shifting.
                       - If all high bits match but some lower required bits are
                         zero, the block is numerically smaller than the mask
                         and cannot match with further shifting.
                   If the block has high order bits not in the mask it is
                   greater than the mask and we continue checking, which is
                   correct. This strategy optimizes out some useless shifts. */
                while (shifted_bits >= bits_remain_mask) {
                    if (is_mask_match(shifted_bits, bits_remain_mask)) {
                        return (CCC_Count){
                            .count = (block_index * BLOCK_BITS) + bit_index,
                        };
                    }
                    ++bit_index;
                    shifted_bits >>= 1;
                }
                bit_count = 0;
            }
            /* 2 cases covered: the ones remaining are greater than this block
               could hold or we did not find a match by the masking we just did.
               In either case we need the maximum contiguous ones that run all
               the way to the MSB. The best we could have is a full block of
               1's. Otherwise we need to find where to start our new search for
               contiguous 1's. This could be the next block if there are not 1's
               that continue to MSB. */
            Bit_count const leading_ones = count_leading_zeros(~bits);
            bit_count += leading_ones;
            if (leading_ones < BLOCK_BITS) {
                window_start
                    = (block_index * BLOCK_BITS) + (BLOCK_BITS - leading_ones);
            }
        }
        if (window_start + num_bits > exclusive_end) {
            return (CCC_Count){.error = CCC_RESULT_FAIL};
        }
        bit_index = 0;
        ++block_index;
        window_end += BLOCK_BITS;
        assert(
            block_index < block_count_index(bitset->capacity)
            && "only load bits within block array capacity"
        );
        bits
            = ones ? bitset->blocks[block_index] : ~bitset->blocks[block_index];
    }
}

/** A leading bit is the first bit in the range to be set to the indicated value
within a block starting the search from the Most Significant Bit of each block.
This means that if the range is larger than a single block we iterate in
descending order through the set of blocks starting at `i + count
- 1` for the range of `[i, i + count)`. The search within a given block
proceeds from Most Significant Bit toward Least Significant Bit. */
static CCC_Count
first_leading_bit_range(
    struct CCC_Flat_bitset const *const bitset,
    size_t const start_index,
    size_t const count,
    CCC_Tribool const is_one
) {
    if (!bitset || !count || start_index >= bitset->count
        || start_index + count <= start_index
        || start_index + count > bitset->count) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    size_t const window_start = start_index + count - 1;
    Bit_count const start_bit = bit_count_index(window_start);
    Bit_count const end_bit = bit_count_index(start_index);
    Block_count start_block = block_count_index(window_start);
    Bit_block first_block_mask = trailing_ones_mask(start_bit + 1);
    if (start_index + count < BLOCK_BITS) {
        first_block_mask &= leading_ones_mask(BLOCK_BITS - end_bit);
    }
    Bit_count leading_zeros = count_leading_zeros(
        first_block_mask
        & (is_one ? bitset->blocks[start_block] : ~bitset->blocks[start_block])
    );
    if (leading_zeros != BLOCK_BITS) {
        return (CCC_Count){
            .count = (start_block * BLOCK_BITS)
                   + (Block_count)(BLOCK_BITS - leading_zeros - 1),
        };
    }
    Block_count const end_block = block_count_index(start_index);
    if (start_block == end_block) {
        return (CCC_Count){.error = CCC_RESULT_FAIL};
    }
    while (--start_block > end_block) {
        leading_zeros = count_leading_zeros(
            is_one ? bitset->blocks[start_block] : ~bitset->blocks[start_block]
        );
        if (leading_zeros != BLOCK_BITS) {
            return (CCC_Count){
                .count = (start_block * BLOCK_BITS)
                       + (Block_count)(BLOCK_BITS - leading_zeros - 1),
            };
        }
    }
    Bit_block const last_block_on = leading_ones_mask(BLOCK_BITS - end_bit);
    leading_zeros = count_leading_zeros(
        last_block_on
        & (is_one ? bitset->blocks[end_block] : ~bitset->blocks[end_block])
    );
    if (leading_zeros != BLOCK_BITS) {
        return (CCC_Count){
            .count = (end_block * BLOCK_BITS)
                   + (Block_count)(BLOCK_BITS - leading_zeros - 1),
        };
    }
    return (CCC_Count){.error = CCC_RESULT_FAIL};
}

/** Iterating backward from MSB to LSB requires more care to avoid unsigned
integer wrapping. Therefore, this code is not identical to the trailing version
due to a few more branches. This function previously used signed types to avoid
this branching but that required making new signed types, copious casting, and
verification of input to be within ptrdiff_t limits. For consistency and
portability I think committing to unsigned is better for this function. */
static CCC_Count
first_leading_bits_range( /* NOLINT (*cognitive-complexity) */
    struct CCC_Flat_bitset const *const bitset,
    size_t const index,
    size_t const range_count,
    size_t const bits_required,
    CCC_Tribool const ones
) {
    if (!bitset || !range_count || !bits_required || index >= bitset->count
        || bits_required > range_count) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    size_t bit_count = 0;
    size_t window_start = (index + range_count - 1);
    Block_count block_index = block_count_index(window_start);
    Block_count window_inclusive_end = ((block_index * BLOCK_BITS));
    Bit_count bit_index = bit_count_index(window_start);
    Bit_block bits
        = ones ? bitset->blocks[block_index] : ~bitset->blocks[block_index];
    bits &= trailing_ones_mask(bit_index + 1);
    for (;;) {
        if (window_inclusive_end < index) {
            bits &= leading_ones_mask(BLOCK_BITS - bit_count_index(index));
        }
        if (!bits) {
            window_start = block_index ? (block_index * BLOCK_BITS) - 1 : 0;
            bit_count = 0;
        } else {
            size_t bits_remain = bits_required - bit_count;
            /* I would rather check for a prefix that could be completing in
               this block here than check for a failure and reset to number of
               bits on every iteration of the next loop. Think of this like loop
               unrolling while also making next block simpler. */
            if (bits_remain <= BLOCK_BITS && bits_remain < bits_required) {
                assert(bit_index < BLOCK_BITS && "shifts are valid for block");
                Bit_block const shifted_block = bits
                                             << (BLOCK_BITS - bit_index - 1);
                if (is_mask_match(
                        shifted_block, leading_ones_mask((Bit_count)bits_remain)
                    )) {
                    return (CCC_Count){.count = window_start};
                }
                bits_remain = bits_required;
                Bit_count const leading_ones
                    = count_leading_zeros(~shifted_block);
                assert(bit_index >= leading_ones && "index cannot underflow");
                bit_index -= leading_ones;
                window_start = (block_index * BLOCK_BITS) + bit_index;
                bit_count = 0;
            }
            if (bits_remain <= BLOCK_BITS) {
                assert(bit_index < BLOCK_BITS && "shifts are valid for block");
                Bit_block shifted_block = bits << (BLOCK_BITS - bit_index - 1);
                Bit_block const bits_remain_mask
                    = leading_ones_mask((Bit_count)bits_remain);
                /* Can't find a clever way to reduce shifts like trailing. */
                Bit_count const end_index = (Bit_count)(bits_remain - 1);
                while (bit_index >= end_index) {
                    if (is_mask_match(shifted_block, bits_remain_mask)) {
                        return (CCC_Count){
                            .count = (block_index * BLOCK_BITS) + bit_index,
                        };
                    }
                    --bit_index;
                    shifted_block <<= 1;
                }
                bit_count = 0;
            }
            Bit_count const trailing_ones = count_trailing_zeros(~bits);
            bit_count += trailing_ones;
            if (trailing_ones < BLOCK_BITS) {
                window_start = (block_index * BLOCK_BITS) + trailing_ones;
                if (window_start) {
                    --window_start;
                }
            }
        }
        if (window_start < index + bits_required - 1) {
            return (CCC_Count){.error = CCC_RESULT_FAIL};
        }
        bit_index = BLOCK_BITS - 1;
        --block_index;
        window_inclusive_end = window_inclusive_end >= BLOCK_BITS
                                 ? window_inclusive_end - BLOCK_BITS
                                 : 0;
        assert(
            block_index < block_count_index(bitset->capacity)
            && "current block within range while iterating toward LSB"
        );
        bits
            = ones ? bitset->blocks[block_index] : ~bitset->blocks[block_index];
    }
}

/** Performs the any or none scan operation over the specified range. The only
difference between the operations is the return value. Specify the desired
Tribool value to return upon encountering an on bit. For any this is CCC_TRUE.
For none this is CCC_FALSE. Saves writing two identical fns. */
static CCC_Tribool
any_or_none_range(
    struct CCC_Flat_bitset const *const bitset,
    size_t const i,
    size_t const count,
    CCC_Tribool const any_or_none
) {
    size_t const range_end = i + count;
    if (!bitset || !count || i >= bitset->count || range_end < i
        || range_end > bitset->count || any_or_none < CCC_FALSE
        || any_or_none > CCC_TRUE) {
        return CCC_TRIBOOL_ERROR;
    }
    Block_count start_block = block_count_index(i);
    Bit_count const start_bit = bit_count_index(i);
    Bit_block first_block_mask = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + count < BLOCK_BITS) {
        first_block_mask &= trailing_ones_mask((Bit_count)(start_bit + count));
    }
    if (first_block_mask & bitset->blocks[start_block]) {
        return any_or_none;
    }
    Block_count const end_block = block_count_index(range_end - 1);
    if (end_block == start_block) {
        return !any_or_none;
    }
    /* If this is the any check we might get lucky by checking the last
       block before looping over everything. */
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(range_end - 1) + 1);
    if (last_block_mask & bitset->blocks[end_block]) {
        return any_or_none;
    }
    for (++start_block; start_block < end_block; ++start_block) {
        if (bitset->blocks[start_block] & BLOCK_ON) {
            return any_or_none;
        }
    }
    return !any_or_none;
}

/** Check for all on is slightly different from the any or none checks so we
need a painfully repetitive function. */
static CCC_Tribool
all_range(
    struct CCC_Flat_bitset const *const bitset,
    size_t const i,
    size_t const count
) {
    size_t const range_end = i + count;
    if (!bitset || !count || i >= bitset->count || range_end < i
        || range_end > bitset->count) {
        return CCC_TRIBOOL_ERROR;
    }
    Block_count start_block = block_count_index(i);
    Bit_count const start_bit = bit_count_index(i);
    Bit_block first_block_mask = leading_ones_mask(BLOCK_BITS - start_bit);
    if (start_bit + count < BLOCK_BITS) {
        first_block_mask &= trailing_ones_mask((Bit_count)(start_bit + count));
    }
    if ((first_block_mask & bitset->blocks[start_block]) != first_block_mask) {
        return CCC_FALSE;
    }
    Block_count const end_block = block_count_index(range_end - 1);
    if (end_block == start_block) {
        return CCC_TRUE;
    }
    while (++start_block < end_block) {
        if (bitset->blocks[start_block] != BLOCK_ON) {
            return CCC_FALSE;
        }
    }
    Bit_block const last_block_mask
        = trailing_ones_mask(bit_count_index(range_end - 1) + 1);
    return is_mask_match(bitset->blocks[end_block], last_block_mask);
}

/** Given the 0 based index from `[0, count of used bits in set)` returns a
reference to the block that such a bit belongs to. This block reference will
point to some block at index [0, count of blocks used in the set). */
static inline Bit_block *
block_at(
    struct CCC_Flat_bitset const *const bitset, size_t const flat_bitset_index
) {
    return &bitset->blocks[block_count_index(flat_bitset_index)];
}

/** Sets all bits in bulk to value b and fixes the end block to ensure all bits
in the final block that are not in use are zeroed out. */
static inline void
set_all(struct CCC_Flat_bitset *const bitset, CCC_Tribool const b) {
    int const v = b ? ~0 : 0;
    (void)memset(bitset->blocks, v, block_count(bitset->count) * SIZEOF_BLOCK);
    fix_end(bitset);
}

/** Given the appropriate block in which bit_i resides, sets the bits position
to 0 or 1 as specified by the CCC_Tribool argument b.

Assumes block has been retrieved correctly in range [0, count of blocks in set)
and that bit_i is in range [0, count of active bits in set). */
static inline void
set(Bit_block *const block,
    size_t const flat_bitset_index,
    CCC_Tribool const b) {
    if (b) {
        *block |= on(flat_bitset_index);
    } else {
        *block &= ~on(flat_bitset_index);
    }
}

/** Given the bit set and the set index--set index is allowed to be greater than
the size of one block--returns the status of the bit at that index. */
static inline CCC_Tribool
status(Bit_block const *const bitset, size_t const flat_bitset_index) {
    /* Be careful. The & op does not promise to evaluate to 1 or 0. We often
       just use it where that conversion takes place implicitly for us. */
    return (*bitset & on(flat_bitset_index)) != 0;
}

/** Given the true bit index in the bit set, expected to be in the range
[0, count of active bits in set), returns a Bit_block mask with only this bit
on in block to which it belongs. This mask guarantees to have a bit on within
a bit block at index [0, BIT_BLOCK_BITS - 1). */
static inline Bit_block
on(size_t flat_bitset_index) {
    return (Bit_block)1 << bit_count_index(flat_bitset_index);
}

/** Clears unused bits in the last block according to count. Sets the last block
to have only the used bits set to their given values and all bits after to zero.
This is used as a safety mechanism throughout the code after complex operations
on bit blocks to ensure any side effects on unused bits are deleted. */
static inline void
fix_end(struct CCC_Flat_bitset *const bitset) {
    bitset->count
        ? (*block_at(bitset, bitset->count - 1)
           &= trailing_ones_mask(bit_count_index(bitset->count - 1) + 1))
        : (bitset->blocks[0] = 0);
}

/** Returns the 0-based index of the block in the block array allocation to
which the given index belongs. Assumes the given index is somewhere between [0,
count of bits set). The returned index then represents the block in which this
index resides which is in the range [0, block containing last in use bit). */
static inline Block_count
block_count_index(size_t const flat_bitset_index) {
    static_assert(
        (typeof(flat_bitset_index))~((typeof(flat_bitset_index))0)
            >= (typeof(flat_bitset_index))0,
        "shifting to avoid division with power of 2 divisor is only "
        "defined for unsigned types"
    );
    return flat_bitset_index >> BLOCK_BITS_LOG2;
}

/** Returns the 0-based index within a block to which the given index belongs.
This index will always be between [0, BIT_BLOCK_BITS - 1). */
static inline Bit_count
bit_count_index(size_t const flat_bitset_index) {
    return flat_bitset_index & (BLOCK_BITS - 1);
}

/** Returns the number of blocks required to store the given bits. Assumes bits
is non-zero. For any bits > 1 the block count is always less than bits.*/
static inline Block_count
block_count(size_t const bit_count) {
    static_assert(
        (typeof(bit_count))~((typeof(bit_count))0) >= (typeof(bit_count))0,
        "shifting to avoid division with power of 2 divisor is only "
        "defined for unsigned types"
    );
    assert(bit_count && "calculating block count for non-empty bitset");
    return (bit_count + (BLOCK_BITS - 1)) >> BLOCK_BITS_LOG2;
}

/** Returns true if the on bit mask is present in the block. All one bits in the
mask must be found at the same positions in the block being queried. */
static inline CCC_Tribool
is_mask_match(Bit_block const block, Bit_block const on_mask) {
    return (block & on_mask) == on_mask;
}

/** Returns a mask of the specified count of trailing bits set to 1 (CCC_TRUE)
within a bit block. This is the same integer type that is stored in the bit set
integer block array. Trailing ones are ones starting from index 0, the Least
Significant Bit, counting toward the Most Significant Bit of the block. A count
of zero will return 0. A count equivalent to the block bit width will return a
mask with all bits set to 1. */
static inline Bit_block
trailing_ones_mask(Bit_count const ones_count) {
    assert(ones_count <= BLOCK_BITS && "shift is well defined for mask");
    return ones_count ? BLOCK_ON >> (BLOCK_BITS - ones_count) : 0;
}

/** Returns a mask of the specified count of leading bits set to 1 (CCC_TRUE)
within a bit block. This is the same integer type that is stored in the bit set
integer block array. Leading ones are ones starting from index BLOCK_BITS - 1,
the Most Significant Bit, counting toward 0, the Least Significant Bit, of the
block. A count of zero will return 0. A count equivalent to the block bit width
will return a mask with all bits set to 1. */
static inline Bit_block
leading_ones_mask(Bit_count const ones_count) {
    assert(ones_count <= BLOCK_BITS && "shift is well defined for mask");
    return ones_count ? BLOCK_ON << (BLOCK_BITS - ones_count) : 0;
}

/** The following asserts assure that whether portable or built in bit
operations are used in the coming section we are safe in our assumptions about
widths and counts.  Much of the code relies on the assumption that iterating
over blocks at at a time is faster than using mathematical operations to
conceptually iterate over bits. This assumptions mostly comes from the use of
these built-ins to keep the processing time linear for range based queries,
while avoiding division and modulo operations. I should test to see the
performance implications when these built-ins are gone. However they are pretty
ubiquitous these days. */

static_assert(
    BLOCK_MSB < BLOCK_ON, "most significant bit is set for correct block width"
);
static_assert(
    SIZEOF_BLOCK == sizeof(unsigned),
    "builtins remain in sync with bitset block width"
);

/** Counts the number of trailing zeros in a bit block starting from least
significant bit. */
static inline Bit_count
count_trailing_zeros(Bit_block const b) {
    assert(
        ccc_count_trailing_zeros(BLOCK_MSB) <= U8_BLOCK_MAX
        && "builtins return counts that are valid for smaller width types we "
           "use"
    );
    return (Bit_count)ccc_count_trailing_zeros(b);
}

/** Counts the leading zeros in a bit block starting from the most significant
bit. */
static inline Bit_count
count_leading_zeros(Bit_block const b) {
    assert(
        ccc_count_leading_zeros((Bit_block)1) <= U8_BLOCK_MAX
        && "builtins return counts that are valid for smaller width types we "
           "use"
    );
    return (Bit_count)ccc_count_leading_zeros(b);
}

/** Counts the on bits in a bit block. */
static inline Bit_count
popcount(Bit_block const b) {
    /* There are different pop counts for different integer widths. Be sure
       to catch the use of the wrong one by mistake here at compile time. */
    assert(
        ccc_popcount((Bit_block)~0) <= U8_BLOCK_MAX
        && "builtins return counts that are valid for smaller width types we "
           "use"
    );
    return (Bit_count)ccc_popcount(b);
}
