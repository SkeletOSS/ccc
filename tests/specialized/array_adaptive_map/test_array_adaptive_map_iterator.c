#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FLAT_BITSET_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC
#define ARRAY_ADAPTIVE_MAP_USING_NAMESPACE_CCC
#define TYPES_USING_NAMESPACE_CCC

#include "array_adaptive_map_utility.h"
#include "ccc/flat_bitset.h"
#include "ccc/specialized/array_adaptive_map.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/stack_allocator.h"

check_static_begin(
    check_range,
    Array_adaptive_map const *const map,
    Handle_range const *const r,
    size_t const n,
    int const expect_range[]
) {
    size_t index = 0;
    CCC_Handle_index iterator = handle_range_begin(r);
    for (; iterator != handle_range_end(r) && index < n;
         iterator = next(map, iterator), ++index) {
        struct Val const *const v = array_adaptive_map_at(map, iterator);
        int const cur_id = v->id;
        check(expect_range[index], cur_id);
    }
    check(iterator, handle_range_end(r));
    if (iterator != end(map)) {
        struct Val const *const v = array_adaptive_map_at(map, iterator);
        check(v->id, expect_range[n - 1]);
    }
    check_fail_end({
        (void)fprintf(stderr, "%sCHECK: (int[%zu]){", CHECK_GREEN, n);
        for (size_t j = 0; j < n; ++j) {
            (void)fprintf(stderr, "%d, ", expect_range[j]);
        }
        (void)fprintf(stderr, "}\n%s", CHECK_NONE);
        (void)fprintf(
            stderr, "%sCHECK_ERROR:%s (int[%zu]){", CHECK_RED, CHECK_GREEN, n
        );
        iterator = handle_range_begin(r);
        for (size_t j = 0; j < n && iterator != handle_range_end(r);
             ++j, iterator = next(map, iterator)) {
            struct Val const *const v = array_adaptive_map_at(map, iterator);
            if (iterator == end(map) || !iterator) {
                return CHECK_STATUS;
            }
            if (expect_range[j] == v->id) {
                (void)fprintf(
                    stderr, "%s%d, %s", CHECK_GREEN, expect_range[j], CHECK_NONE
                );
            } else {
                (void)fprintf(stderr, "%s%d, %s", CHECK_RED, v->id, CHECK_NONE);
            }
        }
        for (; iterator != handle_range_end(r);
             iterator = next(map, iterator)) {
            struct Val const *const v = array_adaptive_map_at(map, iterator);
            (void)fprintf(stderr, "%s%d, %s", CHECK_RED, v->id, CHECK_NONE);
        }
        (void)fprintf(stderr, "%s}\n%s", CHECK_GREEN, CHECK_NONE);
    });
}

check_static_begin(
    check_range_reverse,
    Array_adaptive_map const *const map,
    Handle_range_reverse const *const r,
    size_t const n,
    int const expect_range_reverse[]
) {
    CCC_Handle_index iterator = handle_range_reverse_begin(r);
    size_t index = 0;
    for (; iterator != handle_range_reverse_end(r);
         iterator = reverse_next(map, iterator)) {
        struct Val const *const v = array_adaptive_map_at(map, iterator);
        int const cur_id = v->id;
        check(expect_range_reverse[index], cur_id);
        ++index;
    }
    check(iterator, handle_range_reverse_end(r));
    if (iterator != reverse_end(map)) {
        struct Val const *const v = array_adaptive_map_at(map, iterator);
        check(v->id, expect_range_reverse[n - 1]);
    }
    check_fail_end({
        (void)fprintf(stderr, "%sCHECK: (int[%zu]){", CHECK_GREEN, n);
        size_t j = 0;
        for (; j < n; ++j) {
            (void)fprintf(stderr, "%d, ", expect_range_reverse[j]);
        }
        (void)fprintf(stderr, "}\n%s", CHECK_NONE);
        (void)fprintf(
            stderr, "%sCHECK_ERROR:%s (int[%zu]){", CHECK_RED, CHECK_GREEN, n
        );
        iterator = handle_range_reverse_begin(r);
        for (j = 0; j < n && iterator != handle_range_reverse_end(r);
             ++j, iterator = reverse_next(map, iterator)) {
            struct Val const *const v = array_adaptive_map_at(map, iterator);
            if (iterator == reverse_end(map) || !iterator) {
                return CHECK_STATUS;
            }
            if (expect_range_reverse[j] == v->id) {
                (void)fprintf(
                    stderr,
                    "%s%d, %s",
                    CHECK_GREEN,
                    expect_range_reverse[j],
                    CHECK_NONE
                );
            } else {
                (void)fprintf(stderr, "%s%d, %s", CHECK_RED, v->id, CHECK_NONE);
            }
        }
        for (; iterator != handle_range_reverse_end(r);
             iterator = reverse_next(map, iterator)) {
            struct Val const *const v = array_adaptive_map_at(map, iterator);
            (void)fprintf(stderr, "%s%d, %s", CHECK_RED, v->id, CHECK_NONE);
        }
        (void)fprintf(stderr, "%s}\n%s", CHECK_GREEN, CHECK_NONE);
    });
}

check_static_begin(iterator_check, Array_adaptive_map *s) {
    size_t const size = count(s).count;
    size_t iterator_count = 0;
    for (CCC_Handle_index e = begin(s); e != end(s); e = next(s, e)) {
        ++iterator_count;
        check(iterator_count <= size, true);
    }
    check(iterator_count, size);
    iterator_count = 0;
    for (CCC_Handle_index e = reverse_begin(s); e != end(s);
         e = reverse_next(s, e)) {
        ++iterator_count;
        check(iterator_count <= size, true);
    }
    check(iterator_count, size);
    check_end();
}

check_static_begin(array_adaptive_map_test_forward_iterator) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(array_adaptive_map_begin(NULL), 0);
    check(array_adaptive_map_reverse_begin(NULL), 0);
    check(array_adaptive_map_end(NULL), 0);
    check(array_adaptive_map_reverse_end(NULL), 0);
    check(array_adaptive_map_next(NULL, 0), 0);
    check(array_adaptive_map_reverse_next(NULL, 0), 0);
    /* We should have the expected behavior iteration over empty tree. */
    int j = 0;
    for (CCC_Handle_index e = begin(&s); e != end(&s); e = next(&s, e), ++j) {}
    check(j, 0);
    int const num_nodes = 33;
    int const prime = 37;
    size_t shuffled_index = (size_t)prime % (size_t)num_nodes;
    for (int i = 0; i < num_nodes; ++i) {
        (void)insert_or_assign(
            &s,
            &(struct Val){.id = (int)shuffled_index, .val = i},
            &(CCC_Allocator){}
        );
        check(validate(&s), true);
        shuffled_index = (shuffled_index + (size_t)prime) % (size_t)num_nodes;
    }
    int keys_inorder[33];
    check(inorder_fill(keys_inorder, (size_t)num_nodes, &s), count(&s).count);
    j = 0;
    for (CCC_Handle_index e = begin(&s); e != end(&s) && j < num_nodes;
         e = next(&s, e), ++j) {
        struct Val const *const v = array_adaptive_map_at(&s, e);
        check(v->id, keys_inorder[j]);
    }
    check_end();
}

check_static_begin(array_adaptive_map_test_iterate_removal) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    size_t const num_nodes = 1000;
    for (size_t i = 0; i < num_nodes; ++i) {
        (void)insert_or_assign(
            &s,
            &(struct Val){
                .id = (int)((size_t)rand() % (num_nodes + 1)), /* NOLINT */
                .val = (int)i,
            },
            &(CCC_Allocator){}
        );
        check(validate(&s), true);
    }
    check(iterator_check(&s), CHECK_PASS);
    int const limit = 400;
    size_t cur_node = 0;
    for (CCC_Handle_index i = begin(&s), next = 0;
         i != end(&s) && cur_node < num_nodes;
         i = next, ++cur_node) {
        next = next(&s, i);
        struct Val const *const v = array_adaptive_map_at(&s, i);
        if (v->id > limit) {
            (void)remove_key_value(&s, &(struct Val){.id = v->id});
            check(validate(&s), true);
        }
    }
    check_end();
}

check_static_begin(array_adaptive_map_test_iterate_remove_key_value_reinsert) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    size_t const num_nodes = 1000;
    for (size_t i = 0; i < num_nodes; ++i) {
        /* NOLINTNEXTLINE */
        (void)try_insert(
            &s,
            &(struct Val){
                .id = (int)((size_t)rand() % (num_nodes + 1)),
                .val = (int)i,
            },
            &(CCC_Allocator){}
        );
        check(validate(&s), true);
    }
    check(iterator_check(&s), CHECK_PASS);
    size_t const old_size = count(&s).count;
    int const limit = 400;
    int new_unique_array_id = 1001;
    for (CCC_Handle_index i = begin(&s), next = 0; i != end(&s); i = next) {
        next = next(&s, i);
        struct Val const *const v = array_adaptive_map_at(&s, i);
        if (v->id < limit) {
            struct Val new_val = {.id = v->id};
            (void)remove_key_value(&s, &new_val);
            new_val.id = new_unique_array_id;
            CCC_Handle e = insert_or_assign(&s, &new_val, &(CCC_Allocator){});
            check(unwrap(&e) != 0, true);
            check(validate(&s), true);
            ++new_unique_array_id;
        }
    }
    check(count(&s).count, old_size);
    check_end();
}

check_static_begin(array_adaptive_map_test_valid_range) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(
        CCC_range_begin(
            array_adaptive_map_equal_range_wrap(NULL, &(int){}, &(int){})
        ),
        0
    );
    check(
        CCC_range_begin(
            array_adaptive_map_equal_range_wrap(&s, NULL, &(int){})
        ),
        0
    );
    check(
        CCC_range_begin(
            array_adaptive_map_equal_range_wrap(&s, &(int){}, NULL)
        ),
        0
    );
    check(
        CCC_range_reverse_begin(array_adaptive_map_equal_range_reverse_wrap(
            NULL, &(int){}, &(int){}
        )),
        0
    );
    check(
        CCC_range_reverse_begin(
            array_adaptive_map_equal_range_reverse_wrap(&s, NULL, &(int){})
        ),
        0
    );
    check(
        CCC_range_reverse_begin(
            array_adaptive_map_equal_range_reverse_wrap(&s, &(int){}, NULL)
        ),
        0
    );
    check(
        CCC_range_begin(
            array_adaptive_map_equal_range_wrap(&s, &(int){}, &(int){})
        ),
        0
    );
    check(
        CCC_range_reverse_begin(
            array_adaptive_map_equal_range_reverse_wrap(&s, &(int){}, &(int){})
        ),
        0
    );
    int const num_nodes = 25;
    /* 0, 5, 10, 15, 20, 25, 30, 35,... 120 */
    for (int i = 0, id = 0; i < num_nodes; ++i, id += 5) {
        (void)insert_or_assign(
            &s, &(struct Val){.id = id, .val = i}, &(CCC_Allocator){}
        );
        check(validate(&s), true);
    }
    /* This should be the following range [6,44). 6 should raise to
       next value not less than 6, 10 and 44 should be the first
       value greater than 44, 45. */
    check(
        check_range(
            &s,
            array_adaptive_map_equal_range_wrap(&s, &(int){6}, &(int){44}),
            8,
            (int[8]){10, 15, 20, 25, 30, 35, 40, 45}
        ),
        CHECK_PASS
    );
    /* This should be the following range [119,84). 119 should be
       dropped to first value not greater than 119 and last should
       be dropped to first value less than 84. */
    check(
        check_range_reverse(
            &s,
            array_adaptive_map_equal_range_reverse_wrap(
                &s, &(int){119}, &(int){84}
            ),
            8,
            (int[8]){115, 110, 105, 100, 95, 90, 85, 80}
        ),
        CHECK_PASS
    );
    check_end();
}

check_static_begin(array_adaptive_map_test_valid_range_equals) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int const num_nodes = 25;
    /* 0, 5, 10, 15, 20, 25, 30, 35,... 120 */
    for (int i = 0, id = 0; i < num_nodes; ++i, id += 5) {
        (void)insert_or_assign(
            &s, &(struct Val){.id = id, .val = i}, &(CCC_Allocator){}
        );
        check(validate(&s), true);
    }
    /* This should be the following range [5,50). 5 should stay at the start,
       and 45 is equal to our end key so is bumped to next greater, 50. */
    check(
        check_range(
            &s,
            array_adaptive_map_equal_range_wrap(&s, &(int){10}, &(int){40}),
            8,
            (int[8]){10, 15, 20, 25, 30, 35, 40, 45}
        ),
        CHECK_PASS
    );
    /* This should be the following range [115,84). 115 should be
       is a valid start to the range and 85 is eqal to end key so must
       be dropped to first value less than 85, 80. */
    check(
        check_range_reverse(
            &s,
            array_adaptive_map_equal_range_reverse_wrap(
                &s, &(int){115}, &(int){85}
            ),
            8,
            (int[8]){115, 110, 105, 100, 95, 90, 85, 80}
        ),
        CHECK_PASS
    );
    check_end();
}

check_static_begin(array_adaptive_map_test_invalid_range) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int const num_nodes = 25;
    /* 0, 5, 10, 15, 20, 25, 30, 35,... 120 */
    for (int i = 0, id = 0; i < num_nodes; ++i, id += 5) {
        (void)insert_or_assign(
            &s, &(struct Val){.id = id, .val = i}, &(CCC_Allocator){}
        );
        check(validate(&s), true);
    }
    /* This should be the following range [95,999). 95 should raise to
       next value not less than 95, 95 and 999 should be the first
       value greater than 999, none or the end. */
    check(
        check_range(
            &s,
            array_adaptive_map_equal_range_wrap(&s, &(int){95}, &(int){999}),
            6,
            (int[6]){95, 100, 105, 110, 115, 120}
        ),
        CHECK_PASS
    );
    /* This should be the following range [36,-999). 36 should be
       dropped to first value not greater than 36 and last should
       be dropped to first value less than -999 which is end. */
    check(
        check_range_reverse(
            &s,
            array_adaptive_map_equal_range_reverse_wrap(
                &s, &(int){36}, &(int){-999}
            ),
            8,
            (int[8]){35, 30, 25, 20, 15, 10, 5, 0}
        ),
        CHECK_PASS
    );
    check_end();
}

check_static_begin(array_adaptive_map_test_empty_range) {
    CCC_Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int const num_nodes = 25;
    int const step = 5;
    /* 0, 5, 10, 15, 20, 25, 30, 35,... 120 */
    for (int i = 0, id = 0; i < num_nodes; ++i, id += step) {
        (void)insert_or_assign(
            &s, &(struct Val){.id = id, .val = i}, &(CCC_Allocator){}
        );
        check(validate(&s), true);
    }
    /* Nonexistant range returns end [begin, end) in both positions.
       which may not be the end element but a value in the tree. However,
       Normal iteration patterns would consider this empty. */
    CCC_Handle_range const forward_range
        = equal_range(&s, &(int){-50}, &(int){-25});
    check(
        ((struct Val *)
             array_adaptive_map_at(&s, handle_range_begin(&forward_range)))
            ->id,
        0
    );
    check(
        ((struct Val *)
             array_adaptive_map_at(&s, handle_range_end(&forward_range)))
            ->id,
        0
    );
    check(handle_range_begin(&forward_range), handle_range_end(&forward_range));
    CCC_Handle_range_reverse const rev_range
        = equal_range_reverse(&s, &(int){150}, &(int){999});
    check(
        handle_range_reverse_begin(&rev_range),
        handle_range_reverse_end(&rev_range)
    );
    check(
        ((struct Val *)
             array_adaptive_map_at(&s, handle_range_reverse_begin(&rev_range)))
            ->id,
        (num_nodes * step) - step
    );
    check(
        ((struct Val *)
             array_adaptive_map_at(&s, handle_range_reverse_end(&rev_range)))
            ->id,
        (num_nodes * step) - step
    );
    check_end();
}

static void
destroy_element(CCC_Arguments const arguments) {
    struct Val const *const i = arguments.type;
    Flat_bitset *const is_destroyed_buffer = arguments.context;
    (void)flat_bitset_set(is_destroyed_buffer, (size_t)i->id, CCC_TRUE);
}

check_static_begin(array_adaptive_map_test_clear_with_destructor) {
    enum : size_t {
        MIN_CAP = 16
    };
    CCC_Array_adaptive_map map = array_adaptive_map_with_storage(
        id, (CCC_Key_comparator){.compare = id_order}, (struct Val[MIN_CAP]){}
    );
    Flat_bitset is_destroyed = flat_bitset_with_storage(0, (Bit[MIN_CAP]){});
    int i = 0;
    for (;;) {
        CCC_Handle const *const e = array_adaptive_map_try_insert_with(
            &map, i, &(CCC_Allocator){}, (struct Val){.val = i}
        );
        check(occupied(e), CCC_FALSE);
        CCC_Handle_index const h = unwrap(e);
        if (!h) {
            break;
        }
        struct Val const *const v = array_adaptive_map_at(&map, h);
        CCC_Result const bit_push = flat_bitset_push_back(
            &is_destroyed, CCC_FALSE, &(CCC_Allocator){}
        );
        check(bit_push, CCC_RESULT_OK);
        check(v->id, i);
        check(v->val, i);
        ++i;
    }
    size_t const full_count = count(&map).count;
    array_adaptive_map_clear(
        &map,
        &(CCC_Destructor){
            .destroy = destroy_element,
            .context = &is_destroyed,
        }
    );
    i = 0;
    while (!flat_bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = flat_bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(i, full_count);
    check_end();
}

check_static_begin(array_adaptive_map_test_clear_and_free_with_destructor) {
    enum : size_t {
        MIN_CAP = 16
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((
            typeof(array_adaptive_map_storage_for((struct Val[MIN_CAP]){}))[1]
        ){}),
    };
    CCC_Array_adaptive_map map = array_adaptive_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        MIN_CAP
    );
    Flat_bitset is_destroyed = flat_bitset_with_storage(0, (Bit[MIN_CAP]){});
    int i = 0;
    for (;;) {
        CCC_Handle const *const e = array_adaptive_map_try_insert_with(
            &map, i, &(CCC_Allocator){}, (struct Val){.val = i}
        );
        check(occupied(e), CCC_FALSE);
        CCC_Handle_index index = unwrap(e);
        if (!index) {
            break;
        }
        struct Val const *const v = array_adaptive_map_at(&map, index);
        CCC_Result const bit_push = flat_bitset_push_back(
            &is_destroyed, CCC_FALSE, &(CCC_Allocator){}
        );
        check(bit_push, CCC_RESULT_OK);
        check(v->id, i);
        check(v->val, i);
        ++i;
    }
    size_t const full_count = count(&map).count;
    array_adaptive_map_clear_and_free(
        &map,
        &(CCC_Destructor){
            .destroy = destroy_element,
            .context = &is_destroyed,
        },
        &allocator
    );
    i = 0;
    while (!flat_bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = flat_bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(i, full_count);
    check_end();
}

int
main(void) {
    return check_run(
        array_adaptive_map_test_forward_iterator(),
        array_adaptive_map_test_iterate_removal(),
        array_adaptive_map_test_valid_range(),
        array_adaptive_map_test_valid_range_equals(),
        array_adaptive_map_test_invalid_range(),
        array_adaptive_map_test_empty_range(),
        array_adaptive_map_test_iterate_remove_key_value_reinsert(),
        array_adaptive_map_test_clear_with_destructor(),
        array_adaptive_map_test_clear_and_free_with_destructor(),
    );
}
