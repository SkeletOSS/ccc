#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "std_allocator.h"

#include "ccc/types.h"

/** So much hassle just because stdlib does not implement aligned_realloc. This
is a proof of concept for how someone could wrap stdlib aligned_alloc and free
to perform the basic functions of an alignment-aware allocator.

This struct is written to the allocation returned by aligned_alloc. The layout
is as follows:

Increasing addresses -->

| Aligned_user_bytes | pad | Log_2_alignment | aligned user base address |

The padding between the aligned base address Aligned_user_bytes returned by
aligned_alloc and the aligned user base address will vary depending on the user
requested alignment. That is why we place the log2(alignment) byte directly
before the user block base address. We ensure the user block base address is
also aligned by rounding up our allocation to accommodate the appropriate
alignment for the Aligned_user_bytes. The Log_2_alignment byte has an alignment
of 1 so fits at any address directly preceding the aligned user base address.

Overall, this allocator approach will lead to a less efficient allocator that
is slower, creates worse fragmentation, and uses more space than if the standard
library implemented aligned_realloc. That is why, in general, a custom allocator
should be found or implemented that takes care of alignment for the user. We
have no access to internal allocator headers or the ability to perform actions
such as coalescing with this wrapper approach. */
typedef size_t Aligned_user_bytes;
/** The log2(alignment) where alignment is a power of two alignment requested
by the user. Because alignments are guaranteed to be powers of 2, tracking
their logarithms is simple and allows us to store absurdly large alignment
values (up to 2^256).

This also removes any concern over alignment of this metadata as a byte can be
aligned to any address and we place it directly before the user aligned base
address. Storing alignment also allows for sanity checks on reallocation and
helps locate the base address of the entire allocation where we store the size
of the user allocation. */
typedef uint8_t Log_2_alignment;

enum : Log_2_alignment {
    LOG_2_ALIGNMENT_BITS = sizeof(Log_2_alignment) * CHAR_BIT,
    LOG_2_ALIGNMENT_MSB = (Log_2_alignment)1 << (LOG_2_ALIGNMENT_BITS - 1),
    LOG_2_MAX = UINT8_MAX
};

/** Defined extern in allocate.h */
CCC_Allocator const std_allocator = {
    .allocate = std_aligned_allocate,
};

static size_t roundup(size_t, size_t);
static size_t max_size_t(size_t, size_t);
static size_t min_size_t(size_t, size_t);
static void *record_aligned_alloc(size_t, size_t);
static void *record_aligned_realloc(size_t, void *, size_t);
static Aligned_user_bytes *allocation_for(void const *);
static inline Log_2_alignment *alignment_for(void const *);
static Log_2_alignment count_trailing_zeros(size_t);

void *
std_aligned_allocate(CCC_Allocator_arguments const arguments) {
    if (!arguments.input && !arguments.bytes) {
        return NULL;
    }
    size_t const alignment
        = arguments.alignment ? arguments.alignment : alignof(max_align_t);

    if (alignment & (alignment - 1)) {
        assert(
            (alignment & (alignment - 1)) == 0
            && "alignment must be a power of 2"
        );
        return NULL;
    }
    size_t const max_alignment
        = max_size_t(alignment, alignof(Aligned_user_bytes));
    if (!arguments.input) {
        return record_aligned_alloc(max_alignment, arguments.bytes);
    }
    if (!arguments.bytes) {
        free(allocation_for(arguments.input));
        return NULL;
    }
    return record_aligned_realloc(
        max_alignment, arguments.input, arguments.bytes
    );
}

static inline void *
record_aligned_alloc(size_t const alignment, size_t const bytes) {
    assert(alignment && "alignment required for allocation metadata");
    size_t const user_allocation_bytes = roundup(bytes, alignment);
    if (user_allocation_bytes < bytes) {
        assert(
            user_allocation_bytes >= bytes
            && "aligned byte request does not overflow"
        );
        return NULL;
    }
    size_t const total_aligned_multiple_bytes = roundup(
        user_allocation_bytes + sizeof(Aligned_user_bytes)
            + sizeof(Log_2_alignment),
        alignment
    );
    if (total_aligned_multiple_bytes < bytes) {
        assert(
            total_aligned_multiple_bytes >= bytes
            && "aligned byte request does not overflow"
        );
        return NULL;
    }
    Aligned_user_bytes *const allocation
        = aligned_alloc(alignment, total_aligned_multiple_bytes);
    if (!allocation) {
        return NULL;
    }
    void *const aligned_user_start
        = (char *)allocation
        + roundup(
              sizeof(Aligned_user_bytes) + sizeof(Log_2_alignment), alignment
        );
    *allocation = user_allocation_bytes;
    *((Log_2_alignment *)aligned_user_start - 1)
        = count_trailing_zeros(alignment);
    return aligned_user_start;
}

static void *
record_aligned_realloc(
    size_t const alignment, void *const input, size_t const new_bytes
) {
    Aligned_user_bytes const *const old_allocation = allocation_for(input);
    Log_2_alignment const *const old_log_2_alignment = alignment_for(input);
    size_t const old_alignment = (size_t)1 << (*old_log_2_alignment);
    if (alignment < old_alignment) {
        assert(
            alignment >= old_alignment
            && "aligned reallocation request must have valid alignment for "
               "previously allocated type"
        );
        return NULL;
    }
    assert(
        (old_alignment & (old_alignment - 1)) == 0
        && "Aligned_user_bytes has probably not been corrupted."
    );
    void *const aligned_location = record_aligned_alloc(alignment, new_bytes);
    if (!aligned_location) {
        return NULL;
    }
    size_t const bytes_to_copy = min_size_t(*old_allocation, new_bytes);
    (void)memcpy(aligned_location, input, bytes_to_copy);
    free((void *)old_allocation);
    return aligned_location;
}

static inline size_t
roundup(size_t const bytes, size_t const alignment) {
    return (bytes + (alignment - 1)) & ~(alignment - 1);
}

static inline size_t
max_size_t(size_t const a, size_t const b) {
    return a > b ? a : b;
}

static inline size_t
min_size_t(size_t const a, size_t const b) {
    return a < b ? a : b;
}

static inline Aligned_user_bytes *
allocation_for(void const *const aligned_user_pointer) {
    Log_2_alignment const *const log_2_alignment
        = alignment_for(aligned_user_pointer);
    size_t const alignment = (size_t)1 << (*log_2_alignment);
    Aligned_user_bytes *const allocation
        = (Aligned_user_bytes *)((char *)aligned_user_pointer
                                 - roundup(
                                     sizeof(Aligned_user_bytes)
                                         + sizeof(Log_2_alignment),
                                     alignment
                                 ));
    return allocation;
}

static inline Log_2_alignment *
alignment_for(void const *const aligned_user_pointer) {
    return (Log_2_alignment *)aligned_user_pointer - 1;
}

#if defined(__has_builtin) && __has_builtin(__builtin_ctzl)
/** Counts the number of trailing zeros in a bit block starting from least
significant bit. */
static inline Log_2_alignment
count_trailing_zeros(size_t const b) {
    static_assert(
        __builtin_ctzl(LOG_2_ALIGNMENT_MSB) <= LOG_2_MAX,
        "builtins return counts that are valid for smaller width types we use"
    );
    return b ? (Log_2_alignment)__builtin_ctzl(b) : LOG_2_ALIGNMENT_BITS;
}

#else /* !defined(__has_builtin) || !__has_builtin(__builtin_ctzl) */

/** Counts the number of trailing zeros in a bit block starting from least
significant bit. */
static inline Log_2_alignment
count_trailing_zeros(size_t b) {
    if (!b) {
        return LOG_2_ALIGNMENT_BITS;
    }
    Log_2_alignment cnt = 0;
    for (; (b & 1U) == 0; ++cnt, b >>= 1U) {}
    return cnt;
}

#endif /* defined(__has_builtin) && __has_builtin(__builtin_ctzl) */
