#include <stddef.h>
#include <stdlib.h>

#define FLAT_BITSET_USING_NAMESPACE_CCC
#define FLAT_HASH_MAP_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "ccc/flat_bitset.h"
#include "ccc/flat_hash_map.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "flat_hash_map_utility.h"
#include "utility/allocate.h"
#include "utility/random.h"
#include "utility/stack_allocator.h"

struct Owner {
    int key;
    void *allocation;
};

static CCC_Order
owners_eq(CCC_Key_comparator_arguments const order) {
    int const *const left = order.key_left;
    struct Owner const *const right = order.type_right;
    return (*left > right->key) - (*left < right->key);
}

static void
destroy_owner_allocation(CCC_Arguments const t) {
    struct Owner const *const o = t.type;
    free(o->allocation);
}

check_static_begin(flat_hash_map_test_insert_then_iterate) {
    CCC_Flat_hash_map fh = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    int const size = STANDARD_FIXED_CAP;
    for (int i = 0; i < size; i += 2) {
        CCC_Entry e = try_insert(
            &fh, &(struct Val){.key = i, .val = i}, &(CCC_Allocator){}
        );
        check(occupied(&e), false);
        check(validate(&fh), true);
        e = try_insert(
            &fh, &(struct Val){.key = i, .val = i}, &(CCC_Allocator){}
        );
        check(occupied(&e), true);
        check(validate(&fh), true);
        struct Val const *const v = unwrap(&e);
        check(v == NULL, false);
        check(v->key, i);
        check(v->val, i);
    }
    check(CCC_flat_hash_map_next(&fh, (void *)0xFFFFFFFFFFFFFFFF), NULL);
    int seen = 0;
    for (int i = 0; i < size; i += 2) {
        check(contains(&fh, &i), true);
        check(
            occupied(flat_hash_map_entry_wrap(&fh, &i, &(CCC_Allocator){})),
            true
        );
        check(validate(&fh), true);
        ++seen;
    }
    check((size_t)seen, count(&fh).count);
    int seen2 = 0;
    for (struct Val const *i = begin(&fh); i != end(&fh); i = next(&fh, i)) {
        check(i->val % 2 == 0, true);
        ++seen2;
    }
    check(seen, seen2);
    check_end();
}

check_static_begin(flat_hash_map_test_insert_delete_then_iterate) {
    CCC_Flat_hash_map fh = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    for (int i = 0; i < (int)STANDARD_FIXED_CAP; i += 2) {
        CCC_Entry e = try_insert(
            &fh, &(struct Val){.key = i, .val = i}, &(CCC_Allocator){}
        );
        check(occupied(&e), false);
        check(validate(&fh), true);
        e = try_insert(
            &fh, &(struct Val){.key = i, .val = i}, &(CCC_Allocator){}
        );
        check(occupied(&e), true);
        check(validate(&fh), true);
        struct Val const *const v = unwrap(&e);
        check(v == NULL, false);
        check(v->key, i);
        check(v->val, i);
    }
    int seen = 0;
    for (int i = 0; i < (int)STANDARD_FIXED_CAP / 2; i += 2) {
        check(contains(&fh, &i), true);
        CCC_Entry const e
            = flat_hash_map_remove_key_value(&fh, &(struct Val){.key = i});
        check(occupied(&e), true);
        check(validate(&fh), true);
        ++seen;
    }
    check((size_t)seen, count(&fh).count);
    int seen2 = 0;
    for (struct Val const *i = begin(&fh); i != end(&fh); i = next(&fh, i)) {
        check(i->val % 2 == 0, true);
        ++seen2;
    }
    check(seen, seen2);
    check_end();
}

/** We want to make sure the clear and free method that uses the more
efficient iterator is able to free all elements allocated with no leaks when
run under sanitizers. */
check_static_begin(flat_hash_map_test_insert_allocate_clear_free) {
    CCC_Flat_hash_map fh = flat_hash_map_default(
        struct Owner,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = owners_eq,
        })
    );
    int const size = 32;
    for (int i = 0; i < size; ++i) {
        CCC_Entry *e = flat_hash_map_try_insert_with(
            &fh,
            i,
            &std_allocator,
            (struct Owner){.allocation = malloc(sizeof(size_t))}
        );
        check(occupied(e), CCC_FALSE);
        struct Owner const *const o = unwrap(e);
        check(o != NULL, CCC_TRUE);
        check(o->allocation != NULL, CCC_TRUE);
    }
    check_end({
        CCC_flat_hash_map_clear_and_free(
            &fh,
            &(CCC_Destructor){.destroy = destroy_owner_allocation},
            &std_allocator
        );
    });
}

check_static_begin(flat_hash_map_test_insert_clear_insert_determinism) {
    CCC_Flat_hash_map h = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[SMALL_FIXED_CAP]){}
    );
    int to_insert[SMALL_FIXED_CAP] = {};
    int key_order[SMALL_FIXED_CAP] = {};
    iota(to_insert, SMALL_FIXED_CAP, 0);
    rand_shuffle(sizeof(int), to_insert, SMALL_FIXED_CAP, &(int){});
    int i = 0;
    for (;;) {
        int const cur = to_insert[i];
        struct Val const *const v = unwrap(flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Val){.val = i}
        ));
        if (!v) {
            break;
        }
        check(v->key, to_insert[i]);
        check(v->val, i);
        check(validate(&h), true);
        ++i;
    }
    size_t const full_size = count(&h).count;
    i = 0;
    for (struct Val const *e = flat_hash_map_begin(&h);
         e != flat_hash_map_end(&h);
         e = flat_hash_map_next(&h, e)) {
        key_order[i++] = e->key;
    }
    i = 0;
    flat_hash_map_clear(&h, &(CCC_Destructor){});
    for (;;) {
        int const cur = to_insert[i];
        struct Val const *const v = unwrap(flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Val){.val = i}
        ));
        if (!v) {
            break;
        }
        check(v->key, to_insert[i]);
        check(v->val, i);
        check(validate(&h), true);
        ++i;
    }
    check(full_size, (size_t)i);
    i = 0;
    for (struct Val const *e = flat_hash_map_begin(&h);
         e != flat_hash_map_end(&h);
         e = flat_hash_map_next(&h, e)) {
        check(key_order[i++], e->key);
    }
    check_end();
}

static void
destroy_element(CCC_Arguments const arguments) {
    struct Val const *const i = arguments.type;
    Flat_bitset *const is_destroyed_buffer = arguments.context;
    (void)flat_bitset_set(is_destroyed_buffer, (size_t)i->key, CCC_TRUE);
}

check_static_begin(flat_hash_map_test_clear_with_destructor) {
    enum : size_t {
        MIN_CAP = 16
    };
    CCC_Flat_hash_map h = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[MIN_CAP]){}
    );
    Flat_bitset is_destroyed = flat_bitset_with_storage(0, (Bit[MIN_CAP]){});
    int i = 0;
    for (;;) {
        CCC_Entry const *const e = flat_hash_map_try_insert_with(
            &h, i, &(CCC_Allocator){}, (struct Val){.val = i}
        );
        check(occupied(e), CCC_FALSE);
        struct Val *v = unwrap(e);
        if (!v) {
            break;
        }
        CCC_Result const bit_push = flat_bitset_push_back(
            &is_destroyed, CCC_FALSE, &(CCC_Allocator){}
        );
        check(bit_push, CCC_RESULT_OK);
        check(v->key, i);
        check(v->val, i);
        ++i;
    }
    size_t const full_count = count(&h).count;
    flat_hash_map_clear(
        &h,
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

check_static_begin(flat_hash_map_test_clear_and_free_with_destructor) {
    enum : size_t {
        MIN_CAP = 16
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for(
            (typeof(flat_hash_map_storage_for((struct Val[MIN_CAP]){}))[1]){}
        ),
    };
    CCC_Flat_hash_map h = flat_hash_map_with_capacity(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        allocator,
        MIN_CAP
    );
    Flat_bitset is_destroyed = flat_bitset_with_storage(0, (Bit[MIN_CAP]){});
    int i = 0;
    for (;;) {
        struct Val const *const v = unwrap(flat_hash_map_insert_or_assign_with(
            &h, i, &(CCC_Allocator){}, (struct Val){.val = i}
        ));
        if (!v) {
            break;
        }
        CCC_Result const bit_push = flat_bitset_push_back(
            &is_destroyed, CCC_FALSE, &(CCC_Allocator){}
        );
        check(bit_push, CCC_RESULT_OK);
        check(v->key, i);
        check(v->val, i);
        ++i;
    }
    size_t const full_count = count(&h).count;
    flat_hash_map_clear_and_free(
        &h,
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
        flat_hash_map_test_insert_then_iterate(),
        flat_hash_map_test_insert_delete_then_iterate(),
        flat_hash_map_test_insert_allocate_clear_free(),
        flat_hash_map_test_insert_clear_insert_determinism(),
        flat_hash_map_test_clear_with_destructor(),
        flat_hash_map_test_clear_and_free_with_destructor(),
    );
}
