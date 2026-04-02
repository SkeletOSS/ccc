#define FLAT_BUFFER_USING_NAMESPACE_CCC
#include "ccc/flat_buffer.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/allocate.h"
#include "utility/stack_allocator.h"

check_static_begin(flat_buffer_test_reserve_null_input) {
    Flat_buffer b = flat_buffer_default(int);
    check(
        flat_buffer_reserve(NULL, 1, &std_allocator), CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        flat_buffer_reserve(&b, 0, &std_allocator), CCC_RESULT_ARGUMENT_ERROR
    );
    check(flat_buffer_reserve(&b, 1, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_copy_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    Flat_buffer b = flat_buffer_default(int);
    check(
        flat_buffer_copy(NULL, &b, &std_allocator), CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        flat_buffer_copy(&a, NULL, &std_allocator), CCC_RESULT_ARGUMENT_ERROR
    );
    check(flat_buffer_copy(&a, &b, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_copy_exhaustion) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    Flat_buffer a = flat_buffer_default(int);
    Flat_buffer b = flat_buffer_default(int);
    check(flat_buffer_copy(&a, &b, &allocator), CCC_RESULT_OK);
    check(flat_buffer_reserve(&b, 8, &allocator), CCC_RESULT_OK);
    check(flat_buffer_copy(&a, &b, &allocator), CCC_RESULT_ALLOCATOR_ERROR);
    a = flat_buffer_with_storage(0, (int[8]){});
    b.data = NULL;
    check(flat_buffer_copy(&a, &b, &allocator), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_allocate_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(
        flat_buffer_allocate(NULL, 0, &std_allocator), CCC_RESULT_ARGUMENT_ERROR
    );
    check(flat_buffer_allocate(&a, 0, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_allocate_back_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_allocate_back(NULL, &std_allocator), NULL);
    check(flat_buffer_allocate_back(&a, NULL), NULL);
    check_end();
}

check_static_begin(flat_buffer_test_allocator_exhaustion) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_allocate(&a, 10, &allocator), CCC_RESULT_ALLOCATOR_ERROR);
    check(flat_buffer_reserve(&a, 10, &allocator), CCC_RESULT_ALLOCATOR_ERROR);
    check(flat_buffer_reserve(&a, 8, &allocator), CCC_RESULT_OK);
    check(flat_buffer_reserve(&a, 4, &allocator), CCC_RESULT_OK);
    check_end();
}

check_static_begin(flat_buffer_test_push_back_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_push_back(NULL, &(int){}, &std_allocator), NULL);
    check(flat_buffer_push_back(&a, NULL, &std_allocator), NULL);
    check(flat_buffer_push_back(&a, &(int){}, NULL), NULL);
    check_end();
}

check_static_begin(flat_buffer_test_insert_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_insert(NULL, 0, &(int){}, &std_allocator), NULL);
    check(flat_buffer_insert(&a, 0, NULL, &std_allocator), NULL);
    check(flat_buffer_insert(&a, 0, &(int){}, NULL), NULL);
    check(flat_buffer_insert(&a, 0, &(int){}, &std_allocator), NULL);
    check_end();
}

check_static_begin(flat_buffer_test_pop_back_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_pop_back(NULL), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_pop_back(&a), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_pop_back_n_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_pop_back_n(NULL, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_pop_back_n(&a, 1), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_erase_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_erase(NULL, 0), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_erase(&a, 1), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_at_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_at(NULL, 0), NULL);
    check(flat_buffer_at(&a, 1), NULL);
    check_end();
}

check_static_begin(flat_buffer_test_index_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_index(NULL, NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_index(&a, NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_counting_null_input) {
    check(flat_buffer_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_capacity(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_sizeof_type(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_count_bytes(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_capacity_bytes(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_count_plus(NULL, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_count_minus(NULL, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_count_set(NULL, 1), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_clear_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(
        flat_buffer_clear(NULL, &(CCC_Destructor){}), CCC_RESULT_ARGUMENT_ERROR
    );
    check(flat_buffer_clear(&a, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_bools_null_input) {
    Flat_buffer a = flat_buffer_default(int);
    check(flat_buffer_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(flat_buffer_is_empty(&a), CCC_TRUE);
    check(flat_buffer_is_full(NULL), CCC_TRIBOOL_ERROR);
    check(flat_buffer_is_full(&a), CCC_FALSE);
    check_end();
}

check_static_begin(flat_buffer_test_iterators_null_input) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    check(flat_buffer_reverse_begin(NULL), NULL);
    check(flat_buffer_next(&b, NULL), NULL);
    check(flat_buffer_reverse_next(&b, NULL), NULL);
    check(
        flat_buffer_reverse_next(&b, (void *)0x1), flat_buffer_reverse_end(&b)
    );
    check_end();
}

int
main(void) {
    return check_run(
        flat_buffer_test_reserve_null_input(),
        flat_buffer_test_allocator_exhaustion(),
        flat_buffer_test_copy_null_input(),
        flat_buffer_test_copy_exhaustion(),
        flat_buffer_test_allocate_null_input(),
        flat_buffer_test_allocate_back_null_input(),
        flat_buffer_test_push_back_null_input(),
        flat_buffer_test_insert_null_input(),
        flat_buffer_test_pop_back_null_input(),
        flat_buffer_test_pop_back_n_null_input(),
        flat_buffer_test_erase_null_input(),
        flat_buffer_test_at_null_input(),
        flat_buffer_test_index_null_input(),
        flat_buffer_test_counting_null_input(),
        flat_buffer_test_clear_null_input(),
        flat_buffer_test_bools_null_input(),
        flat_buffer_test_iterators_null_input(),
    );
}
