#ifndef BUF_UTIL_H
#define BUF_UTIL_H

#include <stddef.h>

#include "ccc/flat_buffer.h"
#include "ccc/types.h"

/** Sorts a Flat_buffer according to its size and the provided order assuming
that elements are stored from indices [0, N) where N is the size not capacity of
the buffer. CCC_ORDER_LESSER orders elements in non-decreasing order from index
`[0,N)`. CCC_ORDER_GREATER orders elements in non-increasing order from index
`[0,N)`. Requires a comparison function from the user and one swap slot
equivalent to the size of an element stored in the buffer. Elements are sorted
in a non-decreasing order. Therefore if a non-increasing list is needed simply
reverse the return of the comparison function for non-equivalent values. */
CCC_Result quicksort(
    CCC_Flat_buffer *b,
    void *swap,
    CCC_Order order,
    CCC_Comparator const *comparator
);

/** Compares the Flat_buffer contents as left hand side to the provided sequence
of elements as the right hand side. The sequence type must match the type stored
in the Flat_buffer because that type will be used to iterate through the
sequence. */
CCC_Order
buforder(CCC_Flat_buffer const *left, size_t right_count, void const *right);

/** Appends the provided range into the buffer. If the range will exceed
capacity of a fixed size buffer, only those elements which fit will be pushed
and a failure will be returned. If resizing is allowed the full range should
be appended unless reallocation fails in which case an error is returned. */
CCC_Result append_range(
    CCC_Flat_buffer *b,
    size_t range_count,
    void const *range,
    CCC_Allocator const *allocator
);

/** Returns max int between a and b. Ties go to a. */
static inline int
maxint(int a, int b) {
    return a > b ? a : b;
}

#endif /* BUF_UTIL_H */
