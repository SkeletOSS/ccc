#include <stddef.h>
#include <string.h>

#define BUFFER_USING_NAMESPACE_CCC

#include "buffer_utility.h"
#include "ccc/buffer.h"
#include "ccc/types.h"

static inline void
swap(void *const temp, void *const a, void *const b, size_t const ab_size) {
    if (a != b) {
        (void)memcpy(temp, a, ab_size);
        (void)memcpy(a, b, ab_size);
        (void)memcpy(b, temp, ab_size);
    }
}

/** For now we do not use randomization for the partition selection meaning we
fall prey to the O(N^2) worst case runtime more easily. With void and iterators
it is complicated to select a randomized slot but it would still be possible.*/
static void *
partition(Buffer *const b, void *const temp, void *lo, void *hi,
          CCC_Order const out_of_order,
          CCC_Comparator_context const *const comparator) {
    void *const pivot_val = hi;
    void *i = lo;
    for (void *j = lo; j < hi; j = buffer_next(b, j)) {
        CCC_Order const order_result
            = comparator->compare((CCC_Comparator_arguments){
                .type_left = j,
                .type_right = pivot_val,
                .context = comparator->context,
            });
        if (order_result != out_of_order) {
            swap(temp, i, j, b->sizeof_type);
            i = buffer_next(b, i);
        }
    }
    swap(temp, i, hi, b->sizeof_type);
    return i;
}

/* NOLINTBEGIN(*misc-no-recursion*) */

/** Canonical C quicksort. See Wikipedia for the pseudocode or a breakdown of
different trade offs. See CLRS extra problems for eliminating two recursive
calls and reducing stack space to O(log(N)).

    https://en.wikipedia.org/wiki/Quicksort

This implementation does not try to be special or efficient. In fact because
this is meant to test the Buffer container, it uses iterators only to swap and
sort data. This is a fun way to test that part of the Buffer interface for
correctness and turns out to be pretty nice and clean. */
static void
sort_rec(Buffer *const b, void *const temp, void *lo, void *hi,
         CCC_Order const out_of_order,
         CCC_Comparator_context const *const comparator) {
    while (lo < hi) {
        void const *const pivot_i
            = partition(b, temp, lo, hi, out_of_order, comparator);
        if ((char const *)pivot_i - (char const *)lo
            < (char const *)hi - (char const *)pivot_i) {
            sort_rec(b, temp, lo, buffer_reverse_next(b, pivot_i), out_of_order,
                     comparator);
            lo = buffer_next(b, pivot_i);
        } else {
            sort_rec(b, temp, buffer_next(b, pivot_i), hi, out_of_order,
                     comparator);
            hi = buffer_reverse_next(b, pivot_i);
        }
    }
}

/* NOLINTEND(*misc-no-recursion*) */

/** Sorts the provided Buffer in average time O(N * log(N)) and O(log(N))
stack space. This implementation does not try to be hyper efficient. In fact, we
test out using iterators here rather than indices. */
CCC_Result
quicksort(CCC_Buffer *const b, void *const swap, CCC_Order const order,
          CCC_Comparator_context const *const comparator) {
    if (!b || !comparator || !comparator->compare || !swap
        || (order != CCC_ORDER_LESSER && order != CCC_ORDER_GREATER)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (buffer_count(b).count) {
        CCC_Order const out_of_order
            = order == CCC_ORDER_LESSER ? CCC_ORDER_GREATER : CCC_ORDER_LESSER;
        sort_rec(b, swap, buffer_begin(b), buffer_reverse_begin(b),
                 out_of_order, comparator);
    }
    return CCC_RESULT_OK;
}

CCC_Order
buforder(CCC_Buffer const *const left, size_t const right_count,
         void const *const right) {
    size_t const type_size = buffer_sizeof_type(left).count;
    size_t const buffer_size = buffer_count(left).count;
    if (buffer_size < right_count) {
        return CCC_ORDER_LESSER;
    }
    if (buffer_size < right_count) {
        return CCC_ORDER_GREATER;
    }
    int const order
        = memcmp(buffer_begin(left), right, buffer_size * type_size);
    if (order == 0) {
        return CCC_ORDER_EQUAL;
    }
    if (order < 0) {
        return CCC_ORDER_LESSER;
    }
    return CCC_ORDER_GREATER;
}

CCC_Result
append_range(CCC_Buffer *const b, size_t range_count, void const *const range,
             CCC_Allocator_context const *const allocator) {
    unsigned char const *p = range;
    size_t const sizeof_type = buffer_sizeof_type(b).count;
    while (range_count--) {
        void const *const appended = CCC_buffer_push_back(b, p, allocator);
        if (!appended) {
            return CCC_RESULT_FAIL;
        }
        p += sizeof_type;
    }
    return CCC_RESULT_OK;
}
