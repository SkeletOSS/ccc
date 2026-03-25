#include "ccc/flat_hash_map.h"
#include "ccc/types.h"
#include "checkers.h"
#include "flat_hash_map_utility.h"
#include "utility/allocate.h"
#include "utility/stack_allocator.h"

check_static_begin(flat_hash_map_test_counting_null_input) {
    check(CCC_flat_hash_map_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_flat_hash_map_capacity(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_hash_map_test_status_null_input) {
    check(CCC_flat_hash_map_occupied(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_flat_hash_map_insert_error(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_flat_hash_map_entry_status(NULL), CCC_ENTRY_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_hash_map_test_bool_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    check(CCC_flat_hash_map_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_flat_hash_map_contains(NULL, &(struct Val){}), CCC_TRIBOOL_ERROR);
    check(CCC_flat_hash_map_contains(&m, NULL), CCC_TRIBOOL_ERROR);
    check(CCC_flat_hash_map_contains(&m, &(struct Val){}), CCC_FALSE);
    check_end();
}

check_static_begin(flat_hash_map_test_entry_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Flat_hash_map_entry *fe
        = CCC_flat_hash_map_entry_wrap(NULL, &(struct Val){}, &std_allocator);
    check(CCC_flat_hash_map_entry_status(fe), CCC_RESULT_ARGUMENT_ERROR);
    fe = CCC_flat_hash_map_entry_wrap(&m, NULL, &std_allocator);
    check(CCC_flat_hash_map_entry_status(fe), CCC_RESULT_ARGUMENT_ERROR);
    fe = CCC_flat_hash_map_entry_wrap(&m, &(struct Val){}, NULL);
    check(CCC_flat_hash_map_entry_status(fe), CCC_RESULT_ARGUMENT_ERROR);
    fe = CCC_flat_hash_map_entry_wrap(&m, &(struct Val){}, &(CCC_Allocator){});
    check(
        CCC_flat_hash_map_entry_status(fe), CCC_RESULT_NO_ALLOCATION_FUNCTION
    );
    check(CCC_flat_hash_map_insert_error(fe), CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_or_insert_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Flat_hash_map_entry *fe
        = CCC_flat_hash_map_entry_wrap(NULL, &(struct Val){}, &std_allocator);
    struct Val *v = CCC_flat_hash_map_or_insert(fe, &(struct Val){});
    check(v == NULL, CCC_TRUE);
    fe = CCC_flat_hash_map_entry_wrap(&m, &(struct Val){}, &(CCC_Allocator){});
    v = CCC_flat_hash_map_or_insert(fe, &(struct Val){});
    check(v == NULL, CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_insert_entry_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Flat_hash_map_entry *fe
        = CCC_flat_hash_map_entry_wrap(NULL, &(struct Val){}, &std_allocator);
    struct Val *v = CCC_flat_hash_map_insert_entry(fe, &(struct Val){});
    check(v == NULL, CCC_TRUE);
    fe = CCC_flat_hash_map_entry_wrap(&m, &(struct Val){}, &(CCC_Allocator){});
    v = CCC_flat_hash_map_insert_entry(fe, &(struct Val){});
    check(v == NULL, CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_swap_entry_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Entry *e = CCC_flat_hash_map_swap_entry_wrap(
        NULL, &(struct Val){}, &std_allocator
    );
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_swap_entry_wrap(&m, NULL, &std_allocator);
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_swap_entry_wrap(&m, &(struct Val){}, NULL);
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_swap_entry_wrap(
        &m, &(struct Val){}, &(CCC_Allocator){}
    );
    check(CCC_entry_status(e), CCC_ENTRY_INSERT_ERROR);
    check_end();
}

check_static_begin(flat_hash_map_test_insert_or_assign_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Entry *e = CCC_flat_hash_map_insert_or_assign_wrap(
        NULL, &(struct Val){}, &std_allocator
    );
    check(CCC_entry_status(e), CCC_RESULT_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_insert_or_assign_wrap(&m, NULL, &std_allocator);
    check(CCC_entry_status(e), CCC_RESULT_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_insert_or_assign_wrap(&m, &(struct Val){}, NULL);
    check(CCC_entry_status(e), CCC_RESULT_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_insert_or_assign_wrap(
        &m, &(struct Val){}, &(CCC_Allocator){}
    );
    check(CCC_entry_status(e), CCC_ENTRY_INSERT_ERROR);
    check(CCC_entry_insert_error(e), CCC_TRUE);
    check_end();
}

check_static_begin(flat_hash_map_test_try_insert_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Entry *e = CCC_flat_hash_map_try_insert_wrap(
        NULL, &(struct Val){}, &std_allocator
    );
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_try_insert_wrap(&m, NULL, &std_allocator);
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_try_insert_wrap(&m, &(struct Val){}, NULL);
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_try_insert_wrap(
        &m, &(struct Val){}, &(CCC_Allocator){}
    );
    check(CCC_entry_status(e), CCC_ENTRY_INSERT_ERROR);
    check_end();
}

check_static_begin(flat_hash_map_test_remove_entry_null_input) {
    CCC_Flat_hash_map_entry *fe
        = CCC_flat_hash_map_entry_wrap(NULL, &(struct Val){}, &std_allocator);
    CCC_Entry *e = CCC_flat_hash_map_remove_entry_wrap(NULL);
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_remove_entry_wrap(fe);
    check(CCC_entry_status(e), CCC_ENTRY_VACANT);
    check_end();
}

check_static_begin(flat_hash_map_test_remove_key_value_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Entry *e
        = CCC_flat_hash_map_remove_key_value_wrap(NULL, &(struct Val){});
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_remove_key_value_wrap(&m, NULL);
    check(CCC_entry_status(e), CCC_ENTRY_ARGUMENT_ERROR);
    e = CCC_flat_hash_map_remove_key_value_wrap(&m, &(struct Val){});
    check(CCC_entry_status(e), CCC_ENTRY_VACANT);
    check_end();
}

check_static_begin(flat_hash_map_test_iterator_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    struct Val *v = CCC_flat_hash_map_begin(NULL);
    check(v, NULL);
    v = CCC_flat_hash_map_begin(&m);
    check(v, NULL);
    check(CCC_flat_hash_map_next(&m, v), NULL);
    check(CCC_flat_hash_map_clear(&m, &(CCC_Destructor){}), CCC_RESULT_OK);
    check_end();
}

check_static_begin(flat_hash_map_test_destructor_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    check(
        CCC_flat_hash_map_clear(NULL, &(CCC_Destructor){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(CCC_flat_hash_map_clear(&m, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_flat_hash_map_clear(&m, &(CCC_Destructor){}), CCC_RESULT_OK);
    check_end();
}

check_static_begin(flat_hash_map_test_reserve_null_input) {
    CCC_Flat_hash_map m = CCC_flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[16]){}
    );
    m.mask = 2;
    check(
        CCC_flat_hash_map_reserve(&m, 1, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    m.data = NULL;
    check(
        CCC_flat_hash_map_reserve(&m, 1, &(CCC_Allocator){}),
        CCC_RESULT_ALLOCATOR_ERROR
    );
    check_end();
}

check_static_begin(flat_hash_map_test_copy_null_input) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((char[1]){}),
    };
    CCC_Flat_hash_map m = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    CCC_Flat_hash_map o = CCC_flat_hash_map_default(
        struct Val,
        key,
        (CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }
    );
    check(CCC_flat_hash_map_copy(&m, &o, &allocator), CCC_RESULT_OK);
    o = CCC_flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){
            .hash = flat_hash_map_int_to_u64,
            .compare = flat_hash_map_id_order,
        }),
        (struct Val[16]){}
    );
    (void)CCC_flat_hash_map_insert_or_assign(
        &o, &(struct Val){}, &(CCC_Allocator){}
    );
    check(
        CCC_flat_hash_map_copy(&m, &o, &allocator), CCC_RESULT_ALLOCATOR_ERROR
    );
    check_end();
}

int
main(void) {
    return check_run(
        flat_hash_map_test_counting_null_input(),
        flat_hash_map_test_status_null_input(),
        flat_hash_map_test_bool_null_input(),
        flat_hash_map_test_reserve_null_input(),
        flat_hash_map_test_entry_null_input(),
        flat_hash_map_test_or_insert_null_input(),
        flat_hash_map_test_insert_entry_null_input(),
        flat_hash_map_test_swap_entry_null_input(),
        flat_hash_map_test_insert_or_assign_null_input(),
        flat_hash_map_test_try_insert_null_input(),
        flat_hash_map_test_iterator_null_input(),
        flat_hash_map_test_remove_entry_null_input(),
        flat_hash_map_test_remove_key_value_null_input(),
        flat_hash_map_test_copy_null_input(),
        flat_hash_map_test_destructor_null_input(),
    );
}
