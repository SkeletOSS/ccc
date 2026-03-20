#ifndef CCC_PQ_UTIL_H
#define CCC_PQ_UTIL_H

#include <stddef.h>

#include "checkers.h"
#include "priority_queue.h"
#include "types.h"
#include "utility/stack_allocator.h"

struct Val {
    int id;
    int val;
    CCC_Priority_queue_node elem;
};

void val_update(CCC_Arguments);
CCC_Order val_order(CCC_Comparator_arguments);

/** Expects queue to have allocator permissions. */
enum Check_result insert_shuffled(
    CCC_Priority_queue *, size_t, int, CCC_Allocator const *allocator
);

/** @brief Checks that the priority queue deterministically orders elements
in strictly increasing or decreasing order as determined by the initialization
order. Copies elements between priority queues to confirm this, checking the
keys remain in the same order.
@param[in] priority_queue_pointer the priority queue to test.
@param[in] priority_queue_allocator the allocator used by the priority queue.
@param[in] type_compound_literal_array the compound literal array specifying
space for the exact same number of elements as exists in the passed priority
queue. This will be used to copy over elements and check ordering.
@return a passing check result if successful a failing check result if not.
@warning Buffers are allocated on the stack so only relatively small test cases
should be used. */
#define check_inorder_fill(                                                    \
    priority_queue_pointer,                                                    \
    priority_queue_allocator,                                                  \
    type_compound_literal_array...                                             \
)                                                                              \
    (__extension__({                                                           \
        CCC_Priority_queue *const check_priority_queue_pointer                 \
            = (priority_queue_pointer);                                        \
        enum Check_result check_inorder_result = CHECK_FAIL;                   \
        if (check_priority_queue_pointer                                       \
            && CCC_priority_queue_count(check_priority_queue_pointer).count    \
                   == (sizeof(type_compound_literal_array)                     \
                       / sizeof(*type_compound_literal_array))) {              \
            check_inorder_result = private_inorder_fill(                       \
                &(CCC_Allocator){                                              \
                    .allocate = stack_allocator_allocate,                      \
                    .context                                                   \
                    = &stack_allocator_for(type_compound_literal_array),       \
                },                                                             \
                (                                                              \
                    int[sizeof(type_compound_literal_array)                    \
                        / sizeof(*type_compound_literal_array)]                \
                ){},                                                           \
                sizeof(type_compound_literal_array)                            \
                    / sizeof(*type_compound_literal_array),                    \
                priority_queue_pointer,                                        \
                priority_queue_allocator                                       \
            );                                                                 \
        }                                                                      \
        check_inorder_result;                                                  \
    }))

/** Private for this header. Do not use directly. Use macro instead. */
enum Check_result private_inorder_fill(
    CCC_Allocator const *copy_allocator,
    int[],
    size_t,
    CCC_Priority_queue *queue,
    CCC_Allocator const *queue_allocator
);

#endif /* CCC_PQ_UTIL_H */
