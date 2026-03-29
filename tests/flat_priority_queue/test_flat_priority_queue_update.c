#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC

#include "checkers.h"
#include "flat_priority_queue.h"
#include "flat_priority_queue_utility.h"
#include "traits.h"
#include "types.h"

check_static_begin(flat_priority_queue_test_insert_iterate_pop) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand(1);
    enum : int {
        NUM_NODES = 500,
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_storage(
            CCC_ORDER_LESSER,
            (CCC_Comparator){.compare = val_order},
            (struct Val[NUM_NODES + 1]){}
        );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        struct Val val = {
            /* NOLINTNEXTLINE */
            .val = (int)((size_t)rand() % (NUM_NODES + 1)),
            .id = (int)i,
        };
        check(
            push(
                &flat_priority_queue, &val, &(struct Val){}, &(CCC_Allocator){}
            ) != NULL,
            true
        );
        check(validate(&flat_priority_queue), true);
    }
    size_t pop_count = 0;
    while (!is_empty(&flat_priority_queue)) {
        (void)pop(&flat_priority_queue, &(struct Val){});
        ++pop_count;
        check(validate(&flat_priority_queue), true);
    }
    check(pop_count, NUM_NODES);
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_removal) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(*vals)),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t seen = 0, remaining = NUM_NODES; seen < remaining; ++seen) {
        struct Val *cur = &vals[seen];
        if (cur->val > limit) {
            (void)erase(&flat_priority_queue, cur, &(struct Val){});
            check(validate(&flat_priority_queue), true);
            --remaining;
        }
    }
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_update) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(vals[0])),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t val = 0; val < NUM_NODES; ++val) {
        struct Val *cur = &vals[val];
        int backoff = cur->val / 2;
        if (cur->val > limit) {
            struct Val const *const updated = update(
                &flat_priority_queue,
                cur,
                &(struct Val){},
                &(CCC_Modifier){
                    .modify = val_update,
                    .context = &backoff,
                }
            );
            check(updated != NULL, true);
            check(updated->val, backoff);
            check(validate(&flat_priority_queue), true);
        }
    }
    check(count(&flat_priority_queue).count, NUM_NODES);
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_update_with) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(vals[0])),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t val = 0; val < NUM_NODES; ++val) {
        int backoff = vals[val].val / 2;
        if (vals[val].val > limit) {
            struct Val *updated = &vals[val];
            updated = CCC_flat_priority_queue_update_with(
                &flat_priority_queue, updated, { updated->val = backoff; }
            );
            check(updated != NULL, true);
            check(updated->val, backoff);
            check(validate(&flat_priority_queue), true);
        }
    }
    check(count(&flat_priority_queue).count, NUM_NODES);
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_increase) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(vals[0])),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t val = 0; val < NUM_NODES; ++val) {
        struct Val *cur = &vals[val];
        int greater = cur->val * 2;
        if (cur->val < limit) {
            struct Val const *const updated = increase(
                &flat_priority_queue,
                cur,
                &(struct Val){},
                &(CCC_Modifier){
                    .modify = val_update,
                    .context = &greater,
                }
            );
            check(updated != NULL, true);
            check(updated->val, greater);
            check(validate(&flat_priority_queue), true);
        }
    }
    check(count(&flat_priority_queue).count, NUM_NODES);
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_increase_with) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(vals[0])),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t val = 0; val < NUM_NODES; ++val) {
        int greater = vals[val].val * 2;
        if (vals[val].val < limit) {
            struct Val *updated = &vals[val];
            updated = CCC_flat_priority_queue_increase_with(
                &flat_priority_queue, updated, { updated->val = greater; }
            );
            check(updated != NULL, true);
            check(updated->val, greater);
            check(validate(&flat_priority_queue), true);
        }
    }
    check(count(&flat_priority_queue).count, NUM_NODES);
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_decrease) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(vals[0])),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t val = 0; val < NUM_NODES; ++val) {
        struct Val *cur = &vals[val];
        int backoff = cur->val / 2;
        if (cur->val > limit) {
            struct Val const *const updated = decrease(
                &flat_priority_queue,
                cur,
                &(struct Val){},
                &(CCC_Modifier){
                    .modify = val_update,
                    .context = &backoff,
                }
            );
            check(updated != NULL, true);
            check(updated->val, backoff);
            check(validate(&flat_priority_queue), true);
        }
    }
    check(count(&flat_priority_queue).count, NUM_NODES);
    check_end();
}

check_static_begin(flat_priority_queue_test_priority_decrease_with) {
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        NUM_NODES = 500,
    };
    struct Val vals[NUM_NODES + 1];
    CCC_Flat_priority_queue flat_priority_queue = CCC_flat_priority_queue_for(
        struct Val,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order},
        (sizeof(vals) / sizeof(vals[0])),
        vals
    );
    for (size_t i = 0; i < NUM_NODES; ++i) {
        /* Force duplicates. */
        struct Val const *res = CCC_flat_priority_queue_emplace(
            &flat_priority_queue,
            &(CCC_Allocator){},
            (struct Val){
                /* NOLINTNEXTLINE */
                .val = (int)((size_t)rand() % (NUM_NODES + 1)),
                .id = (int)i,
            }
        );
        check(res != NULL, true);
        check(validate(&flat_priority_queue), true);
    }
    int const limit = NUM_NODES / 2;
    for (size_t val = 0; val < NUM_NODES; ++val) {
        int backoff = vals[val].val / 2;
        if (vals[val].val > limit) {
            struct Val *updated = &vals[val];
            updated = CCC_flat_priority_queue_decrease_with(
                &flat_priority_queue, updated, { updated->val = backoff; }
            );
            check(updated != NULL, true);
            check(updated->val, backoff);
            check(validate(&flat_priority_queue), true);
        }
    }
    check(count(&flat_priority_queue).count, NUM_NODES);
    check_end();
}

int
main(void) {
    return check_run(
        flat_priority_queue_test_insert_iterate_pop(),
        flat_priority_queue_test_priority_update(),
        flat_priority_queue_test_priority_update_with(),
        flat_priority_queue_test_priority_increase(),
        flat_priority_queue_test_priority_increase_with(),
        flat_priority_queue_test_priority_decrease(),
        flat_priority_queue_test_priority_decrease_with(),
        flat_priority_queue_test_priority_removal()
    );
}
