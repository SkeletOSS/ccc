#include "ccc/specialized/priority_queue.h"
#include "ccc/types.h"
#include "checkers.h"
#include "priority_queue_utility.h"
#include "utility/stack_allocator.h"

check_static_begin(priority_queue_test_status_null) {
    check(CCC_priority_queue_front(NULL), NULL);
    check(CCC_priority_queue_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_priority_queue_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
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

check_static_begin(priority_queue_test_update_null) {
    CCC_Priority_queue pq = CCC_priority_queue_default(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    check(
        CCC_priority_queue_update(
            NULL, &(struct Val){}.elem, &(CCC_Modifier){}
        ),
        NULL
    );
    check(CCC_priority_queue_update(&pq, NULL, &(CCC_Modifier){}), NULL);
    check(CCC_priority_queue_update(&pq, &(struct Val){}.elem, NULL), NULL);
    check(
        CCC_priority_queue_increase(
            NULL, &(struct Val){}.elem, &(CCC_Modifier){}
        ),
        NULL
    );
    check(CCC_priority_queue_increase(&pq, NULL, &(CCC_Modifier){}), NULL);
    check(CCC_priority_queue_increase(&pq, &(struct Val){}.elem, NULL), NULL);
    check(
        CCC_priority_queue_decrease(
            NULL, &(struct Val){}.elem, &(CCC_Modifier){}
        ),
        NULL
    );
    check(CCC_priority_queue_decrease(&pq, NULL, &(CCC_Modifier){}), NULL);
    check(CCC_priority_queue_decrease(&pq, &(struct Val){}.elem, NULL), NULL);
    check_end();
}

check_static_begin(priority_queue_test_erase_null) {
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
    check(CCC_priority_queue_pop(NULL, &allocator), CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_priority_queue_pop(&pq, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_priority_queue_extract(&pq, NULL), NULL);
    check(
        CCC_priority_queue_erase(&pq, NULL, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_priority_queue_erase(&pq, &(struct Val){}.elem, NULL),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_priority_queue_clear(NULL, &(CCC_Destructor){}, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_priority_queue_clear(&pq, NULL, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_priority_queue_clear(&pq, &(CCC_Destructor){}, NULL),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check_end();
}

int
main(void) {
    return check_run(
        priority_queue_test_status_null(),
        priority_queue_test_insert_null(),
        priority_queue_test_update_null(),
        priority_queue_test_erase_null(),
    );
}
