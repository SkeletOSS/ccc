#include <assert.h>
#include <stddef.h>
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

| aligned base | padding | struct Allocation | aligned user base |

The padding between the base address returned by aligned_alloc and the actual
struct Allocation will vary depending on the user requested alignment. That is
why we place the struct allocation directly before the user block base address.
We ensure the user block base address is also aligned by rounding up our
allocation to accommodate the appropriate alignment for the addition of the
struct Allocation.

Overall, this allocator approach will lead to a less efficient allocator that
is slower and creates worse fragmentation than if the standard library
implemented aligned_realloc. That is why, in general, a custom allocator should
be found or implemented that takes care of alignment for the user. We have no
access to internal allocator headers or the ability to perform actions such as
coalescing with this wrapper approach. */
struct Allocation {
    /** The user requested bytes rounded up to alignment. */
    size_t aligned_user_bytes;
    /** The previously requested alignment. Helps calculate the offset of this
     struct and provides sanity checks for reallocation. */
    size_t alignment;
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
static struct Allocation *allocation_for(void const *);
static void *base_for(void const *);

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
        = max_size_t(alignment, alignof(struct Allocation));
    if (!arguments.input) {
        return record_aligned_alloc(max_alignment, arguments.bytes);
    }
    if (!arguments.bytes) {
        free(base_for(arguments.input));
        return NULL;
    }
    return record_aligned_realloc(
        max_alignment, arguments.input, arguments.bytes
    );
}

static inline void *
record_aligned_alloc(size_t const alignment, size_t const bytes) {
    size_t const user_allocation_bytes = roundup(bytes, alignment);
    if (user_allocation_bytes < bytes) {
        assert(
            user_allocation_bytes >= bytes
            && "aligned byte request does not overflow"
        );
        return NULL;
    }
    size_t const total_aligned_multiple_bytes
        = roundup(user_allocation_bytes + sizeof(struct Allocation), alignment);
    if (total_aligned_multiple_bytes < bytes) {
        assert(
            total_aligned_multiple_bytes >= bytes
            && "aligned byte request does not overflow"
        );
        return NULL;
    }
    void *const aligned_base
        = aligned_alloc(alignment, total_aligned_multiple_bytes);
    if (!aligned_base) {
        return NULL;
    }
    void *const aligned_user_start
        = (char *)aligned_base + roundup(sizeof(struct Allocation), alignment);
    struct Allocation *const allocation_position
        = allocation_for(aligned_user_start);
    allocation_position->aligned_user_bytes = user_allocation_bytes;
    allocation_position->alignment = alignment;
    return aligned_user_start;
}

static void *
record_aligned_realloc(
    size_t const alignment, void *const input, size_t const new_bytes
) {
    struct Allocation const *const old_allocation = allocation_for(input);
    if (alignment < old_allocation->alignment) {
        assert(
            alignment >= old_allocation->alignment
            && "aligned reallocation request must have valid alignment for "
               "previously allocated type"
        );
        return NULL;
    }
    assert(
        (old_allocation->alignment & (old_allocation->alignment - 1)) == 0
        && "struct Allocation has probably not been corrupted."
    );
    void *const aligned_location = record_aligned_alloc(alignment, new_bytes);
    if (!aligned_location) {
        return NULL;
    }
    size_t const bytes_to_copy
        = min_size_t(old_allocation->aligned_user_bytes, new_bytes);
    (void)memcpy(aligned_location, input, bytes_to_copy);
    free(base_for(input));
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

static inline struct Allocation *
allocation_for(void const *const aligned_user_pointer) {
    struct Allocation *const allocation
        = (struct Allocation *)((char *)aligned_user_pointer
                                - sizeof(struct Allocation));
    return allocation;
}

static inline void *
base_for(void const *const aligned_user_pointer) {
    struct Allocation const *const allocation
        = (struct Allocation *)((char *)aligned_user_pointer
                                - sizeof(struct Allocation));
    return (char *)aligned_user_pointer
         - roundup(sizeof(struct Allocation), allocation->alignment);
}
