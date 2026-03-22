/** This file dedicated to testing the Handle Interface. The interface has
grown significantly requiring a dedicated file to test all code paths in all
the handle functions. */
#include <stddef.h>
#include <stdlib.h>

#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "array_tree_map.h"
#include "array_tree_map_utility.h"
#include "checkers.h"
#include "traits.h"
#include "types.h"

static inline struct Val
val(int const val) {
    return (struct Val){.val = val};
}

static inline struct Val
idval(int const id, int const val) {
    return (struct Val){.id = id, .val = val};
}

static inline void
plus(CCC_Arguments const t) {
    ((struct Val *)t.type)->val++;
}

static inline void
pluscontext(CCC_Arguments const t) {
    ((struct Val *)t.type)->val += *(int *)t.context;
}

/* Every test should have three uses of each tested function: one when the
   container is empty, one when the container has a few elements and one when
   the container has many elements. If the function has different behavior
   given an element being present or absent, each possibility should be
   tested at each of those three stages. */

/* Fills the container with n elements with id and val starting at the provided
   value and incrementing by 1 until n is reached. Assumes id_and_val are
   not present by key in the table and all subsequent inserts are unique. */
check_static_begin(
    fill_n,
    CCC_Array_tree_map *const array_tree_map,
    size_t const n,
    int id_and_val,
    CCC_Allocator const *const allocator
) {
    for (size_t i = 0; i < n; ++i, ++id_and_val) {
        CCC_Handle hndl = swap_handle(
            array_tree_map,
            &(struct Val){.id = id_and_val, .val = id_and_val},
            allocator
        );
        check(insert_error(&hndl), false);
        check(occupied(&hndl), false);
        check(validate(array_tree_map), true);
    }
    check_end();
}

/* Internally there is some maintenance to perform when swapping values for
   the user on insert. Leave this test here to always catch this. */
check_static_begin(array_tree_map_test_validate) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    CCC_Handle hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, 1);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    check_end();
}

check_static_begin(array_tree_map_test_insert) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Handle hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, 1);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, i);
    check(v->id, i);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, i);
    check(v->id, i);
    check_end();
}

check_static_begin(array_tree_map_test_remove_key_value) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Handle hndl = CCC_remove_key_value(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, 0);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, 1);
    struct Val old = {.id = -1};
    hndl = CCC_remove_key_value(&array_tree_map, &old);
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, 0);
    check(old.val, -1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = CCC_remove_key_value(
        &array_tree_map, &(struct Val){.id = i, .val = i}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 1);
    old = (struct Val){.id = i};
    hndl = CCC_remove_key_value(&array_tree_map, &old);
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i);
    check(old.val, i);
    check(old.id, i);

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = CCC_remove_key_value(
        &array_tree_map, &(struct Val){.id = i, .val = i}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i);
    hndl = swap_handle(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 1);
    old = (struct Val){.id = i};
    hndl = CCC_remove_key_value(&array_tree_map, &old);
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i);
    check(old.val, i);
    check(old.id, i);
    check_end();
}

check_static_begin(array_tree_map_test_try_insert) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Handle hndl = try_insert(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, 1);
    hndl = try_insert(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = try_insert(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = try_insert(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, i);
    check(v->id, i);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = try_insert(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = try_insert(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, i);
    check(v->id, i);
    check_end();
}

check_static_begin(array_tree_map_test_try_insert_with) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Handle *hndl = array_tree_map_try_insert_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-1)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, 1);
    hndl = array_tree_map_try_insert_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-1)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = array_tree_map_try_insert_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = array_tree_map_try_insert_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i);
    check(v->id, i);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = array_tree_map_try_insert_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = array_tree_map_try_insert_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i);
    check(v->id, i);
    check_end();
}

check_static_begin(array_tree_map_test_insert_or_assign) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Handle hndl = insert_or_assign(
        &array_tree_map, &(struct Val){.id = -1, .val = -1}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, 1);
    hndl = insert_or_assign(
        &array_tree_map, &(struct Val){.id = -1, .val = -2}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, -2);
    check(v->id, -1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = insert_or_assign(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = insert_or_assign(
        &array_tree_map,
        &(struct Val){.id = i, .val = i + 1},
        &(CCC_Allocator){}
    );
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = insert_or_assign(
        &array_tree_map, &(struct Val){.id = i, .val = i}, &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = insert_or_assign(
        &array_tree_map,
        &(struct Val){.id = i, .val = i + 1},
        &(CCC_Allocator){}
    );
    check(validate(&array_tree_map), true);
    check(occupied(&hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(&hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check_end();
}

check_static_begin(array_tree_map_test_insert_or_assign_with) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Handle *hndl = array_tree_map_insert_or_assign_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-1)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, 1);
    hndl = array_tree_map_insert_or_assign_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-2)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, -2);
    check(v->id, -1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i + 1)
    );
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 2);
    hndl = array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i + 1)
    );
    check(validate(&array_tree_map), true);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check_end();
}

check_static_begin(array_tree_map_test_array_and_modify) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Array_tree_map_handle *hndl
        = array_tree_map_handle_wrap(&array_tree_map, &(int){-1});
    check(validate(&array_tree_map), true);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, 0);
    hndl = and_modify(hndl, &(CCC_Modifier){.modify = plus});
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, 0);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-1)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &(int){-1});
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    hndl = and_modify(hndl, &(CCC_Modifier){.modify = plus});
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, 0);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 1);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, i + 2);
    hndl = and_modify(hndl, &(CCC_Modifier){.modify = plus});
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 1);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, i + 2);
    hndl = and_modify(hndl, &(CCC_Modifier){.modify = plus});
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check_end();
}

check_static_begin(array_tree_map_test_array_and_context_modify) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    int context = 1;
    CCC_Array_tree_map_handle *hndl
        = array_tree_map_handle_wrap(&array_tree_map, &(int){-1});
    hndl = and_modify(
        hndl, &(CCC_Modifier){.modify = pluscontext, .context = &context}
    );
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, 0);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-1)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &(int){-1});
    check(occupied(hndl), true);
    check(count(&array_tree_map).count, 1);
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    hndl = and_modify(
        hndl, &(CCC_Modifier){.modify = pluscontext, .context = &context}
    );
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, 0);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = and_modify(
        hndl, &(CCC_Modifier){.modify = pluscontext, .context = &context}
    );
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 1);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = and_modify(
        hndl, &(CCC_Modifier){.modify = pluscontext, .context = &context}
    );
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check(count(&array_tree_map).count, i + 2);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = and_modify(
        hndl, &(CCC_Modifier){.modify = pluscontext, .context = &context}
    );
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 1);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = and_modify(
        hndl, &(CCC_Modifier){.modify = pluscontext, .context = &context}
    );
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check(count(&array_tree_map).count, i + 2);
    check_end();
}

check_static_begin(array_tree_map_test_array_and_modify_with) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    CCC_Array_tree_map_handle *hndl
        = array_tree_map_handle_wrap(&array_tree_map, &(int){-1});
    hndl = array_tree_map_and_modify_with(hndl, struct Val *, { T->val++; });
    check(count(&array_tree_map).count, 0);
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, 0);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, -1, &(CCC_Allocator){}, val(-1)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &(int){-1});
    struct Val *v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, -1);
    check(v->id, -1);
    hndl = array_tree_map_and_modify_with(hndl, struct Val *, { T->val++; });
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, 0);
    check(count(&array_tree_map).count, 1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = array_tree_map_and_modify_with(hndl, struct Val *, { T->val++; });
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 1);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = array_tree_map_and_modify_with(hndl, struct Val *, { T->val++; });
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check(count(&array_tree_map).count, i + 2);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = array_tree_map_and_modify_with(hndl, struct Val *, { T->val++; });
    check(occupied(hndl), false);
    check(count(&array_tree_map).count, i + 1);
    (void)array_tree_map_insert_or_assign_with(
        &array_tree_map, i, &(CCC_Allocator){}, val(i)
    );
    check(validate(&array_tree_map), true);
    hndl = array_tree_map_handle_wrap(&array_tree_map, &i);
    hndl = array_tree_map_and_modify_with(hndl, struct Val *, { T->val++; });
    v = array_tree_map_at(&array_tree_map, unwrap(hndl));
    check(v != NULL, true);
    check(v->val, i + 1);
    check(v->id, i);
    check(count(&array_tree_map).count, i + 2);
    check_end();
}

check_static_begin(array_tree_map_test_or_insert) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    struct Val *v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(struct Val){.id = -1, .val = -1},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(struct Val){.id = -1, .val = -2},
            &(CCC_Allocator){}
        )
    );
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i + 1},
            &(CCC_Allocator){}
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i + 1},
            &(CCC_Allocator){}
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    check_end();
}

check_static_begin(array_tree_map_test_or_insert_with) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    struct Val *v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_or_insert_with(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(CCC_Allocator){},
            idval(-1, -1)
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_or_insert_with(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(CCC_Allocator){},
            idval(-1, -2)
        )
    );
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_or_insert_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i)
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_or_insert_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i + 1)
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_or_insert_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i)
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_or_insert_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i + 1)
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    check_end();
}

check_static_begin(array_tree_map_test_insert_handle) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    struct Val *v = array_tree_map_at(
        &array_tree_map,
        insert_handle(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(struct Val){.id = -1, .val = -1},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    v = array_tree_map_at(
        &array_tree_map,
        insert_handle(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(struct Val){.id = -1, .val = -2},
            &(CCC_Allocator){}
        )
    );
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -2);
    check(count(&array_tree_map).count, 1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    v = array_tree_map_at(
        &array_tree_map,
        insert_handle(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        insert_handle(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i + 1},
            &(CCC_Allocator){}
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i + 1);
    check(count(&array_tree_map).count, i + 2);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    v = array_tree_map_at(
        &array_tree_map,
        insert_handle(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        insert_handle(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i + 1},
            &(CCC_Allocator){}
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i + 1);
    check(count(&array_tree_map).count, i + 2);
    check_end();
}

check_static_begin(array_tree_map_test_insert_handle_with) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    struct Val *v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_insert_handle_with(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(CCC_Allocator){},
            idval(-1, -1)
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_insert_handle_with(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(CCC_Allocator){},
            idval(-1, -2)
        )
    );
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -2);
    check(count(&array_tree_map).count, 1);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_insert_handle_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i)
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_insert_handle_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i + 1)
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i + 1);
    check(count(&array_tree_map).count, i + 2);
    ++i;

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_insert_handle_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i)
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 2);
    v = array_tree_map_at(
        &array_tree_map,
        array_tree_map_insert_handle_with(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(CCC_Allocator){},
            idval(i, i + 1)
        )
    );
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i + 1);
    check(count(&array_tree_map).count, i + 2);
    check_end();
}

check_static_begin(array_tree_map_test_remove_handle) {
    CCC_Array_tree_map array_tree_map = array_tree_map_with_storage(
        id,
        (CCC_Key_comparator){.compare = id_order},
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int size = 30;
    struct Val *v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &(int){-1}),
            &(struct Val){.id = -1, .val = -1},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 1);
    CCC_Handle *e = array_tree_map_remove_handle_wrap(
        array_tree_map_handle_wrap(&array_tree_map, &(int){-1})
    );
    check(validate(&array_tree_map), true);
    check(occupied(e), true);
    v = array_tree_map_at(&array_tree_map, unwrap(e));
    check(v != NULL, true);
    check(v->id, -1);
    check(v->val, -1);
    check(count(&array_tree_map).count, 0);
    int i = 0;

    check(
        fill_n(&array_tree_map, (size_t)size / 2, i, &(CCC_Allocator){}),
        CHECK_PASS
    );

    i += (size / 2);
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 1);
    e = array_tree_map_remove_handle_wrap(
        array_tree_map_handle_wrap(&array_tree_map, &i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(e), true);
    v = array_tree_map_at(&array_tree_map, unwrap(e));
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i);

    check(
        fill_n(
            &array_tree_map, (size_t)size - (size_t)i, i, &(CCC_Allocator){}
        ),
        CHECK_PASS
    );

    i = size;
    v = array_tree_map_at(
        &array_tree_map,
        or_insert(
            array_tree_map_handle_wrap(&array_tree_map, &i),
            &(struct Val){.id = i, .val = i},
            &(CCC_Allocator){}
        )
    );
    check(validate(&array_tree_map), true);
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i + 1);
    e = array_tree_map_remove_handle_wrap(
        array_tree_map_handle_wrap(&array_tree_map, &i)
    );
    check(validate(&array_tree_map), true);
    check(occupied(e), true);
    v = array_tree_map_at(&array_tree_map, unwrap(e));
    check(v != NULL, true);
    check(v->id, i);
    check(v->val, i);
    check(count(&array_tree_map).count, i);
    check_end();
}

int
main(void) {
    return check_run(
        array_tree_map_test_insert(),
        array_tree_map_test_remove_key_value(),
        array_tree_map_test_validate(),
        array_tree_map_test_try_insert(),
        array_tree_map_test_try_insert_with(),
        array_tree_map_test_insert_or_assign(),
        array_tree_map_test_insert_or_assign_with(),
        array_tree_map_test_array_and_modify(),
        array_tree_map_test_array_and_context_modify(),
        array_tree_map_test_array_and_modify_with(),
        array_tree_map_test_or_insert(),
        array_tree_map_test_or_insert_with(),
        array_tree_map_test_insert_handle(),
        array_tree_map_test_insert_handle_with(),
        array_tree_map_test_remove_handle()
    );
}
