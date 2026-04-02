#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC
#define FLAT_BITSET_USING_NAMESPACE_CCC

#include "array_tree_map_utility.h"
#include "ccc/array_tree_map.h"
#include "ccc/flat_bitset.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/allocate.h"

check_static_begin(array_tree_map_test_insert_erase_shuffled) {
    CCC_Array_tree_map s = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(
        CCC_handle_status(
            array_tree_map_remove_key_value_wrap(NULL, &(struct Val){})
        ),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_handle_status(array_tree_map_remove_key_value_wrap(&s, NULL)),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    size_t const size = 50;
    int const prime = 53;
    check(insert_shuffled(&s, size, prime, &(CCC_Allocator){}), CHECK_PASS);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &s), size);
    for (size_t i = 0; i + 1 < size; ++i) {
        check(sorted_check[i] <= sorted_check[i + 1], true);
    }
    /* Now let's delete everything with no errors. */
    for (size_t i = 0; i < size; ++i) {
        CCC_Handle const *const h = array_tree_map_remove_key_value_wrap(
            &s, &(struct Val){.id = (int)i}
        );
        check(occupied(h), true);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(array_tree_map_test_prime_shuffle) {
    CCC_Array_tree_map s = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(
        CCC_handle_status(CCC_array_tree_map_remove_handle_wrap(NULL)),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    enum : size_t {
        SHUFFLE_CAP = 50,
        PRIME = 53,
        LESS = 10,
    };
    /* We want the tree to have a smattering of duplicates so
       reduce the shuffle range so it will repeat some values. */
    size_t shuffled_index = PRIME % (SHUFFLE_CAP - LESS);
    Flat_bitset repeats
        = flat_bitset_with_storage(SHUFFLE_CAP, (CCC_Bit[SHUFFLE_CAP]){});
    for (size_t i = 0; i < SHUFFLE_CAP; ++i) {
        if (occupied(array_tree_map_try_insert_wrap(
                &s,
                (&(struct Val){
                    .id = (int)shuffled_index,
                    .val = (int)shuffled_index,
                }),
                &(CCC_Allocator){}
            ))) {
            CCC_Tribool const was = flat_bitset_set(&repeats, i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        check(validate(&s), true);
        shuffled_index = (shuffled_index + PRIME) % (SHUFFLE_CAP - LESS);
    }
    check(array_tree_map_count(&s).count < SHUFFLE_CAP, true);
    for (size_t i = 0; i < SHUFFLE_CAP; ++i) {
        CCC_Handle const *const e = array_tree_map_remove_handle_wrap(
            array_tree_map_handle_wrap(&s, &i)
        );
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(e) || is_repeat, true);
        check(validate(&s), true);
    }
    check_end();
}

check_static_begin(array_tree_map_test_weak_srand) {
    CCC_Array_tree_map s = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    srand((unsigned)time(NULL)); /* NOLINT */
    enum : int {
        SRAND_CAP = 100,
    };
    int id_keys[SRAND_CAP];
    Flat_bitset repeats
        = flat_bitset_with_storage(SRAND_CAP, (CCC_Bit[SRAND_CAP]){});
    for (int i = 0; i < SRAND_CAP; ++i) {
        int const rand_i = (int)rand(); /* NOLINT */
        if (occupied(array_tree_map_try_insert_wrap(
                &s,
                (&(struct Val){
                    .id = rand_i,
                    .val = i,
                }),
                &(CCC_Allocator){}
            ))) {
            CCC_Tribool const was
                = flat_bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        (void)swap_handle(
            &s, &(struct Val){.id = rand_i, .val = i}, &(CCC_Allocator){}
        );
        id_keys[i] = rand_i;
        check(validate(&s), true);
    }
    for (int i = 0; i < SRAND_CAP; ++i) {
        CCC_Handle const h
            = CCC_remove_key_value(&s, &(struct Val){.id = id_keys[i]});
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, true);
        check(validate(&s), true);
    }
    check(is_empty(&s), true);
    check_end();
}

enum : int {
    CYCLES_TEST_CAP = 500,
};

check_static_begin(array_tree_map_test_insert_erase_cycles_no_allocate) {
    CCC_Array_tree_map s = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    srand((unsigned)time(NULL)); /* NOLINT */
    int id_keys[CYCLES_TEST_CAP];
    Flat_bitset repeats = flat_bitset_with_storage(
        CYCLES_TEST_CAP, (CCC_Bit[CYCLES_TEST_CAP]){}
    );
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        int const rand_i = (int)rand(); /* NOLINT */
        if (occupied(array_tree_map_insert_or_assign_wrap(
                &s,
                (&(struct Val){
                    .id = rand_i,
                    .val = i,
                }),
                &(CCC_Allocator){}
            ))) {
            CCC_Tribool const was
                = flat_bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        id_keys[i] = rand_i;
        check(validate(&s), CCC_TRUE);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Handle h
            = CCC_remove_key_value(&s, &(struct Val){.id = id_keys[i]});
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, CCC_TRUE);
        check(validate(&s), CCC_TRUE);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Handle h = insert_or_assign(
            &s, &(struct Val){.id = id_keys[i]}, &(CCC_Allocator){}
        );
        check(occupied(&h), false);
        check(validate(&s), CCC_TRUE);
    }
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        CCC_Handle h
            = CCC_remove_key_value(&s, &(struct Val){.id = id_keys[i]});
        CCC_Tribool const is_repeat = flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, CCC_TRUE);
        check(validate(&s), CCC_TRUE);
    }
    check(is_empty(&s), CCC_TRUE);
    check_end();
}

/** Make sure this test uses standard library allocator. Resizing is important
to test for handle maps. Stack allocator does not allow resizing. */
check_static_begin(array_tree_map_test_insert_erase_cycles_allocate) {
    CCC_Array_tree_map s = array_tree_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    /* NOLINTNEXTLINE (cert-msc51-cpp) */
    srand((unsigned)time(NULL));
    int id_keys[CYCLES_TEST_CAP];
    CCC_Flat_bitset repeats = CCC_flat_bitset_with_storage(
        CYCLES_TEST_CAP, (CCC_Bit[CYCLES_TEST_CAP]){}
    );
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        int const rand_i = (int)rand(); /* NOLINT */
        if (occupied(array_tree_map_insert_or_assign_wrap(
                &s,
                (&(struct Val){
                    .id = rand_i,
                    .val = i,
                }),
                &std_allocator
            ))) {
            CCC_Tribool const was
                = CCC_flat_bitset_set(&repeats, (size_t)i, CCC_TRUE);
            check(was != CCC_TRIBOOL_ERROR, CCC_TRUE);
        }
        id_keys[i] = rand_i;
        check(validate(&s), CCC_TRUE);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Handle h
            = CCC_remove_key_value(&s, &(struct Val){.id = id_keys[i]});
        CCC_Tribool const is_repeat = CCC_flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, CCC_TRUE);
        check(validate(&s), CCC_TRUE);
    }
    for (int i = 0; i < CYCLES_TEST_CAP / 2; ++i) {
        CCC_Handle h = insert_or_assign(
            &s, &(struct Val){.id = id_keys[i]}, &std_allocator
        );
        check(occupied(&h), false);
        check(validate(&s), CCC_TRUE);
    }
    for (int i = 0; i < CYCLES_TEST_CAP; ++i) {
        CCC_Handle h
            = CCC_remove_key_value(&s, &(struct Val){.id = id_keys[i]});
        CCC_Tribool const is_repeat = CCC_flat_bitset_test(&repeats, (size_t)i);
        check(is_repeat != CCC_TRIBOOL_ERROR, CCC_TRUE);
        check(occupied(&h) || is_repeat, CCC_TRUE);
        check(validate(&s), CCC_TRUE);
    }
    check(is_empty(&s), CCC_TRUE);
    check_end({
        array_tree_map_clear_and_free(&s, &(CCC_Destructor){}, &std_allocator);
    });
}

int
main(void) {
    return check_run(
        array_tree_map_test_insert_erase_shuffled(),
        array_tree_map_test_prime_shuffle(),
        array_tree_map_test_weak_srand(),
        array_tree_map_test_insert_erase_cycles_no_allocate(),
        array_tree_map_test_insert_erase_cycles_allocate()
    );
}
