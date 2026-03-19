#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC
#define ADAPTIVE_MAP_USING_NAMESPACE_CCC

#include "adaptive_map_utility.h"
#include "ccc/adaptive_map.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/stack_allocator.h"

check_static_begin(adaptive_map_test_prime_shuffle) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[50]){}),
    };
    Adaptive_map s = adaptive_map_default(
        struct Val, elem, key, &(CCC_Key_comparator){.compare = id_order}
    );
    size_t const size = 50;
    size_t const prime = 53;
    size_t const less = 10;
    /* We want the tree to have a smattering of duplicates so
       reduce the shuffle range so it will repeat some values. */
    size_t shuffled_index = prime % (size - less);
    bool repeats[50] = {};
    for (size_t i = 0; i < size; ++i) {
        if (occupied(adaptive_map_insert_or_assign_wrap(
                &s,
                &(struct Val){
                    .val = (int)shuffled_index,
                    .key = (int)shuffled_index,
                }
                     .elem,
                &allocator
            ))) {
            repeats[i] = true;
        }
        check(validate(&s), true);
        shuffled_index = (shuffled_index + prime) % (size - less);
    }
    check(adaptive_map_count(&s).count < size, true);
    struct Val *const vals
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t i = 0; i < size; ++i) {
        check(
            occupied(adaptive_map_remove_entry_wrap(
                adaptive_map_entry_wrap(&s, &vals[i].key), &allocator
            )) || repeats[i],
            true
        );
        check(validate(&s), true);
    }
    check_end();
}

check_static_begin(adaptive_map_test_insert_erase_shuffled) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[50]){}),
    };
    Adaptive_map s = adaptive_map_default(
        struct Val, elem, key, &(CCC_Key_comparator){.compare = id_order}
    );
    size_t const size = 50;
    int const prime = 53;
    check(insert_shuffled(&s, size, prime, &allocator), CHECK_PASS);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &s), CHECK_PASS);
    /* Now let's delete everything with no errors. */
    struct Val *const vals
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t i = 0; i < size; ++i) {
        struct Val *v = unwrap(
            adaptive_map_remove_key_value_wrap(&s, &vals[i].elem, &allocator)
        );
        check(v != NULL, true);
        check(v->key, vals[i].key);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(adaptive_map_test_weak_srand) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[100]){}),
    };
    Adaptive_map s = adaptive_map_default(
        struct Val, elem, key, &(CCC_Key_comparator){.compare = id_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand(time(NULL));
    int const num_nodes = 100;
    bool repeats[100] = {};
    for (int i = 0; i < num_nodes; ++i) {
        if (occupied(adaptive_map_insert_or_assign_wrap(
                &s,
                (&(struct Val){
                    .key = rand(), /* NOLINT */
                    .val = i,
                }
                      .elem),
                &allocator
            ))) {
            repeats[i] = true;
        }
        check(validate(&s), true);
    }
    struct Val *const vals
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (int i = 0; i < num_nodes; ++i) {
        CCC_Entry entry = CCC_remove_key_value(&s, &vals[i].elem, &allocator);
        check(occupied(&entry) || repeats[i], true);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(adaptive_map_test_insert_erase_cycles) {
    /* Over allocate because we do more insertions near the end. */
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[200]){}),
    };
    Adaptive_map s = adaptive_map_default(
        struct Val, elem, key, &(CCC_Key_comparator){.compare = id_order}
    );
    srand(time(NULL)); /* NOLINT */
    int const num_nodes = 100;
    int keys[100] = {};
    bool repeats[100] = {};
    for (int i = 0; i < num_nodes; ++i) {
        keys[i] = rand(); /* NOLINT */
        if (occupied(adaptive_map_insert_or_assign_wrap(
                &s,
                &(struct Val){
                    .key = keys[i],
                    .val = i,
                }
                     .elem,
                &allocator
            ))) {
            repeats[i] = true;
        }
        check(validate(&s), true);
    }
    for (int i = 0; i < num_nodes / 2; ++i) {
        CCC_Entry h = adaptive_map_remove_entry(
            adaptive_map_entry_wrap(&s, &keys[i]), &allocator
        );
        check(occupied(&h) || repeats[i], true);
        check(validate(&s), true);
    }
    for (int i = 0; i < num_nodes / 2; ++i) {
        CCC_Entry const *const entry = adaptive_map_insert_or_assign_with(
            &s,
            keys[i],
            &allocator,
            (struct Val){
                .val = i,
            }
        );
        check(occupied(entry), false);
        check(validate(&s), true);
    }
    for (int i = 0; i < num_nodes; ++i) {
        CCC_Entry const entry = adaptive_map_remove_entry(
            adaptive_map_entry_wrap(&s, &keys[i]), &allocator
        );
        check(occupied(&entry) || repeats[i], true);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

int
main(void) {
    return check_run(
        adaptive_map_test_insert_erase_shuffled(),
        adaptive_map_test_prime_shuffle(),
        adaptive_map_test_weak_srand(),
        adaptive_map_test_insert_erase_cycles()
    );
}
