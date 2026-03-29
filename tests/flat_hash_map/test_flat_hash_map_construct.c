#include <assert.h>
#include <stddef.h>

#define FLAT_HASH_MAP_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "ccc/flat_hash_map.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "flat_hash_map_utility.h"
#include "utility/allocate.h"

static void
mod(CCC_Arguments const u) {
    struct Val *v = u.type;
    v->val += 5;
}

static void
modw(CCC_Arguments const u) {
    struct Val *v = u.type;
    v->val = *((int *)u.context);
}

static Flat_hash_map static_fh = flat_hash_map_with_storage(
    key,
    ((CCC_Hasher){
        .hash = flat_hash_map_int_to_u64,
        .compare = flat_hash_map_id_order,
    }),
    (struct Val[SMALL_FIXED_CAP]){}
);

static Flat_hash_map
construct_empty(void) {
    return flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
}

check_static_begin(flat_hash_map_construct_empty) {
    Flat_hash_map constructed = construct_empty();
    check(is_empty(&constructed), CCC_TRUE);
    check(validate(&constructed), CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_static_initialize) {
    check(flat_hash_map_capacity(&static_fh).count, SMALL_FIXED_CAP);
    check(flat_hash_map_count(&static_fh).count, 0);
    check(validate(&static_fh), CCC_TRUE);
    check(is_empty(&static_fh), CCC_TRUE);
    struct Val def = {.key = 137, .val = 0};

    /* Returning a vacant entry is possible when modification is attempted. */
    Flat_hash_map_entry *ent = and_modify(
        flat_hash_map_entry_wrap(&static_fh, &def.key, &(CCC_Allocator){}),
        &(CCC_Modifier){.modify = mod}
    );
    check(occupied(ent), CCC_FALSE);
    check((unwrap(ent) == NULL), CCC_TRUE);

    /* Inserting default value before an in place modification is possible. */
    struct Val *v = or_insert(
        flat_hash_map_entry_wrap(&static_fh, &def.key, &(CCC_Allocator){}), &def
    );
    check(v != NULL, CCC_TRUE);
    v->val++;
    struct Val const *const inserted = get_key_value(&static_fh, &def.key);
    check((inserted != NULL), CCC_TRUE);
    check(inserted->key, 137);
    check(inserted->val, 1);

    /* Modifying an existing value or inserting default is possible when no
       context input is needed. */
    struct Val *v2 = or_insert(
        and_modify(
            flat_hash_map_entry_wrap(&static_fh, &def.key, &(CCC_Allocator){}),
            &(CCC_Modifier){.modify = mod}
        ),
        &def
    );
    check((v2 != NULL), CCC_TRUE);
    check(inserted->key, 137);
    check(v2->val, 6);

    /* Modifying an existing value that requires external input is also
       possible with slightly different signature. */
    struct Val *v3 = or_insert(
        and_modify(
            flat_hash_map_entry_wrap(&static_fh, &def.key, &(CCC_Allocator){}),
            &(CCC_Modifier){
                .modify = modw,
                .context = &def.key,
            }
        ),
        &def
    );
    check((v3 != NULL), CCC_TRUE);
    check(inserted->key, 137);
    check(v3->val, 137);
    check_end();
}

check_static_begin(flat_hash_map_test_with_literal) {
    Flat_hash_map fh = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(flat_hash_map_capacity(&fh).count, SMALL_FIXED_CAP);
    check(flat_hash_map_count(&fh).count, 0);
    check(validate(&fh), CCC_TRUE);
    check(is_empty(&fh), CCC_TRUE);
    struct Val def = {.key = 137, .val = 0};

    /* Returning a vacant entry is possible when modification is attempted. */
    Flat_hash_map_entry *ent = and_modify(
        flat_hash_map_entry_wrap(&fh, &def.key, &(CCC_Allocator){}),
        &(CCC_Modifier){.modify = mod}
    );
    check(occupied(ent), CCC_FALSE);
    check((unwrap(ent) == NULL), CCC_TRUE);

    /* Inserting default value before an in place modification is possible. */
    struct Val *v = or_insert(
        flat_hash_map_entry_wrap(&fh, &def.key, &(CCC_Allocator){}), &def
    );
    check(v != NULL, CCC_TRUE);
    v->val++;
    struct Val const *const inserted = get_key_value(&fh, &def.key);
    check((inserted != NULL), CCC_TRUE);
    check(inserted->key, 137);
    check(inserted->val, 1);

    /* Modifying an existing value or inserting default is possible when no
       context input is needed. */
    struct Val *v2 = or_insert(
        and_modify(
            flat_hash_map_entry_wrap(&fh, &def.key, &(CCC_Allocator){}),
            &(CCC_Modifier){.modify = mod}
        ),
        &def
    );
    check((v2 != NULL), CCC_TRUE);
    check(inserted->key, 137);
    check(v2->val, 6);

    /* Modifying an existing value that requires external input is also
       possible with slightly different signature. */
    struct Val *v3 = or_insert(
        and_modify(
            flat_hash_map_entry_wrap(&fh, &def.key, &(CCC_Allocator){}),
            &(CCC_Modifier){
                .modify = modw,
                .context = &def.key,
            }
        ),
        &def
    );
    check((v3 != NULL), CCC_TRUE);
    check(inserted->key, 137);
    check(v3->val, 137);
    check_end();
}

check_static_begin(flat_hash_map_test_copy_no_allocate) {
    Flat_hash_map source = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[SMALL_FIXED_CAP]){}
    );
    Flat_hash_map destination = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    (void)swap_entry(&source, &(struct Val){.key = 0}, &(CCC_Allocator){});
    (void)swap_entry(
        &source, &(struct Val){.key = 1, .val = 1}, &(CCC_Allocator){}
    );
    (void)swap_entry(
        &source, &(struct Val){.key = 2, .val = 2}, &(CCC_Allocator){}
    );
    check(count(&source).count, 3);
    check(is_empty(&destination), CCC_TRUE);
    CCC_Result res
        = flat_hash_map_copy(&destination, &source, &(CCC_Allocator){});
    check(res, CCC_RESULT_OK);
    check(count(&destination).count, count(&source).count);
    for (int i = 0; i < 3; ++i) {
        CCC_Entry source_e
            = CCC_remove_key_value(&source, &(struct Val){.key = i});
        CCC_Entry destination_e
            = CCC_remove_key_value(&destination, &(struct Val){.key = i});
        check(occupied(&source_e), occupied(&destination_e));
    }
    check(is_empty(&source), is_empty(&destination));
    check(is_empty(&destination), CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_copy_no_allocate_fail) {
    Flat_hash_map source = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    Flat_hash_map destination = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[SMALL_FIXED_CAP]){}
    );
    (void)swap_entry(&source, &(struct Val){.key = 0}, &(CCC_Allocator){});
    (void)swap_entry(
        &source, &(struct Val){.key = 1, .val = 1}, &(CCC_Allocator){}
    );
    (void)swap_entry(
        &source, &(struct Val){.key = 2, .val = 2}, &(CCC_Allocator){}
    );
    check(count(&source).count, 3);
    check(is_empty(&destination), CCC_TRUE);
    CCC_Result res = flat_hash_map_copy(&destination, &source, NULL);
    check(res != CCC_RESULT_OK, CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_copy_allocate) {
    Flat_hash_map destination = flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }
    );
    Flat_hash_map source = flat_hash_map_from(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        std_allocator,
        0,
        (struct Val[]){
            {.key = 0},
            {.key = 1, .val = 1},
            {.key = 2, .val = 2},
        }
    );
    check(count(&source).count, 3);
    check(is_empty(&destination), CCC_TRUE);
    CCC_Result res = flat_hash_map_copy(&destination, &source, &std_allocator);
    check(res, CCC_RESULT_OK);
    check(count(&destination).count, count(&source).count);
    for (int i = 0; i < 3; ++i) {
        CCC_Entry source_e
            = CCC_remove_key_value(&source, &(struct Val){.key = i});
        CCC_Entry destination_e
            = CCC_remove_key_value(&destination, &(struct Val){.key = i});
        check(occupied(&source_e), occupied(&destination_e));
    }
    check(is_empty(&source), is_empty(&destination));
    check(is_empty(&destination), CCC_TRUE);
    check_end({
        (void)flat_hash_map_clear_and_free(
            &source, &(CCC_Destructor){}, &std_allocator
        );
        (void)flat_hash_map_clear_and_free(
            &destination, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(flat_hash_map_test_copy_allocate_fail) {
    Flat_hash_map source = flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }
    );
    Flat_hash_map destination = flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }
    );
    (void)swap_entry(&source, &(struct Val){.key = 0}, &std_allocator);
    (void)swap_entry(
        &source, &(struct Val){.key = 1, .val = 1}, &std_allocator
    );
    (void)swap_entry(
        &source, &(struct Val){.key = 2, .val = 2}, &std_allocator
    );
    check(count(&source).count, 3);
    check(is_empty(&destination), CCC_TRUE);
    CCC_Result res
        = flat_hash_map_copy(&destination, &source, &(CCC_Allocator){});
    check(res != CCC_RESULT_OK, CCC_TRUE);
    check_end({
        (void)flat_hash_map_clear_and_free(
            &source, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(flat_hash_map_test_empty) {
    Flat_hash_map fh = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[SMALL_FIXED_CAP]){}
    );
    check(is_empty(&fh), CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_init_from) {
    Flat_hash_map map_from_list = flat_hash_map_from(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        std_allocator,
        0,
        (struct Val[]){
            {.key = 0, .val = 0},
            {.key = 1, .val = 1},
            {.key = 2, .val = 2},
        }
    );
    check(validate(&map_from_list), CCC_TRUE);
    check(count(&map_from_list).count, 3);
    size_t seen = 0;
    for (struct Val const *i = begin(&map_from_list); i != end(&map_from_list);
         i = next(&map_from_list, i)) {
        check(
            (i->key == 0 && i->val == 0) || (i->key == 1 && i->val == 1)
                || (i->key == 2 && i->val == 2),
            CCC_TRUE
        );
        ++seen;
    }
    check(seen, 3);
    check_end({
        flat_hash_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(flat_hash_map_test_init_from_overwrite) {
    Flat_hash_map map_from_list = flat_hash_map_from(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        std_allocator,
        0,
        (struct Val[]){
            {.key = 0, .val = 0},
            {.key = 0, .val = 1},
            {.key = 0, .val = 2},
        }
    );
    check(validate(&map_from_list), CCC_TRUE);
    check(count(&map_from_list).count, 1);
    size_t seen = 0;
    for (struct Val const *i = begin(&map_from_list); i != end(&map_from_list);
         i = next(&map_from_list, i)) {
        check(i->key, 0);
        check(i->val, 2);
        ++seen;
    }
    check(seen, 1);
    check_end({
        flat_hash_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(flat_hash_map_test_init_from_fail) {
    // Whoops forgot an allocation function.
    Flat_hash_map map_from_list = flat_hash_map_from(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (CCC_Allocator){},
        0,
        (struct Val[]){
            {.key = 0, .val = 0},
            {.key = 0, .val = 1},
            {.key = 0, .val = 2},
        }
    );
    check(validate(&map_from_list), CCC_TRUE);
    check(count(&map_from_list).count, 0);
    size_t seen = 0;
    for (struct Val const *i = begin(&map_from_list); i != end(&map_from_list);
         i = next(&map_from_list, i)) {
        check(i->key, 0);
        check(i->val, 2);
        ++seen;
    }
    check(seen, 0);
    CCC_Entry e = CCC_flat_hash_map_insert_or_assign(
        &map_from_list, &(struct Val){.key = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(CCC_entry_insert_error(&e), CCC_TRUE);
    check_end({
        flat_hash_map_clear_and_free(
            &map_from_list, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

check_static_begin(flat_hash_map_test_init_with_capacity) {
    Flat_hash_map fh = flat_hash_map_with_capacity(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        std_allocator,
        32
    );
    check(validate(&fh), CCC_TRUE);
    check(flat_hash_map_capacity(&fh).count >= 32, CCC_TRUE);
    for (int i = 0; i < 10; ++i) {
        CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
            &fh, &(struct Val){.key = i, .val = i}, &std_allocator
        );
        check(CCC_entry_insert_error(&e), CCC_FALSE);
        check(flat_hash_map_validate(&fh), CCC_TRUE);
    }
    check(flat_hash_map_count(&fh).count, 10);
    size_t seen = 0;
    for (struct Val const *i = begin(&fh); i != end(&fh); i = next(&fh, i)) {
        check(i->key >= 0 && i->key < 10, CCC_TRUE);
        check(i->val >= 0 && i->val < 10, CCC_TRUE);
        check(i->val, i->key);
        ++seen;
    }
    check(seen, 10);
    check_end({
        flat_hash_map_clear_and_free(&fh, &(CCC_Destructor){}, &std_allocator);
    });
}

check_static_begin(flat_hash_map_test_init_with_capacity_no_op) {
    /* Initialize with 0 cap is OK just does nothing. */
    Flat_hash_map fh = flat_hash_map_with_capacity(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        std_allocator,
        0
    );
    check(validate(&fh), CCC_TRUE);
    check(flat_hash_map_capacity(&fh).count, 0);
    check(flat_hash_map_count(&fh).count, 0);
    CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
        &fh, &(struct Val){.key = 1, .val = 1}, &std_allocator
    );
    check(CCC_entry_insert_error(&e), CCC_FALSE);
    check(flat_hash_map_validate(&fh), CCC_TRUE);
    check(flat_hash_map_count(&fh).count, 1);
    size_t seen = 0;
    for (struct Val const *i = begin(&fh); i != end(&fh); i = next(&fh, i)) {
        check(i->key, i->val);
        ++seen;
    }
    check(flat_hash_map_count(&fh).count, 1);
    check(flat_hash_map_capacity(&fh).count > 0, CCC_TRUE);
    check(seen, 1);
    check_end({
        flat_hash_map_clear_and_free(&fh, &(CCC_Destructor){}, &std_allocator);
    });
}

check_static_begin(flat_hash_map_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    Flat_hash_map fh = flat_hash_map_with_capacity(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (CCC_Allocator){},
        32
    );
    check(validate(&fh), CCC_TRUE);
    check(flat_hash_map_capacity(&fh).count, 0);
    CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
        &fh, &(struct Val){.key = 1, .val = 1}, &(CCC_Allocator){}
    );
    check(CCC_entry_insert_error(&e), CCC_TRUE);
    check(flat_hash_map_validate(&fh), CCC_TRUE);
    check(flat_hash_map_count(&fh).count, 0);
    size_t seen = 0;
    for (struct Val const *i = begin(&fh); i != end(&fh); i = next(&fh, i)) {
        check(i->key, i->val);
        ++seen;
    }
    check(flat_hash_map_count(&fh).count, 0);
    check(seen, 0);
    check_end({
        flat_hash_map_clear_and_free(
            &fh, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

int
main(void) {
    return check_run(
        flat_hash_map_construct_empty(),
        flat_hash_map_test_static_initialize(),
        flat_hash_map_test_with_literal(),
        flat_hash_map_test_copy_no_allocate(),
        flat_hash_map_test_copy_no_allocate_fail(),
        flat_hash_map_test_copy_allocate(),
        flat_hash_map_test_copy_allocate_fail(),
        flat_hash_map_test_empty(),
        flat_hash_map_test_init_from(),
        flat_hash_map_test_init_from_overwrite(),
        flat_hash_map_test_init_from_fail(),
        flat_hash_map_test_init_with_capacity(),
        flat_hash_map_test_init_with_capacity_no_op(),
        flat_hash_map_test_init_with_capacity_fail()
    );
}
