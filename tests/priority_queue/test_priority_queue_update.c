#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC

#include "checkers.h"
#include "priority_queue.h"
#include "priority_queue_utility.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

enum : int {
    HEAP_CAP = 100,
};

check_static_begin(priority_queue_test_insert_iterate_pop) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    while (!CCC_priority_queue_is_empty(&priority_queue)) {
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
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        if (i->val > limit) {
            (void)CCC_priority_queue_extract(&priority_queue, &i->elem);
            check(validate(&priority_queue), true);
        }
    }
    check_end();
}

check_static_begin(priority_queue_test_priority_update) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int backoff = i->val / 2;
        if (i->val > limit) {
            check(
                CCC_priority_queue_update(
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
    check(CCC_priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_update_with) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        int backoff = val_array[val].val / 2;
        if (val_array[val].val > limit) {
            struct Val *i = &val_array[val];
            check(
                CCC_priority_queue_update_with(
                    &priority_queue, i, { i->val = backoff; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(CCC_priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_increase) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int inc = (limit * 2) + 1;
        int dec = (i->val / 2) - 1;
        if (i->val > limit && dec < i->val) {
            check(
                CCC_priority_queue_decrease(
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
                CCC_priority_queue_increase(
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
    check(CCC_priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_increase_with) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        int inc = (limit * 2) + 1;
        int dec = (val_array[val].val / 2) - 1;
        struct Val *i = &val_array[val];
        if (val_array[val].val > limit && dec < val_array[val].val) {
            check(
                CCC_priority_queue_decrease_with(
                    &priority_queue, i, { i->val = dec; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        } else if (val_array[val].val < limit && inc > val_array[val].val) {
            check(
                CCC_priority_queue_increase_with(
                    &priority_queue, i, { i->val = inc; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(CCC_priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_decrease) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int inc = (limit * 2) + 1;
        int dec = (i->val / 2) - 1;
        if (i->val < limit && inc > i->val) {
            check(
                CCC_priority_queue_increase(
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
                CCC_priority_queue_decrease(
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
    check(CCC_priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

check_static_begin(priority_queue_test_priority_decrease_with) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[HEAP_CAP]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
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
    int const limit = 400;
    struct Val *const val_array
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t val = 0; val < HEAP_CAP; ++val) {
        struct Val *const i = &val_array[val];
        int inc = (limit * 2) + 1;
        int dec = (i->val / 2) - 1;
        if (i->val < limit && inc > i->val) {
            check(
                CCC_priority_queue_increase_with(
                    &priority_queue, i, { i->val = inc; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        } else if (i->val > limit && dec < i->val) {
            check(
                CCC_priority_queue_decrease_with(
                    &priority_queue, i, { i->val = dec; }
                ) != NULL,
                true
            );
            check(validate(&priority_queue), true);
        }
    }
    check(CCC_priority_queue_count(&priority_queue).count, HEAP_CAP);
    check_end();
}

int
main(void) {
    return check_run(
        priority_queue_test_insert_iterate_pop(),
        priority_queue_test_priority_update(),
        priority_queue_test_priority_update_with(),
        priority_queue_test_priority_removal(),
        priority_queue_test_priority_increase(),
        priority_queue_test_priority_increase_with(),
        priority_queue_test_priority_decrease(),
        priority_queue_test_priority_decrease_with()
    );
}
