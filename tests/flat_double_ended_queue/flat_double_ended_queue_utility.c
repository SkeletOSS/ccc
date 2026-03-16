#include <stddef.h>
#include <stdio.h>

#define TRAITS_USING_NAMESPACE_CCC
#define FLAT_DOUBLE_ENDED_QUEUE_USING_NAMESPACE_CCC

#include "buffer.h"
#include "checkers.h"
#include "flat_double_ended_queue.h"
#include "flat_double_ended_queue_utility.h"
#include "traits.h"
#include "types.h"

CCC_Order
compare_ints(CCC_Comparator_arguments arguments) {
    int const *const lhs = arguments.type_left;
    int const *const rhs = arguments.type_right;
    return (*lhs > *rhs) - (*lhs < *rhs);
}

check_begin(create_queue, Flat_double_ended_queue *const q,
            CCC_Buffer const *const range,
            CCC_Allocator_context const *const allocator) {
    check(q != NULL && range != NULL && allocator != NULL, true);
    if (CCC_buffer_count(range).count) {
        CCC_Result const res
            = flat_double_ended_queue_push_back_range(q, range, allocator);
        check(res, CCC_RESULT_OK);
        check(validate(q), true);
    }
    check_end();
}

check_begin(check_order, Flat_double_ended_queue const *const int_q,
            CCC_Buffer const *const int_order) {
    check(int_q != NULL && int_order != NULL, true);
    check(CCC_buffer_sizeof_type(&int_q->buffer).count,
          CCC_buffer_sizeof_type(int_order).count);
    int const *q_element = begin(int_q);
    for (int const *order_element = begin(int_order);
         q_element != end(int_q) && order_element != end(int_order);
         q_element = next(int_q, q_element),
                   order_element = next(int_order, order_element)) {
        check(q_element == NULL, false);
        check(*q_element, *order_element);
    }
    q_element = reverse_begin(int_q);
    for (int const *order_element = reverse_begin(int_order);
         q_element != reverse_end(int_q)
         && order_element != reverse_end(int_order);
         q_element = reverse_next(int_q, q_element),
                   order_element = reverse_next(int_order, order_element)) {
        check(q_element == NULL, false);
        check(*q_element, *order_element);
    }
    check_fail_end({
        (void)fprintf(stderr, "%sCHECK: (int[%zu]){", CHECK_GREEN,
                      CCC_buffer_count(int_order).count);
        for (int const *i = begin(int_order); i != end(int_order);
             i = next(int_order, i)) {
            (void)fprintf(stderr, "%d, ", *i);
        }
        (void)fprintf(stderr, "}\n%s", CHECK_NONE);
        (void)fprintf(stderr, "%sCHECK_ERROR:%s (int[%zu]){", CHECK_RED,
                      CHECK_GREEN, CCC_buffer_count(int_order).count);
        q_element = begin(int_q);
        for (int const *j = begin(int_order);
             j != end(int_order) && q_element != end(int_q);
             j = next(int_order, j), q_element = next(int_q, q_element)) {
            if (!q_element) {
                return CHECK_STATUS;
            }
            if (*j == *q_element) {
                (void)fprintf(stderr, "%s%d, %s", CHECK_GREEN, *j, CHECK_NONE);
            } else {
                (void)fprintf(stderr, "%s%d, %s", CHECK_RED, *q_element,
                              CHECK_NONE);
            }
        }
        for (; q_element != end(int_q); q_element = next(int_q, q_element)) {
            (void)fprintf(stderr, "%s%d, %s", CHECK_RED, *q_element,
                          CHECK_NONE);
        }
        (void)fprintf(stderr, "%s}\n%s", CHECK_GREEN, CHECK_NONE);
    });
}
