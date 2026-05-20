#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define FLAT_HASH_MAP_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "checkers.h"
#include "flat_hash_map.h"
#include "flat_hash_map_utility.h"
#include "traits.h"
#include "types.h"
#include "utility/random.h"
#include "utility/std_allocator.h"

/** Test constructing a user type that has an alignment constraint with an
allocator that respects it. We know that the flat hash map must align the
metadata array to group size for SIMD performance. So, we want to ensure that
using an aligned allocation for this type*/
struct Aligned_type {
    int i;
    uint64_t d;
};
static_assert(
    alignof(struct Aligned_type) != sizeof(struct Aligned_type),
    "Tested type alignment and size must differ."
);

/** This is a limited test to ensure behavior of aligned allocation works. A
real user allocator would gracefully implement an aligned reallocation function.
We will not. Only use this for single allocation or reserve. No reallocation
is available. Aligned allocate and free are available behaviors. */
static void *
my_aligned_alloc_no_realloc(CCC_Allocator_arguments const arguments) {
    if (!arguments.input && !arguments.bytes) {
        return NULL;
    }
    if (!arguments.input) {
        return aligned_alloc(arguments.alignment, arguments.bytes);
    }
    if (!arguments.bytes) {
        free(arguments.input);
        return NULL;
    }
    return NULL;
}

static CCC_Order
flat_hash_map_aligned_type_order(CCC_Key_comparator_arguments const order) {
    struct Aligned_type const *const right = order.type_right;
    int const left = *((int *)order.key_left);
    return (left > right->i) - (left < right->i);
}

check_static_begin(flat_hash_map_test_erase) {
    CCC_Flat_hash_map fh = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_zero,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[SMALL_FIXED_CAP]){}
    );
    struct Val query = {.key = 137, .val = 99};
    /* Nothing was there before so nothing is in the entry. */
    CCC_Entry ent = swap_entry(&fh, &query, &(CCC_Allocator){});
    check(occupied(&ent), false);
    check(unwrap(&ent) != NULL, true);
    check(count(&fh).count, 1);
    ent = CCC_remove_key_value(&fh, &query);
    check(occupied(&ent), true);
    struct Val *v = unwrap(&ent);
    check(v != NULL, true);
    check(v->key, 137);
    check(v->val, 99);
    check(count(&fh).count, 0);
    query.key = 101;
    ent = CCC_remove_key_value(&fh, &query);
    check(occupied(&ent), false);
    check(count(&fh).count, 0);
    CCC_flat_hash_map_insert_entry_with(
        flat_hash_map_entry_wrap(&fh, &(int){137}, &(CCC_Allocator){}),
        (struct Val){.key = 137, .val = 99}
    );
    check(count(&fh).count, 1);
    check(
        occupied(flat_hash_map_remove_entry_wrap(
            flat_hash_map_entry_wrap(&fh, &(int){137}, &(CCC_Allocator){})
        )),
        true
    );
    check(count(&fh).count, 0);
    check_end();
}

check_static_begin(flat_hash_map_test_shuffle_insert_erase) {
    CCC_Flat_hash_map h = flat_hash_map_default(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        })
    );
    int const to_insert = 100;
    int const larger_prime = 101;
    for (int i = 0, shuffle = larger_prime % to_insert; i < to_insert;
         ++i, shuffle = (shuffle + larger_prime) % to_insert) {
        struct Val *v = unwrap(flat_hash_map_insert_or_assign_with(
            &h,
            shuffle,
            &std_allocator,
            (struct Val){
                .val = i,
            }
        ));
        check(v != NULL, true);
        check(v->key, shuffle);
        check(v->val, i);
        check(validate(&h), true);
    }
    check(count(&h).count, to_insert);
    size_t cur_size = count(&h).count;
    int i = 0;
    while (!is_empty(&h) && cur_size) {
        check(contains(&h, &i), true);
        if (i % 2) {
            struct Val const *const old_val = unwrap(
                flat_hash_map_remove_key_value_wrap(&h, &(struct Val){.key = i})
            );
            check(old_val != NULL, true);
            check(old_val->key, i);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &i, &std_allocator)
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end({
        flat_hash_map_clear_and_free(&h, &(CCC_Destructor){}, &std_allocator);
    });
}

/* This test will force us to test our in place hashing algorithm. */
check_static_begin(flat_hash_map_test_shuffle_erase_fixed) {
    CCC_Flat_hash_map h = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    int to_insert[STANDARD_FIXED_CAP];
    iota(to_insert, STANDARD_FIXED_CAP, 0);
    rand_shuffle(sizeof(int), to_insert, STANDARD_FIXED_CAP, &(int){});
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
    size_t cur_size = count(&h).count;
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        CCC_Entry removed = remove_entry(
            flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
        );
        check(occupied(&removed), true);
        check(validate(&h), true);
    }
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Entry const *const e = flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Val){.val = i}
        );
        check(occupied(e), false);
        check(validate(&h), true);
    }
    check(full_size, cur_size);
    i = 0;
    while (!is_empty(&h) && cur_size) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        if (i % 2) {
            struct Val const *const old_val
                = unwrap(flat_hash_map_remove_key_value_wrap(
                    &h, &(struct Val){.key = cur}
                ));
            check(old_val != NULL, true);
            check(old_val->key, to_insert[i]);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end();
}

check_static_begin(flat_hash_map_test_shuffle_erase_fixed_aligned) {
    CCC_Flat_hash_map h = flat_hash_map_with_storage(
        i,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_aligned_type_order,
        }),
        (struct Aligned_type[STANDARD_FIXED_CAP]){}
    );
    int to_insert[STANDARD_FIXED_CAP];
    iota(to_insert, STANDARD_FIXED_CAP, 0);
    rand_shuffle(sizeof(int), to_insert, STANDARD_FIXED_CAP, &(int){});
    int i = 0;
    for (;;) {
        int const cur = to_insert[i];
        struct Aligned_type const *const v
            = unwrap(flat_hash_map_insert_or_assign_with(
                &h, cur, &(CCC_Allocator){}, (struct Aligned_type){.i = i}
            ));
        if (!v) {
            break;
        }
        check(v->i, to_insert[i]);
        check(validate(&h), true);
        ++i;
    }
    for (struct Aligned_type const *iter = begin(&h); iter != end(&h);
         iter = next(&h, iter)) {
        check(iter != NULL, CCC_TRUE);
        check(iter->i >= 0 && iter->i <= (int)STANDARD_FIXED_CAP, CCC_TRUE);
    }
    size_t const full_size = count(&h).count;
    size_t cur_size = count(&h).count;
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        CCC_Entry removed = remove_entry(
            flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
        );
        check(occupied(&removed), true);
        check(validate(&h), true);
    }
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Entry const *const e = flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Aligned_type){}
        );
        check(occupied(e), false);
        check(validate(&h), true);
    }
    check(full_size, cur_size);
    i = 0;
    while (!is_empty(&h) && cur_size) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        if (i % 2) {
            struct Aligned_type const *const old_val
                = unwrap(flat_hash_map_remove_key_value_wrap(
                    &h, &(struct Aligned_type){.i = cur}
                ));
            check(old_val != NULL, true);
            check(old_val->i, to_insert[i]);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end();
}

/* This test will force us to test our in place hashing algorithm with bad
   collisions */
check_static_begin(flat_hash_map_test_shuffle_erase_fixed_collisions) {
    CCC_Flat_hash_map h = flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_last_digit,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[STANDARD_FIXED_CAP]){}
    );
    int to_insert[STANDARD_FIXED_CAP];
    iota(to_insert, STANDARD_FIXED_CAP, 0);
    rand_shuffle(sizeof(int), to_insert, STANDARD_FIXED_CAP, &(int){});
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
    size_t cur_size = count(&h).count;
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        CCC_Entry removed = remove_entry(
            flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
        );
        check(occupied(&removed), true);
        check(validate(&h), true);
    }
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Entry const *const e = flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Val){.val = i}
        );
        check(occupied(e), false);
        check(validate(&h), true);
    }
    check(full_size, cur_size);
    i = 0;
    while (!is_empty(&h) && cur_size) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        if (i % 2) {
            struct Val const *const old_val
                = unwrap(flat_hash_map_remove_key_value_wrap(
                    &h, &(struct Val){.key = cur}
                ));
            check(old_val != NULL, true);
            check(old_val->key, to_insert[i]);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end();
}

check_static_begin(flat_hash_map_test_shuffle_erase_reserved) {
    /* The map will be given dynamically reserved space but no ability to
       resize. All algorithms should function normally and in place rehashing
       should take effect. */
    CCC_Flat_hash_map h = flat_hash_map_default(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        })
    );
    int const test_amount = 896;
    CCC_Result const res_check
        = CCC_flat_hash_map_reserve(&h, (size_t)test_amount, &std_allocator);
    check(res_check, CCC_RESULT_OK);

    /* Give ourselves plenty more to insert so we don't run out before cap. */
    int to_insert[1024];
    iota(to_insert, 1024, 0);
    rand_shuffle(sizeof(int), to_insert, 1024, &(int){});
    int i = 0;
    for (;;) {
        int const cur = to_insert[i];
        struct Val const *const v = unwrap(flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Val){.val = i}
        ));
        if (!v) {
            break;
        }
        check(v->key, cur);
        check(v->val, i);
        check(validate(&h), true);
        ++i;
    }
    size_t const full_size = count(&h).count;
    size_t cur_size = count(&h).count;
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        CCC_Entry const removed = remove_entry(
            flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
        );
        check(occupied(&removed), true);
        check(validate(&h), true);
    }
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Entry const *const e = flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Val){.val = i}
        );
        check(occupied(e), false);
        check(validate(&h), true);
    }
    check(full_size, cur_size);
    i = 0;
    while (!is_empty(&h) && cur_size) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        if (i % 2) {
            struct Val const *const old_val
                = unwrap(flat_hash_map_remove_key_value_wrap(
                    &h, &(struct Val){.key = cur}
                ));
            check(old_val != NULL, true);
            check(old_val->key, to_insert[i]);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end((void)CCC_flat_hash_map_clear_and_free(
                  &h, &(CCC_Destructor){}, &std_allocator
    ););
}

check_static_begin(flat_hash_map_test_shuffle_erase_reserved_aligned) {
    /* The map will be given dynamically reserved space but no ability to
       resize. All algorithms should function normally and in place rehashing
       should take effect. */
    CCC_Flat_hash_map h = flat_hash_map_default(
        struct Aligned_type,
        i,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_aligned_type_order,
        })
    );
    int const test_amount = 896;
    CCC_Result const res_check = CCC_flat_hash_map_reserve(
        &h,
        (size_t)test_amount,
        &(CCC_Allocator){.allocate = my_aligned_alloc_no_realloc}
    );
    check(res_check, CCC_RESULT_OK);

    /* Give ourselves plenty more to insert so we don't run out before cap. */
    int to_insert[1024];
    iota(to_insert, 1024, 0);
    rand_shuffle(sizeof(int), to_insert, 1024, &(int){});
    int i = 0;
    for (;;) {
        int const cur = to_insert[i];
        struct Aligned_type const *const v
            = unwrap(flat_hash_map_insert_or_assign_with(
                &h, cur, &(CCC_Allocator){}, (struct Aligned_type){.i = i}
            ));
        if (!v) {
            break;
        }
        check(v->i, cur);
        check(validate(&h), true);
        ++i;
    }
    for (struct Aligned_type const *iter = begin(&h); iter != end(&h);
         iter = next(&h, iter)) {
        check(iter != NULL, CCC_TRUE);
        check(iter->i >= 0 && iter->i <= 1024, CCC_TRUE);
    }
    size_t const full_size = count(&h).count;
    size_t cur_size = count(&h).count;
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        CCC_Entry const removed = remove_entry(
            flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
        );
        check(occupied(&removed), true);
        check(validate(&h), true);
    }
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Entry const *const e = flat_hash_map_insert_or_assign_with(
            &h, cur, &(CCC_Allocator){}, (struct Aligned_type){.i = cur}
        );
        check(occupied(e), false);
        check(validate(&h), true);
    }
    check(full_size, cur_size);
    i = 0;
    while (!is_empty(&h) && cur_size) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        if (i % 2) {
            struct Aligned_type const *const old_val
                = unwrap(flat_hash_map_remove_key_value_wrap(
                    &h, &(struct Aligned_type){.i = cur}
                ));
            check(old_val != NULL, true);
            check(old_val->i, to_insert[i]);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &cur, &(CCC_Allocator){})
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end((void)CCC_flat_hash_map_clear_and_free(
                  &h,
                  &(CCC_Destructor){},
                  &(CCC_Allocator){.allocate = my_aligned_alloc_no_realloc}
    ););
}

check_static_begin(flat_hash_map_test_shuffle_erase_dynamic) {
    CCC_Flat_hash_map h = flat_hash_map_default(
        struct Val,
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        })
    );
    int to_insert[1024];
    iota(to_insert, 1024, 0);
    rand_shuffle(sizeof(int), to_insert, 1024, &(int){});
    int inserted = 0;
    for (; inserted < 1024; ++inserted) {
        int const cur = to_insert[inserted];
        struct Val const *const v = unwrap(flat_hash_map_insert_or_assign_with(
            &h, cur, &std_allocator, (struct Val){.val = inserted}
        ));
        check(v->key, to_insert[inserted]);
        check(v->val, inserted);
        check(validate(&h), true);
    }
    size_t const full_size = count(&h).count;
    size_t cur_size = count(&h).count;
    int i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        CCC_Entry removed
            = remove_entry(flat_hash_map_entry_wrap(&h, &cur, &std_allocator));
        check(occupied(&removed), true);
        check(validate(&h), true);
    }
    i = 0;
    for (; i < (int)(full_size / 2); ++i) {
        int const cur = to_insert[i];
        CCC_Entry const *const e = flat_hash_map_insert_or_assign_with(
            &h, cur, &std_allocator, (struct Val){.val = i}
        );
        check(occupied(e), false);
        check(validate(&h), true);
    }
    check(full_size, cur_size);
    i = 0;
    while (!is_empty(&h) && cur_size) {
        int const cur = to_insert[i];
        CCC_Tribool const check = contains(&h, &cur);
        check(check, true);
        if (i % 2) {
            struct Val const *const old_val
                = unwrap(flat_hash_map_remove_key_value_wrap(
                    &h, &(struct Val){.key = cur}
                ));
            check(old_val != NULL, true);
            check(old_val->key, to_insert[i]);
        } else {
            CCC_Entry removed = remove_entry(
                flat_hash_map_entry_wrap(&h, &cur, &std_allocator)
            );
            check(occupied(&removed), true);
        }
        --cur_size;
        ++i;
        check(count(&h).count, cur_size);
        check(validate(&h), true);
    }
    check(count(&h).count, 0);
    check_end(
        flat_hash_map_clear_and_free(&h, &(CCC_Destructor){}, &std_allocator);
    );
}

int
main(void) {
    random_seed((unsigned)time(NULL)); /* NOLINT */
    return check_run(
        flat_hash_map_test_erase(),
        flat_hash_map_test_shuffle_insert_erase(),
        flat_hash_map_test_shuffle_erase_fixed(),
        flat_hash_map_test_shuffle_erase_fixed_aligned(),
        flat_hash_map_test_shuffle_erase_fixed_collisions(),
        flat_hash_map_test_shuffle_erase_reserved(),
        flat_hash_map_test_shuffle_erase_reserved_aligned(),
        flat_hash_map_test_shuffle_erase_dynamic(),
    );
}
