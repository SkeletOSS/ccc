#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BITSET_USING_NAMESPACE_CCC
#define TREE_MAP_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "ccc/bitset.h"
#include "ccc/traits.h"
#include "ccc/tree_map.h"
#include "ccc/types.h"
#include "checkers.h"
#include "tree_map_utility.h"
#include "utility/stack_allocator.h"

check_static_begin(tree_map_test_insert_erase_shuffled) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[50]){}),
    };
    CCC_Tree_map s = tree_map_for(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(
        CCC_entry_status(tree_map_remove_key_value_wrap(
            NULL, &(struct Val){}.elem, &allocator
        )),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(tree_map_remove_key_value_wrap(&s, NULL, &allocator)),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            tree_map_remove_key_value_wrap(&s, &(struct Val){}.elem, NULL)
        ),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    size_t const size = 50;
    int const prime = 53;
    check(insert_shuffled(&s, size, prime, &allocator), CHECK_PASS);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &s), CHECK_PASS);
    struct Val *const vals
        = ((struct Stack_allocator *)allocator.context)->blocks;
    /* Now let's delete everything with no errors. */
    for (size_t i = 0; i < size; ++i) {
        struct Val *v = unwrap(
            tree_map_remove_key_value_wrap(&s, &vals[i].elem, &allocator)
        );
        check(v != NULL, true);
        check(v->key, vals[i].key);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(tree_map_test_insert_erase_shuffled_no_allocator) {
    enum : int {
        CAP = 32,
    };
    struct Val vals[CAP] = {};
    Tree_map s = tree_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(
        CCC_entry_status(tree_map_remove_entry_wrap(NULL, &(CCC_Allocator){})),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            tree_map_remove_entry_wrap(&(CCC_Tree_map_entry){}, NULL)
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    for (int i = 0; i < CAP; ++i) {
        vals[i].key = i;
        CCC_Entry const e
            = tree_map_insert_or_assign(&s, &vals[i].elem, &(CCC_Allocator){});
        check(CCC_entry_insert_error(&e), CCC_FALSE);
    }
    for (int i = 0; i < CAP; ++i) {
        struct Val *v = NULL;
        if (i % 2) {
            v = unwrap(tree_map_remove_key_value_wrap(
                &s, &vals[i].elem, &(CCC_Allocator){}
            ));
        } else {
            v = unwrap(tree_map_remove_entry_wrap(
                tree_map_entry_wrap(&s, &i), &(CCC_Allocator){}
            ));
        }
        check(v != NULL, true);
        check(v->key, vals[i].key);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(tree_map_test_prime_shuffle) {
    CCC_Tree_map s = tree_map_for(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    enum : size_t {
        SHUFFLE_CAP = 50,
        PRIME = 53,
        LESS = 10,
    };
    /* We want the tree to have a smattering of duplicates so
       reduce the shuffle range so it will repeat some values. */
    size_t shuffled_index = PRIME % (SHUFFLE_CAP - LESS);
    struct Val vals[SHUFFLE_CAP];
    Bitset repeats = bitset_with_storage(SHUFFLE_CAP, (CCC_Bit[SHUFFLE_CAP]){});
    for (size_t i = 0; i < SHUFFLE_CAP; ++i) {
        vals[i].val = (int)shuffled_index;
        vals[i].key = (int)shuffled_index;
        CCC_Entry e = swap_entry(
            &s, &vals[i].elem, &(struct Val){}.elem, &(CCC_Allocator){}
        );
        if (unwrap(&e)) {
            CCC_Tribool const was = bitset_set(&repeats, i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
        shuffled_index = (shuffled_index + PRIME) % (SHUFFLE_CAP - LESS);
    }
    /* One test can use our printer function as test output */
    check(tree_map_count(&s).count < SHUFFLE_CAP, true);
    for (size_t i = 0; i < SHUFFLE_CAP; ++i) {
        CCC_Entry const *const e = tree_map_remove_entry_wrap(
            tree_map_entry_wrap(&s, &vals[i].key), &(CCC_Allocator){}
        );
        CCC_Tribool const is_repeat = bitset_test(&repeats, i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(e) || is_repeat, true);
        check(validate(&s), true);
    }
    check_end();
}

check_static_begin(tree_map_test_weak_srand) {
    CCC_Tree_map s = tree_map_for(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    /* Seed the test with any integer for reproducible random test sequence
       currently this will change every test. NOLINTNEXTLINE */
    srand((unsigned)time(NULL));
    enum : int {
        SRAND_CAP = 100,
    };
    struct Val vals[SRAND_CAP];
    Bitset repeats = bitset_with_storage(SRAND_CAP, (CCC_Bit[SRAND_CAP]){});
    for (int i = 0; i < SRAND_CAP; ++i) {
        vals[i].key = (int)rand(); // NOLINT
        vals[i].val = i;
        if (occupied(tree_map_swap_entry_wrap(
                &s, &vals[i].elem, &(struct Val){}.elem, &(CCC_Allocator){}
            ))) {
            CCC_Tribool const was = bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
    }
    for (int i = 0; i < SRAND_CAP; ++i) {
        CCC_Entry entry
            = CCC_remove_key_value(&s, &vals[i].elem, &(CCC_Allocator){});
        CCC_Tribool const is_repeat = bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&entry) || is_repeat, true);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(tree_map_test_insert_erase_cycles) {
    /* Over allocate because we do more insertions near the end. */
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[200]){}),
    };
    CCC_Tree_map s = CCC_tree_map_for(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    srand((unsigned)time(NULL)); /* NOLINT */
    enum : int {
        CYCLES_TEST_CAP = 100,
    };
    int keys[CYCLES_TEST_CAP] = {};
    CCC_Bitset repeats = CCC_bitset_with_storage(
        CYCLES_TEST_CAP, (CCC_Bit[CYCLES_TEST_CAP]){}
    );
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        keys[i] = (int)rand(); /* NOLINT */
        if (occupied(tree_map_insert_or_assign_wrap(
                &s,
                (&(struct Val){
                    .key = keys[i],
                    .val = i,
                }
                      .elem),
                &allocator
            ))) {
            CCC_Tribool const was
                = CCC_bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Entry h
            = remove_entry(tree_map_entry_wrap(&s, &keys[i]), &allocator);
        CCC_Tribool const is_repeat = CCC_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, CCC_TRUE);
        check(validate(&s), true);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Entry const *const entry = CCC_tree_map_insert_or_assign_with(
            &s, keys[i], &allocator, (struct Val){.val = i}
        );
        check(occupied(entry), false);
        check(validate(&s), true);
    }
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        CCC_Entry const entry
            = remove_entry(tree_map_entry_wrap(&s, &keys[i]), &allocator);
        CCC_Tribool const is_repeat = CCC_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&entry) || is_repeat, CCC_TRUE);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

int
main(void) {
    return check_run(
        tree_map_test_insert_erase_shuffled(),
        tree_map_test_insert_erase_shuffled_no_allocator(),
        tree_map_test_prime_shuffle(),
        tree_map_test_weak_srand(),
        tree_map_test_insert_erase_cycles()
    );
}
