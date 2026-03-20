#include <stdbool.h>
#include <stddef.h>

#define TRAITS_USING_NAMESPACE_CCC
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC

#include "array_tree_map.h"
#include "array_tree_map_utility.h"
#include "checkers.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

static Array_tree_map static_map = array_tree_map_with_storage(
    id,
    (CCC_Key_comparator){.compare = id_order},
    (struct Val[SMALL_FIXED_CAP]){}
);

check_static_begin(array_tree_map_test_static) {
    check(array_tree_map_capacity(&static_map).count, SMALL_FIXED_CAP);
    check(array_tree_map_count(&static_map).count, 0);
    check(validate(&static_map), CCC_TRUE);
    check(is_empty(&static_map), CCC_TRUE);
    CCC_Handle const inserted = array_tree_map_insert_or_assign(
        &static_map, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(insert_error(&inserted), CCC_FALSE);
    struct Val const *const v
        = array_tree_map_at(&static_map, unwrap(&inserted));
    check(v->val, 1);
    check(v->val, v->id);
    check_end();
}

check_static_begin(array_tree_map_test_empty) {
    Array_tree_map s = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(array_tree_map_test_with_literal) {
    Array_tree_map s = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&s), true);
    check(capacity(&s).count, SMALL_FIXED_CAP);
    check_end();
}

check_static_begin(array_tree_map_test_copy_no_allocate) {
    Array_tree_map source = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    Array_tree_map destination = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    (void)swap_handle(&source, &(struct Val){.id = 0}, &(CCC_Allocator){});
    (void)swap_handle(
        &source, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    (void)swap_handle(
        &source, &(struct Val){.id = 2, .val = 2}, &(CCC_Allocator){}
    );
    check(count(&source).count, 3);
    check(is_empty(&destination), true);
    CCC_Result res
        = array_tree_map_copy(&destination, &source, &(CCC_Allocator){});
    check(res, CCC_RESULT_OK);
    check(count(&destination).count, count(&source).count);
    for (int i = 0; i < 3; ++i) {
        struct Val source_v = {.id = i};
        struct Val destination_v = {.id = i};
        CCC_Handle source_e = CCC_remove_key_value(&source, &source_v);
        CCC_Handle destination_e
            = CCC_remove_key_value(&destination, &destination_v);
        check(occupied(&source_e), occupied(&destination_e));
        check(source_v.id, destination_v.id);
        check(source_v.val, destination_v.val);
    }
    check(is_empty(&source), is_empty(&destination));
    check(is_empty(&destination), true);
    check_end();
}

check_static_begin(array_tree_map_test_copy_no_allocate_fail) {
    Array_tree_map source = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    Array_tree_map destination = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    (void)swap_handle(&source, &(struct Val){.id = 0}, &(CCC_Allocator){});
    (void)swap_handle(
        &source, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    (void)swap_handle(
        &source, &(struct Val){.id = 2, .val = 2}, &(CCC_Allocator){}
    );
    check(count(&source).count, 3);
    check(is_empty(&destination), true);
    CCC_Result res = array_tree_map_copy(&destination, &source, NULL);
    check(res != CCC_RESULT_OK, true);
    check_end();
}

check_static_begin(array_tree_map_test_copy_allocate) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[2]){}),
    };
    Array_tree_map source = array_tree_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1
    );
    Array_tree_map destination = array_tree_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    (void)swap_handle(&source, &(struct Val){.id = 0}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 1, .val = 1}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 2, .val = 2}, &allocator);
    check(count(&source).count, 3);
    check(is_empty(&destination), true);
    CCC_Result res = array_tree_map_copy(&destination, &source, &allocator);
    check(res, CCC_RESULT_OK);
    check(count(&destination).count, count(&source).count);
    for (int i = 0; i < 3; ++i) {
        struct Val source_v = {.id = i};
        struct Val destination_v = {.id = i};
        CCC_Handle source_e = CCC_remove_key_value(&source, &source_v);
        CCC_Handle destination_e
            = CCC_remove_key_value(&destination, &destination_v);
        check(occupied(&source_e), occupied(&destination_e));
        check(source_v.id, destination_v.id);
        check(source_v.val, destination_v.val);
    }
    check(is_empty(&source), is_empty(&destination));
    check(is_empty(&destination), true);
    check_end({
        (void)array_tree_map_clear_and_free(
            &source, &(CCC_Destructor){}, &allocator
        );
        (void)array_tree_map_clear_and_free(
            &destination, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_tree_map_test_copy_allocate_fail) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[2]){}),
    };
    Array_tree_map source = array_tree_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1
    );
    Array_tree_map destination = array_tree_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    (void)swap_handle(&source, &(struct Val){.id = 0}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 1, .val = 1}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 2, .val = 2}, &allocator);
    check(count(&source).count, 3);
    check(is_empty(&destination), true);
    CCC_Result res
        = array_tree_map_copy(&destination, &source, &(CCC_Allocator){});
    check(res != CCC_RESULT_OK, true);
    check_end({
        (void)array_tree_map_clear_and_free(
            &source, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_tree_map_test_init_from) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_tree_map map_from_list = array_tree_map_from(
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1,
        (struct Val[]){
            {.id = 0, .val = 0},
            {.id = 1, .val = 1},
            {.id = 2, .val = 2},
        }
    );
    check(validate(&map_from_list), true);
    check(count(&map_from_list).count, 3);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map_from_list); i != end(&map_from_list);
         i = next(&map_from_list, i)) {
        struct Val const *const v = array_tree_map_at(&map_from_list, i);
        check(
            (v->id == 0 && v->val == 0) || (v->id == 1 && v->val == 1)
                || (v->id == 2 && v->val == 2),
            true
        );
        ++seen;
    }
    check(seen, 3);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_tree_map_test_init_from_overwrite) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_tree_map map_from_list = array_tree_map_from(
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1,
        (struct Val[]){
            {.id = 0, .val = 0},
            {.id = 0, .val = 1},
            {.id = 0, .val = 2},
        }
    );
    check(validate(&map_from_list), true);
    check(count(&map_from_list).count, 1);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map_from_list); i != end(&map_from_list);
         i = next(&map_from_list, i)) {
        struct Val const *const v = array_tree_map_at(&map_from_list, i);
        check(v->id, 0);
        check(v->val, 2);
        ++seen;
    }
    check(seen, 1);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_tree_map_test_init_from_fail) {
    // Whoops forgot an allocation function.
    Array_tree_map map_from_list = array_tree_map_from(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (CCC_Allocator){},
        0,
        (struct Val[]){
            {.id = 0, .val = 0},
            {.id = 0, .val = 1},
            {.id = 0, .val = 2},
        }
    );
    check(validate(&map_from_list), true);
    check(count(&map_from_list).count, 0);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map_from_list); i != end(&map_from_list);
         i = next(&map_from_list, i)) {
        struct Val const *const v = array_tree_map_at(&map_from_list, i);
        check(v->id, 0);
        check(v->val, 2);
        ++seen;
    }
    check(seen, 0);
    CCC_Handle h = CCC_array_tree_map_insert_or_assign(
        &map_from_list, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(CCC_handle_insert_error(&h), CCC_TRUE);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

check_static_begin(array_tree_map_test_init_with_capacity) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_tree_map map = array_tree_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1
    );
    check(validate(&map), true);
    check(array_tree_map_capacity(&map).count >= SMALL_FIXED_CAP - 1, true);
    for (int i = 0; i < 10; ++i) {
        CCC_Handle const h = CCC_array_tree_map_insert_or_assign(
            &map, &(struct Val){.id = i, .val = i}, &allocator
        );
        check(CCC_handle_insert_error(&h), CCC_FALSE);
        check(array_tree_map_validate(&map), CCC_TRUE);
    }
    check(array_tree_map_count(&map).count, 10);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_tree_map_at(&map, i);
        check(v->id >= 0 && v->id < 10, true);
        check(v->val >= 0 && v->val < 10, true);
        check(v->val, v->id);
        ++seen;
    }
    check(seen, 10);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_tree_map_test_init_with_capacity_no_op) {
    /* Initialize with 0 cap is OK just does nothing. */
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_tree_map map = array_tree_map_with_capacity(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}, allocator, 0
    );
    check(validate(&map), true);
    check(array_tree_map_capacity(&map).count, 0);
    check(array_tree_map_count(&map).count, 0);
    check(
        array_tree_map_reserve(&map, SMALL_FIXED_CAP - 1, &allocator),
        CCC_RESULT_OK
    );
    CCC_Handle const h = CCC_array_tree_map_insert_or_assign(
        &map, &(struct Val){.id = 1, .val = 1}, &allocator
    );
    check(CCC_handle_insert_error(&h), CCC_FALSE);
    check(array_tree_map_validate(&map), CCC_TRUE);
    check(array_tree_map_count(&map).count, 1);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_tree_map_at(&map, i);
        check(v->id, v->val);
        ++seen;
    }
    check(array_tree_map_count(&map).count, 1);
    check(array_tree_map_capacity(&map).count > 0, true);
    check(seen, 1);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_tree_map_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    Array_tree_map map = array_tree_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        (CCC_Allocator){},
        32
    );
    check(validate(&map), true);
    check(array_tree_map_capacity(&map).count, 0);
    CCC_Handle const e = CCC_array_tree_map_insert_or_assign(
        &map, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(CCC_handle_insert_error(&e), CCC_TRUE);
    check(array_tree_map_validate(&map), CCC_TRUE);
    check(array_tree_map_count(&map).count, 0);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_tree_map_at(&map, i);
        check(v->id, v->val);
        ++seen;
    }
    check(array_tree_map_count(&map).count, 0);
    check(seen, 0);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

check_static_begin(array_tree_map_test_context_with_allocator) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_tree_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_tree_map map = CCC_array_tree_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    check(validate(&map), true);
    check(
        array_tree_map_reserve(&map, SMALL_FIXED_CAP - 1, &allocator),
        CCC_RESULT_OK
    );
    check(array_tree_map_capacity(&map).count >= SMALL_FIXED_CAP - 1, true);
    for (int i = 0; i < 10; ++i) {
        CCC_Handle const h = CCC_array_tree_map_insert_or_assign(
            &map, &(struct Val){.id = i, .val = i}, &allocator
        );
        check(CCC_handle_insert_error(&h), CCC_FALSE);
        check(array_tree_map_validate(&map), CCC_TRUE);
    }
    check(array_tree_map_count(&map).count, 10);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_tree_map_at(&map, i);
        check(v->id >= 0 && v->id < 10, true);
        check(v->val >= 0 && v->val < 10, true);
        check(v->val, v->id);
        ++seen;
    }
    check(seen, 10);
    check_end({
        (void)array_tree_map_clear_and_free(
            &map, &(CCC_Destructor){}, &allocator
        );
    });
}

int
main(void) {
    return check_run(
        array_tree_map_test_static(),
        array_tree_map_test_empty(),
        array_tree_map_test_with_literal(),
        array_tree_map_test_copy_no_allocate(),
        array_tree_map_test_copy_no_allocate_fail(),
        array_tree_map_test_copy_allocate(),
        array_tree_map_test_copy_allocate_fail(),
        array_tree_map_test_init_from(),
        array_tree_map_test_init_from_overwrite(),
        array_tree_map_test_init_from_fail(),
        array_tree_map_test_init_with_capacity(),
        array_tree_map_test_init_with_capacity_no_op(),
        array_tree_map_test_init_with_capacity_fail(),
        array_tree_map_test_context_with_allocator()
    );
}
