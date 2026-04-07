#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC
#define FLAT_BITSET_USING_NAMESPACE_CCC

#include "ccc/flat_bitset.h"
#include "ccc/priority_queue.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "priority_queue_utility.h"
#include "utility/stack_allocator.h"
#include "utility/std_allocator.h"

enum : int {
    STANDARD_CAP = 50,
    LARGE_CAP = 99,
    WEAK_SRAND_HEAP_CAP = 1000,
};

check_static_begin(priority_queue_test_insert_remove_key_value_four_dups) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[4]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    for (int i = 0; i < 4; ++i) {
        check(
            push(&priority_queue, &(struct Val){.val = 0}.elem, &allocator)
                != NULL,
            true
        );
        check(validate(&priority_queue), true);
        size_t const size = (size_t)i + 1;
        check(CCC_priority_queue_count(&priority_queue).count, size);
    }
    check(CCC_priority_queue_count(&priority_queue).count, (size_t)4);
    for (int i = 0; i < 4; ++i) {
        check(pop(&priority_queue, &allocator), CCC_RESULT_OK);
        check(validate(&priority_queue), true);
    }
    check(CCC_priority_queue_count(&priority_queue).count, (size_t)0);
    check_end();
}

check_static_begin(priority_queue_test_insert_extract_shuffled) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[STANDARD_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int const prime = 53;
    check(insert_shuffled(&queue, STANDARD_CAP, prime, &allocator), CHECK_PASS);
    struct Val const *min = front(&queue);
    check(min->val, 0);
    enum Check_result const result
        = check_inorder_fill(&queue, &allocator, (struct Val[STANDARD_CAP]){});
    check(result, CHECK_PASS);
    /* Now let's delete everything with no errors. */
    struct Stack_allocator *const stack_meta = allocator.context;
    struct Val const *const end
        = (struct Val *)stack_meta->blocks + STANDARD_CAP;
    for (struct Val *i = stack_meta->blocks; i != end; ++i) {
        (void)CCC_priority_queue_extract(&queue, &i->elem);
        check(validate(&queue), true);
    }
    check(CCC_priority_queue_count(&queue).count, (size_t)0);
    check_end();
}

check_static_begin(priority_queue_test_insert_erase_shuffled) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[STANDARD_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int const prime = 53;
    check(insert_shuffled(&queue, STANDARD_CAP, prime, &allocator), CHECK_PASS);
    struct Val const *min = front(&queue);
    check(min->val, 0);
    enum Check_result const result
        = check_inorder_fill(&queue, &allocator, (struct Val[STANDARD_CAP]){});
    check(result, CHECK_PASS);
    /* Now let's delete everything with no errors. */
    struct Stack_allocator *const stack_meta = allocator.context;
    struct Val const *const end
        = (struct Val *)stack_meta->blocks + STANDARD_CAP;
    for (struct Val *i = stack_meta->blocks; i != end; ++i) {
        (void)CCC_priority_queue_erase(&queue, &i->elem, &allocator);
        check(validate(&queue), true);
    }
    check(CCC_priority_queue_count(&queue).count, (size_t)0);
    check_end();
}

check_static_begin(priority_queue_test_pop_max) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[STANDARD_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int const prime = 53;
    check(insert_shuffled(&queue, STANDARD_CAP, prime, &allocator), CHECK_PASS);
    struct Val const *min = front(&queue);
    check(min->val, 0);
    check(
        check_inorder_fill(&queue, &allocator, (struct Val[STANDARD_CAP]){}),
        CHECK_PASS
    );
    /* Now let's pop from the front of the queue until empty. */
    int prev_val = INT_MIN;
    for (size_t i = 0; i < STANDARD_CAP; ++i) {
        struct Val const *front = front(&queue);
        check(front->val > prev_val, true);
        prev_val = front->val;
        check(pop(&queue, &allocator), CCC_RESULT_OK);
    }
    check(CCC_priority_queue_is_empty(&queue), true);
    check_end();
}

check_static_begin(priority_queue_test_pop_min) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[STANDARD_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int const prime = 53;
    check(insert_shuffled(&queue, STANDARD_CAP, prime, &allocator), CHECK_PASS);
    struct Val const *min = front(&queue);
    check(min->val, 0);
    check(
        check_inorder_fill(&queue, &allocator, (struct Val[STANDARD_CAP]){}),
        CHECK_PASS
    );
    /* Now let's pop from the front of the queue until empty. */
    int prev_val = INT_MIN;
    for (size_t i = 0; i < STANDARD_CAP; ++i) {
        struct Val const *front = front(&queue);
        check(front->val > prev_val, true);
        prev_val = front->val;
        check(pop(&queue, &allocator), CCC_RESULT_OK);
    }
    check(CCC_priority_queue_is_empty(&queue), true);
    check_end();
}

check_static_begin(priority_queue_test_delete_prime_shuffle_duplicates) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[LARGE_CAP]){}),
    };

    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int const prime = 101;
    /* Make the prime shuffle shorter than size for many duplicates. */
    int const less = 77;
    int shuffled_index = prime % (LARGE_CAP - less);
    for (int i = 0; i < LARGE_CAP; ++i) {
        struct Val const *const pushed = push(
            &queue,
            &(struct Val){.val = shuffled_index, .id = i}.elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&queue), true);
        size_t const s = (size_t)i + 1;
        check(CCC_priority_queue_count(&queue).count, s);
        /* Shuffle like this only on insertions to create more dups. */
        shuffled_index = (shuffled_index + prime) % (LARGE_CAP - less);
    }

    shuffled_index = prime % (LARGE_CAP - less);
    size_t cur_size = LARGE_CAP;
    struct Stack_allocator *const stack_meta = allocator.context;
    struct Val *const val_array = (struct Val *)stack_meta->blocks;
    for (int i = 0; i < LARGE_CAP; ++i) {
        (void)CCC_priority_queue_extract(
            &queue, &val_array[shuffled_index].elem
        );
        check(validate(&queue), true);
        --cur_size;
        check(CCC_priority_queue_count(&queue).count, cur_size);
        /* Shuffle normally here so we only remove each elem once. */
        shuffled_index = (shuffled_index + prime) % LARGE_CAP;
    }
    check_end();
}

check_static_begin(priority_queue_test_prime_shuffle) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[STANDARD_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int const prime = 53;
    int const less = 10;
    /* We want the tree to have a smattering of duplicates so
       reduce the shuffle range so it will repeat some values. */
    int shuffled_index = prime % (STANDARD_CAP - less);
    for (int i = 0; i < STANDARD_CAP; ++i) {
        struct Val const *const pushed = push(
            &queue,
            &(struct Val){.val = shuffled_index, .id = shuffled_index}.elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&queue), true);
        shuffled_index = (shuffled_index + prime) % (STANDARD_CAP - less);
    }
    /* Now we go through and free all the elements in order but
       their positions in the tree will be somewhat random */
    size_t cur_size = STANDARD_CAP;
    struct Stack_allocator *const stack_meta = allocator.context;
    struct Val *const val_array = (struct Val *)stack_meta->blocks;
    for (int i = 0; i < STANDARD_CAP; ++i) {
        (void)CCC_priority_queue_erase(&queue, &val_array[i].elem, &allocator);
        check(validate(&queue), true);
        --cur_size;
        check(CCC_priority_queue_count(&queue).count, cur_size);
    }
    check_end();
}

check_static_begin(priority_queue_test_weak_srand) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[WEAK_SRAND_HEAP_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    for (int i = 0; i < WEAK_SRAND_HEAP_CAP; ++i) {
        struct Val const *const pushed = push(
            &queue,
            &(struct Val){
                .val = (int)rand(), /* NOLINT */
                .id = i,
            }
                 .elem,
            &allocator
        );
        check(pushed != NULL, true);
        check(validate(&queue), true);
    }
    struct Stack_allocator *const stack_meta = allocator.context;
    struct Val *const val_array = (struct Val *)stack_meta->blocks;
    for (int i = 0; i < WEAK_SRAND_HEAP_CAP; ++i) {
        (void)CCC_priority_queue_extract(&queue, &val_array[i].elem);
        check(validate(&queue), true);
    }
    check(CCC_priority_queue_is_empty(&queue), true);
    check_end();
}

check_static_begin(priority_queue_test_weak_srand_allocate) {
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    int const num_heap_nodes = 100;
    for (int i = 0; i < num_heap_nodes; ++i) {
        check(
            push(
                &queue,
                &(struct Val){
                    .id = i,
                    .val = (int)rand() /*NOLINT*/,
                }
                     .elem,
                &std_allocator
            ) != NULL,
            true
        );
        check(validate(&queue), true);
    }
    check_end({
        CCC_priority_queue_clear(&queue, &(CCC_Destructor){}, &std_allocator);
    })
}

static void
destroy_elem(CCC_Arguments const arguments) {
    struct Val const *const v = arguments.type;
    Flat_bitset *const is_destroyed = arguments.context;
    (void)flat_bitset_set(is_destroyed, (size_t)v->id, CCC_TRUE);
}

check_static_begin(priority_queue_test_clear_destructor) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[STANDARD_CAP]){}),
    };
    CCC_Priority_queue queue = CCC_priority_queue_for(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    int i = 0;
    Flat_bitset is_destroyed
        = flat_bitset_with_storage(STANDARD_CAP, (Bit[STANDARD_CAP]){});
    while (i < STANDARD_CAP) {
        struct Val const *const v = CCC_priority_queue_push(
            &queue, &(struct Val){.id = i, .val = i}.elem, &allocator
        );
        check(v != NULL, CCC_TRUE);
        ++i;
    }
    check(
        CCC_priority_queue_clear(
            &queue,
            &(CCC_Destructor){
                .destroy = destroy_elem,
                .context = &is_destroyed,
            },
            &allocator
        ),
        CCC_RESULT_OK
    );
    i = 0;
    while (!flat_bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = flat_bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(i, STANDARD_CAP);
    check_end();
}

int
main(void) {
    return check_run(
        priority_queue_test_insert_remove_key_value_four_dups(),
        priority_queue_test_insert_extract_shuffled(),
        priority_queue_test_insert_erase_shuffled(),
        priority_queue_test_pop_max(),
        priority_queue_test_pop_min(),
        priority_queue_test_delete_prime_shuffle_duplicates(),
        priority_queue_test_prime_shuffle(),
        priority_queue_test_weak_srand(),
        priority_queue_test_weak_srand_allocate(),
        priority_queue_test_clear_destructor(),
    );
}
