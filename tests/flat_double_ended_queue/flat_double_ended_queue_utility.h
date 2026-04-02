#ifndef FDEQ_UTIL_H
#define FDEQ_UTIL_H

#include <stddef.h>

#include "ccc/flat_buffer.h"
#include "ccc/flat_double_ended_queue.h"
#include "ccc/types.h"
#include "checkers.h"

CCC_Order compare_ints(CCC_Comparator_arguments arguments);

enum Check_result create_queue(
    CCC_Flat_double_ended_queue *q,
    CCC_Flat_buffer const *range,
    CCC_Allocator const *allocator
);

enum Check_result check_order(
    CCC_Flat_double_ended_queue const *int_q, CCC_Flat_buffer const *int_order
);

#endif /* FDEQ_UTIL_H */
