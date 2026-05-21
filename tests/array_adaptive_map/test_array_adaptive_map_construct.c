#include <stdbool.h>
#include <stddef.h>

#define TRAITS_USING_NAMESPACE_CCC
#define ARRAY_ADAPTIVE_MAP_USING_NAMESPACE_CCC

#include "array_adaptive_map_utility.h"
#include "ccc/array_adaptive_map.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/stack_allocator.h"
#include "utility/std_allocator.h"

static Array_adaptive_map static_map = array_adaptive_map_with_storage(
    id,
    (CCC_Key_comparator){.compare = id_order},
    (struct Val[SMALL_FIXED_CAP]){}
);

static Array_adaptive_map
construct_empty(void) {
    Array_adaptive_map this = array_adaptive_map_default(
        struct Val,
        id,
        (CCC_Key_comparator){
            .compare = id_order,
        }
    );
    return this;
}

check_static_begin(array_adaptive_map_construct_empty) {
    check(array_adaptive_map_validate(NULL), CCC_TRIBOOL_ERROR);
    check(array_adaptive_map_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(array_adaptive_map_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(array_adaptive_map_capacity(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    Array_adaptive_map constructed = construct_empty();
    check(is_empty(&constructed), CCC_TRUE);
    check(validate(&constructed), CCC_TRUE);
    check_end();
}

check_static_begin(array_adaptive_map_test_static) {
    check(array_adaptive_map_capacity(&static_map).count, SMALL_FIXED_CAP);
    check(array_adaptive_map_count(&static_map).count, 0);
    check(validate(&static_map), CCC_TRUE);
    check(is_empty(&static_map), CCC_TRUE);
    CCC_Handle const inserted = array_adaptive_map_insert_or_assign(
        &static_map, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(insert_error(&inserted), CCC_FALSE);
    struct Val const *const v
        = array_adaptive_map_at(&static_map, unwrap(&inserted));
    check(v->val, 1);
    check(v->val, v->id);
    check_end();
}

check_static_begin(array_adaptive_map_test_empty) {
    Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&s), true);
    check_end();
}

check_static_begin(array_adaptive_map_construct_allocator_fixed) {
    Array_adaptive_map allocated = array_adaptive_map_with_allocator_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        std_allocator,
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&allocated), CCC_TRUE);
    check(validate(&allocated), CCC_TRUE);
    check(capacity(&allocated).count > 0, CCC_TRUE);
    check_end(array_adaptive_map_clear_and_free(
                  &allocated, &(CCC_Destructor){}, &std_allocator
    ););
}

check_static_begin(array_adaptive_map_construct_allocator_fixed_fail) {
    Array_adaptive_map allocated = array_adaptive_map_with_allocator_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (CCC_Allocator){},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&allocated), CCC_TRUE);
    check(validate(&allocated), CCC_TRUE);
    check(capacity(&allocated).count, 0);
    check_end(array_adaptive_map_clear_and_free(
                  &allocated, &(CCC_Destructor){}, &std_allocator
    ););
}

check_static_begin(array_adaptive_map_test_with_literal) {
    Array_adaptive_map s = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&s), true);
    check(capacity(&s).count, SMALL_FIXED_CAP);
    check_end();
}

check_static_begin(array_adaptive_map_test_copy_no_allocate) {
    Array_adaptive_map source = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    Array_adaptive_map destination = array_adaptive_map_with_storage(
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
        = array_adaptive_map_copy(&destination, &source, &(CCC_Allocator){});
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

check_static_begin(array_adaptive_map_test_copy_no_allocate_fail) {
    Array_adaptive_map source = array_adaptive_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    Array_adaptive_map destination = array_adaptive_map_with_storage(
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
        = array_adaptive_map_copy(&destination, &source, &(CCC_Allocator){});
    check(res != CCC_RESULT_OK, true);
    check_end();
}

check_static_begin(array_adaptive_map_test_copy_allocate) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_adaptive_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[2]){}),
    };
    Array_adaptive_map source = array_adaptive_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1
    );
    Array_adaptive_map destination = array_adaptive_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    (void)swap_handle(&source, &(struct Val){.id = 0}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 1, .val = 1}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 2, .val = 2}, &allocator);
    check(count(&source).count, 3);
    check(is_empty(&destination), true);
    CCC_Result res = array_adaptive_map_copy(&destination, &source, &allocator);
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
        (void)array_adaptive_map_clear_and_free(
            &source, &(CCC_Destructor){}, &allocator
        );
        (void)array_adaptive_map_clear_and_free(
            &destination, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_adaptive_map_test_copy_allocate_fail) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_adaptive_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[2]){}),
    };
    Array_adaptive_map source = array_adaptive_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1
    );
    Array_adaptive_map destination = array_adaptive_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    (void)swap_handle(&source, &(struct Val){.id = 0}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 1, .val = 1}, &allocator);
    (void)swap_handle(&source, &(struct Val){.id = 2, .val = 2}, &allocator);
    check(count(&source).count, 3);
    check(is_empty(&destination), true);
    CCC_Result res
        = array_adaptive_map_copy(&destination, &source, &(CCC_Allocator){});
    check(res != CCC_RESULT_OK, true);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &source, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

check_static_begin(array_adaptive_map_test_init_from) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_adaptive_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_adaptive_map map_from_list = array_adaptive_map_from(
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
        struct Val const *const v = array_adaptive_map_at(&map_from_list, i);
        check(
            (v->id == 0 && v->val == 0) || (v->id == 1 && v->val == 1)
                || (v->id == 2 && v->val == 2),
            true
        );
        ++seen;
    }
    check(seen, 3);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_adaptive_map_test_init_from_overwrite) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_adaptive_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_adaptive_map map_from_list = array_adaptive_map_from(
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
        struct Val const *const v = array_adaptive_map_at(&map_from_list, i);
        check(v->id, 0);
        check(v->val, 2);
        ++seen;
    }
    check(seen, 1);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_adaptive_map_test_init_from_fail) {
    /* Whoops forgot an allocation function. */
    Array_adaptive_map map_from_list = array_adaptive_map_from(
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
        struct Val const *const v = array_adaptive_map_at(&map_from_list, i);
        check(v->id, 0);
        check(v->val, 2);
        ++seen;
    }
    check(seen, 0);
    CCC_Handle h = CCC_array_adaptive_map_insert_or_assign(
        &map_from_list, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(CCC_handle_insert_error(&h), CCC_TRUE);
    check_end({
        array_adaptive_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

check_static_begin(array_adaptive_map_test_init_with_capacity) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_adaptive_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_adaptive_map map = array_adaptive_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        SMALL_FIXED_CAP - 1
    );
    check(validate(&map), true);
    check(array_adaptive_map_capacity(&map).count >= SMALL_FIXED_CAP - 1, true);
    for (int i = 0; i < 10; ++i) {
        CCC_Handle const h = CCC_array_adaptive_map_insert_or_assign(
            &map, &(struct Val){.id = i, .val = i}, &allocator
        );
        check(CCC_handle_insert_error(&h), CCC_FALSE);
        check(array_adaptive_map_validate(&map), CCC_TRUE);
    }
    check(array_adaptive_map_count(&map).count, 10);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_adaptive_map_at(&map, i);
        check(v->id >= 0 && v->id < 10, true);
        check(v->val >= 0 && v->val < 10, true);
        check(v->val, v->id);
        ++seen;
    }
    check(seen, 10);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &map, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_adaptive_map_test_init_with_capacity_no_op) {
    /* Initialize with 0 cap is OK just does nothing. */
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((typeof(array_adaptive_map_storage_for(
            (struct Val[SMALL_FIXED_CAP]){}
        ))[1]){}),
    };
    Array_adaptive_map map = array_adaptive_map_with_capacity(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}, allocator, 0
    );
    check(validate(&map), true);
    check(array_adaptive_map_capacity(&map).count, 0);
    check(array_adaptive_map_count(&map).count, 0);
    check(
        array_adaptive_map_reserve(&map, SMALL_FIXED_CAP - 1, &allocator),
        CCC_RESULT_OK
    );
    CCC_Handle const h = CCC_array_adaptive_map_insert_or_assign(
        &map, &(struct Val){.id = 1, .val = 1}, &allocator
    );
    check(CCC_handle_insert_error(&h), CCC_FALSE);
    check(array_adaptive_map_validate(&map), CCC_TRUE);
    check(array_adaptive_map_count(&map).count, 1);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_adaptive_map_at(&map, i);
        check(v->id, v->val);
        ++seen;
    }
    check(array_adaptive_map_count(&map).count, 1);
    check(array_adaptive_map_capacity(&map).count > 0, true);
    check(seen, 1);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &map, &(CCC_Destructor){}, &allocator
        );
    });
}

check_static_begin(array_adaptive_map_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    Array_adaptive_map map = array_adaptive_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        (CCC_Allocator){},
        32
    );
    check(validate(&map), true);
    check(array_adaptive_map_capacity(&map).count, 0);
    CCC_Handle const e = CCC_array_adaptive_map_insert_or_assign(
        &map, &(struct Val){.id = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(CCC_handle_insert_error(&e), CCC_TRUE);
    check(array_adaptive_map_validate(&map), CCC_TRUE);
    check(array_adaptive_map_count(&map).count, 0);
    size_t seen = 0;
    for (CCC_Handle_index i = begin(&map); i != end(&map); i = next(&map, i)) {
        struct Val const *const v = array_adaptive_map_at(&map, i);
        check(v->id, v->val);
        ++seen;
    }
    check(array_adaptive_map_count(&map).count, 0);
    check(seen, 0);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &map, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

check_static_begin(array_adaptive_map_test_double_reserve) {
    Array_adaptive_map map = array_adaptive_map_with_capacity(
        struct Val,
        id,
        (CCC_Key_comparator){.compare = id_order},
        std_allocator,
        32
    );
    size_t const cap = array_adaptive_map_capacity(&map).count;
    check(cap >= 32, CCC_TRUE);
    /* Should not over eagerly reserve more space if we have enough cap. */
    check(array_adaptive_map_reserve(&map, 1, &std_allocator), CCC_RESULT_OK);
    check(array_adaptive_map_capacity(&map).count, cap);
    check_end({
        (void)array_adaptive_map_clear_and_free(
            &map, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(array_adaptive_map_test_copy_exhaustion) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for(
            (typeof(array_adaptive_map_storage_for((struct Val[2]){}))[1]){}
        ),
    };
    Array_adaptive_map source = array_adaptive_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    Array_adaptive_map destination = array_adaptive_map_default(
        struct Val, id, (CCC_Key_comparator){.compare = id_order}
    );
    check(
        array_adaptive_map_copy(&destination, &source, &allocator),
        CCC_RESULT_OK
    );
    check(array_adaptive_map_reserve(&source, 1, &allocator), CCC_RESULT_OK);
    check(
        array_adaptive_map_copy(&destination, &source, &allocator),
        CCC_RESULT_ALLOCATOR_ERROR
    );
    source.data = NULL;
    stack_allocator_reset(allocator.context);
    check(
        array_adaptive_map_copy(&destination, &source, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check_end();
}

int
main(void) {
    return check_run(
        array_adaptive_map_construct_empty(),
        array_adaptive_map_test_static(),
        array_adaptive_map_test_empty(),
        array_adaptive_map_construct_allocator_fixed(),
        array_adaptive_map_construct_allocator_fixed_fail(),
        array_adaptive_map_test_with_literal(),
        array_adaptive_map_test_copy_no_allocate(),
        array_adaptive_map_test_copy_no_allocate_fail(),
        array_adaptive_map_test_copy_allocate(),
        array_adaptive_map_test_copy_allocate_fail(),
        array_adaptive_map_test_init_from(),
        array_adaptive_map_test_init_from_overwrite(),
        array_adaptive_map_test_init_from_fail(),
        array_adaptive_map_test_init_with_capacity(),
        array_adaptive_map_test_init_with_capacity_no_op(),
        array_adaptive_map_test_init_with_capacity_fail(),
        array_adaptive_map_test_double_reserve(),
        array_adaptive_map_test_copy_exhaustion(),
    );
}
