#include <stddef.h>
#include <stdlib.h>

#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
#define BUFFER_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "buffer.h"
#include "checkers.h"
#include "flat_priority_queue.h"
#include "flat_priority_queue_utility.h"
#include "sort.h"
#include "traits.h"
#include "types.h"
#include "utility/allocate.h"

CCC_Order
val_order(CCC_Type_comparator_context const order) {
    struct Val const *const left = order.type_left;
    struct Val const *const right = order.type_right;
    return (left->val > right->val) - (left->val < right->val);
}

void
val_update(CCC_Type_context const u) {
    struct Val *const old = u.type;
    old->val = *(int *)u.context;
}

size_t
rand_range(size_t const min, size_t const max) {
    /* NOLINTNEXTLINE */
    return min + (rand() / (RAND_MAX / (max - min + 1) + 1));
}

check_begin(insert_shuffled, CCC_Flat_priority_queue *const priority_queue,
            size_t const size, int const larger_prime) {
    /* Math magic ahead so that we iterate over every index
       eventually but in a shuffled order. Not necessarily
       random but a repeatable sequence that makes it
       easier to debug if something goes wrong. Think
       of the prime number as a random seed, kind of. */
    size_t shuffled_index = larger_prime % size;
    for (size_t i = 0; i < size; ++i) {
        check(push(priority_queue,
                   &(struct Val){
                       .id = (int)shuffled_index,
                       .val = (int)shuffled_index,
                   },
                   &(struct Val){})
                  != NULL,
              CCC_TRUE);
        check(CCC_flat_priority_queue_count(priority_queue).count, i + 1);
        check(validate(priority_queue), true);
        shuffled_index = (shuffled_index + larger_prime) % size;
    }
    check(CCC_flat_priority_queue_count(priority_queue).count, size);
    check_end();
}

/* Iterative inorder traversal to check the heap is sorted. */
check_begin(inorder_fill, int vals[const], size_t const size,
            CCC_Flat_priority_queue const *const flat_priority_queue) {
    if (CCC_flat_priority_queue_count(flat_priority_queue).count != size) {
        return CHECK_FAIL;
    }
    CCC_Buffer copy = CCC_buffer_with_allocator(struct Val, std_allocate);
    CCC_Result r
        = buffer_copy(&copy, &flat_priority_queue->buffer, std_allocate);
    check(r, CCC_RESULT_OK);
    r = CCC_sort_heapsort(&copy, flat_priority_queue->order,
                          flat_priority_queue->compare, &(struct Val){});
    check(r, CCC_RESULT_OK);
    check(CCC_buffer_is_empty(&copy), CCC_FALSE);
    vals[0] = *CCC_buffer_front_as(&copy, int);
    size_t i = 1;
    for (struct Val const *prev = begin(&copy), *v = next(&copy, prev);
         v != end(&copy); prev = v, v = next(&copy, v)) {
        check(prev->val <= v->val, CCC_TRUE);
        vals[i++] = v->val;
    }
    check(i, flat_priority_queue_count(flat_priority_queue).count);
    check_end(clear_and_free(&copy, NULL););
}
