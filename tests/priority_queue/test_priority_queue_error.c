#include "ccc/priority_queue.h"
#include "ccc/types.h"
#include "checkers.h"
#include "priority_queue_utility.h"
#include "utility/stack_allocator.h"

check_static_begin(priority_queue_test_status_null) {
    check(CCC_priority_queue_front(NULL), NULL);
    check_end();
}

check_static_begin(priority_queue_test_insert_null) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((char[1]){}),
    };
    CCC_Priority_queue pq = CCC_priority_queue_default(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    check(
        CCC_priority_queue_push(NULL, &(struct Val){}.elem, &allocator), NULL
    );
    check(CCC_priority_queue_push(&pq, NULL, &allocator), NULL);
    check(CCC_priority_queue_push(&pq, &(struct Val){}.elem, NULL), NULL);
    check(CCC_priority_queue_push(&pq, &(struct Val){}.elem, &allocator), NULL);
    check_end();
}

int
main(void) {
    return check_run(
        priority_queue_test_status_null(), priority_queue_test_insert_null(),
    );
}
