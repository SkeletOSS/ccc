#ifndef FDEQ_UTIL_H
#define FDEQ_UTIL_H

#include <stddef.h>

#include "buffer.h"
#include "checkers.h"
#include "flat_double_ended_queue.h"

CCC_Order compare_ints(CCC_Comparator_arguments arguments);

enum Check_result create_queue(CCC_Flat_double_ended_queue *q,
                               CCC_Buffer const *range,
                               CCC_Allocator const *allocator);

enum Check_result check_order(CCC_Flat_double_ended_queue const *int_q,
                              CCC_Buffer const *int_order);

#endif /* FDEQ_UTIL_H */
