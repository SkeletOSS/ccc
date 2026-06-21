#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC
#define PRIORITY_QUEUE_USING_NAMESPACE_CCC

#include "ccc/specialized/priority_queue.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "priority_queue_utility.h"
#include "utility/stack_allocator.h"

enum : int {
    HEAP_CAP = 100,
};

check_static_begin(priority_queue_test_simple_update) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    check(
        priority_queue_push(
            &priority_queue, &(struct Val){.val = 1}.elem, &allocator
        ) != NULL,
        CCC_TRUE
    );
    struct Val *to_update = priority_queue_push(
        &priority_queue, &(struct Val){.val = 5}.elem, &allocator
    );
    check(to_update != NULL, CCC_TRUE);
    check(
        priority_queue_push(
            &priority_queue, &(struct Val){.val = 3}.elem, &allocator
        ) != NULL,
        CCC_TRUE
    );
    to_update = priority_queue_update(
        &priority_queue,
        &to_update->elem,
        &(CCC_Modifier){.modify = val_update, .context = &(int){0}}
    );
    check(to_update != NULL, CCC_TRUE);
    check(validate(&priority_queue), CCC_TRUE);
    struct Val *min = priority_queue_front(&priority_queue);
    check(min != NULL, CCC_TRUE);
    check(min, to_update);
    check(min->val, 0);
    check_end();
}

check_static_begin(priority_queue_test_simple_increase) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_GREATER,
        (CCC_Comparator){.compare = val_order}
    );
    check(
        priority_queue_push(
            &priority_queue, &(struct Val){.val = 5}.elem, &allocator
        ) != NULL,
        CCC_TRUE
    );
    struct Val *to_update = priority_queue_push(
        &priority_queue, &(struct Val){.val = 1}.elem, &allocator
    );
    check(to_update != NULL, CCC_TRUE);
    check(
        priority_queue_push(
            &priority_queue, &(struct Val){.val = 3}.elem, &allocator
        ) != NULL,
        CCC_TRUE
    );
    to_update = priority_queue_increase(
        &priority_queue,
        &to_update->elem,
        &(CCC_Modifier){.modify = val_update, .context = &(int){6}}
    );
    check(to_update != NULL, CCC_TRUE);
    check(validate(&priority_queue), CCC_TRUE);
    struct Val *max = priority_queue_front(&priority_queue);
    check(max != NULL, CCC_TRUE);
    check(max, to_update);
    check(max->val, 6);
    check(
        priority_queue_increase(
            &priority_queue,
            &max->elem,
            &(CCC_Modifier){.modify = val_update, .context = &(int){7}}
        ) != NULL,
        CCC_TRUE
    );
    check(max->val, 7);
    check(validate(&priority_queue), CCC_TRUE);
    check(
        priority_queue_decrease(
            &priority_queue,
            &max->elem,
            &(CCC_Modifier){.modify = val_update, .context = &(int){4}}
        ) != NULL,
        CCC_TRUE
    );
    check(validate(&priority_queue), CCC_TRUE);
    max = priority_queue_front(&priority_queue);
    check(max != NULL, CCC_TRUE);
    check(max->val, 5);
    check_end();
}

check_static_begin(priority_queue_test_simple_decrease) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    check(
        priority_queue_push(
            &priority_queue, &(struct Val){.val = 1}.elem, &allocator
        ) != NULL,
        CCC_TRUE
    );
    struct Val *to_update = priority_queue_push(
        &priority_queue, &(struct Val){.val = 5}.elem, &allocator
    );
    check(to_update != NULL, CCC_TRUE);
    check(
        priority_queue_push(
            &priority_queue, &(struct Val){.val = 3}.elem, &allocator
        ) != NULL,
        CCC_TRUE
    );
    to_update = priority_queue_decrease(
        &priority_queue,
        &to_update->elem,
        &(CCC_Modifier){.modify = val_update, .context = &(int){0}}
    );
    check(to_update != NULL, CCC_TRUE);
    check(validate(&priority_queue), CCC_TRUE);
    struct Val *min = priority_queue_front(&priority_queue);
    check(min != NULL, CCC_TRUE);
    check(min->val, 0);
    check(
        priority_queue_decrease(
            &priority_queue,
            &min->elem,
            &(CCC_Modifier){.modify = val_update, .context = &(int){-1}}
        ) != NULL,
        CCC_TRUE
    );
    check(validate(&priority_queue), CCC_TRUE);
    check(min->val, -1);
    check_end();
}

check_static_begin(priority_queue_test_insert_iterate_pop) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (HEAP_CAP + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    size_t pop_count = 0;
    while (!priority_queue_is_empty(&priority_queue)) {
        check(pop(&priority_queue, &allocator), CCC_RESULT_OK);
        ++pop_count;
        check(validate(&priority_queue), true);
    }
    check(pop_count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_removal) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        if (i->val > limit) {
            (void)priority_queue_extract(&priority_queue, &i->elem);
            check(validate(&priority_queue), true);
        }
    }
    check_end();
}

check_static_begin(priority_queue_test_priority_update_min_queue) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int backoff = i->val / 2;
        if (i->val > limit) {
            check(
                priority_queue_update(
                    &priority_queue,
                    &i->elem,
                    &(CCC_Modifier){
                        .modify = val_update,
                        .context = &backoff,
                    }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_update_min_queue_with) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        int backoff = val_array[val].val / 2;
        if (val_array[val].val > limit) {
            struct Val *i = &val_array[val];
            check(
                priority_queue_update_with(
                    &priority_queue, i, { i->val = backoff; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_increase_min_queue) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int inc = (limit * 2) + 1;
        int dec = (i->val / 2) - 1;
        if (i->val > limit && dec < i->val) {
            check(
                priority_queue_decrease(
                    &priority_queue,
                    &i->elem,
                    &(CCC_Modifier){
                        .modify = val_update,
                        .context = &dec,
                    }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        } else if (i->val < limit && inc > i->val) {
            check(
                priority_queue_increase(
                    &priority_queue,
                    &i->elem,
                    &(CCC_Modifier){
                        .modify = val_update,
                        .context = &inc,
                    }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_increase_min_queue_with) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        int inc = (limit * 2) + 1;
        int dec = (val_array[val].val / 2) - 1;
        struct Val *i = &val_array[val];
        if (val_array[val].val > limit && dec < val_array[val].val) {
            check(
                priority_queue_decrease_with(
                    &priority_queue, i, { i->val = dec; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        } else if (val_array[val].val < limit && inc > val_array[val].val) {
            check(
                priority_queue_increase_with(
                    &priority_queue, i, { i->val = inc; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_decrease_min_queue) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int inc = (limit * 2) + 1;
        int dec = (i->val / 2) - 1;
        if (i->val < limit && inc > i->val) {
            check(
                priority_queue_increase(
                    &priority_queue,
                    &i->elem,
                    &(CCC_Modifier){
                        .modify = val_update,
                        .context = &inc,
                    }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        } else if (i->val > limit && dec < i->val) {
            check(
                priority_queue_decrease(
                    &priority_queue,
                    &i->elem,
                    &(CCC_Modifier){
                        .modify = val_update,
                        .context = &dec,
                    }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_decrease_min_queue_with) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (size_t i = 0; i < HEAP_CAP; ++i) {
        /* Force duplicates. */
        struct Val const *const pushed = push(
            &priority_queue,
            &(struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % ((HEAP_CAP * 2) + 1)),
                .id = (int)i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&priority_queue), true);
    }
    int const limit = HEAP_CAP / 2;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int inc = (limit * 2) + 1;
        int dec = (i->val / 2) - 1;
        if (i->val < limit && inc > i->val) {
            check(
                priority_queue_increase_with(
                    &priority_queue, i, { i->val = inc; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        } else if (i->val > limit && dec < i->val) {
            check(
                priority_queue_decrease_with(
                    &priority_queue, i, { i->val = dec; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

int
main(void) {
    return check_run(
        priority_queue_test_simple_update(),
        priority_queue_test_simple_increase(),
        priority_queue_test_simple_decrease(),
        priority_queue_test_insert_iterate_pop(),
        priority_queue_test_priority_update_min_queue(),
        priority_queue_test_priority_update_min_queue_with(),
        priority_queue_test_priority_removal(),
        priority_queue_test_priority_increase_min_queue(),
        priority_queue_test_priority_increase_min_queue_with(),
        priority_queue_test_priority_decrease_min_queue(),
        priority_queue_test_priority_decrease_min_queue_with()
    );
}
