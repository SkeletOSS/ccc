#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC
#define ADAPTIVE_MAP_USING_NAMESPACE_CCC
#define FLAT_BITSET_USING_NAMESPACE_CCC

#include "adaptive_map_utility.h"
#include "ccc/flat_bitset.h"
#include "ccc/specialized/adaptive_map.h"
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
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    size_t const size = 50;
    size_t const prime = 53;
    size_t const less = 10;
    /* We want the tree to have a smattering of duplicates so
       reduce the shuffle range so it will repeat some values. */
    size_t shuffled_index = prime % (size - less);

    Flat_bitset repeats = flat_bitset_with_storage(50, (CCC_Bit[50]){});
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
            CCC_Tribool const was = flat_bitset_set(&repeats, i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
        shuffled_index = (shuffled_index + prime) % (size - less);
    }
    check(adaptive_map_count(&s).count < size, true);
    struct Val *const vals
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (size_t i = 0; i < size; ++i) {
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        CCC_Tribool const is_occupied = occupied(adaptive_map_remove_entry_wrap(
            adaptive_map_entry_wrap(&s, &vals[i].key), &allocator
        ));
        check(is_occupied || is_repeat, true);
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
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(
        CCC_entry_status(adaptive_map_remove_key_value_wrap(
            NULL, &(struct Val){}.elem, &allocator
        )),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            adaptive_map_remove_key_value_wrap(&s, NULL, &allocator)
        ),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            adaptive_map_remove_key_value_wrap(&s, &(struct Val){}.elem, NULL)
        ),
        CCC_ENTRY_ARGUMENT_ERROR
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

check_static_begin(adaptive_map_test_insert_erase_shuffled_no_allocator) {
    enum : int {
        CAP = 32,
    };
    struct Val vals[CAP] = {};
    Adaptive_map s = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(
        CCC_entry_status(
            adaptive_map_remove_entry_wrap(NULL, &(CCC_Allocator){})
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            adaptive_map_remove_entry_wrap(&(CCC_Adaptive_map_entry){}, NULL)
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    for (int i = 0; i < CAP; ++i) {
        vals[i].key = i;
        CCC_Entry const e = adaptive_map_insert_or_assign(
            &s, &vals[i].elem, &(CCC_Allocator){}
        );
        check(CCC_entry_insert_error(&e), CCC_FALSE);
    }
    for (int i = 0; i < CAP; ++i) {
        struct Val *v = NULL;
        if (i % 2) {
            v = unwrap(adaptive_map_remove_key_value_wrap(
                &s, &vals[i].elem, &(CCC_Allocator){}
            ));
        } else {
            v = unwrap(adaptive_map_remove_entry_wrap(
                adaptive_map_entry_wrap(&s, &i), &(CCC_Allocator){}
            ));
        }
        check(v != NULL, true);
        check(v->key, vals[i].key);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(adaptive_map_test_weak_srand) {
    enum : int {
        SRAND_CAP = 100,
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[SRAND_CAP]){}),
    };
    Adaptive_map s = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    Flat_bitset repeats
        = flat_bitset_with_storage(SRAND_CAP, (CCC_Bit[SRAND_CAP]){});
    for (int i = 0; i < SRAND_CAP; ++i) {
        if (occupied(adaptive_map_insert_or_assign_wrap(
                &s,
                (&(struct Val){
                    .key = (int)rand(), /* NOLINT */
                    .val = i,
                }
                      .elem),
                &allocator
            ))) {
            CCC_Tribool const was
                = flat_bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
    }
    struct Val *const vals
        = ((struct Stack_allocator *)allocator.context)->blocks;
    for (int i = 0; i < SRAND_CAP; ++i) {
        CCC_Entry entry = CCC_remove_key_value(&s, &vals[i].elem, &allocator);
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&entry) || is_repeat, true);
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
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    enum : int {
        CYCLES_TEST_CAP = 100,
    };
    int keys[CYCLES_TEST_CAP] = {};
    Flat_bitset repeats = flat_bitset_with_storage(
        CYCLES_TEST_CAP, (CCC_Bit[CYCLES_TEST_CAP]){}
    );
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        keys[i] = (int)rand(); /* NOLINT */
        if (occupied(adaptive_map_insert_or_assign_wrap(
                &s,
                &(struct Val){
                    .key = keys[i],
                    .val = i,
                }
                     .elem,
                &allocator
            ))) {
            CCC_Tribool const was
                = flat_bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Entry h = adaptive_map_remove_entry(
            adaptive_map_entry_wrap(&s, &keys[i]), &allocator
        );
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, true);
        check(validate(&s), true);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
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
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        CCC_Entry const entry = adaptive_map_remove_entry(
            adaptive_map_entry_wrap(&s, &keys[i]), &allocator
        );
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&entry) || is_repeat, true);
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
        adaptive_map_test_insert_erase_shuffled_no_allocator(),
        adaptive_map_test_insert_erase_cycles(),
    );
}
