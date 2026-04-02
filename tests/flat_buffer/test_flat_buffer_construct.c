#include <stddef.h>
#include <stdint.h>

#define FLAT_BUFFER_USING_NAMESPACE_CCC

#include "ccc/flat_buffer.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/allocate.h"
#include "utility/stack_allocator.h"

static Flat_buffer const rgb
    = flat_buffer_with_storage(3, (uint8_t[3]){0xFF, 0x00, 0xFF});

check_static_begin(flat_buffer_test_static_global_const) {
    check(flat_buffer_count(&rgb).count, 3);
    check(flat_buffer_capacity(&rgb).count, 3);
    check(flat_buffer_count_bytes(&rgb).count, 3);
    check(flat_buffer_capacity_bytes(&rgb).count, 3);
    uint8_t const *const i = flat_buffer_at(&rgb, 0);
    check(i != NULL, CCC_TRUE);
    check(*i, 0xFF);
    check_end();
}

check_static_begin(flat_buffer_test_fixed) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[5]){});
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_capacity(&b).count, 5);
    int const *const i = flat_buffer_at(&b, 0);
    check(i != NULL, CCC_TRUE);
    check(*i, 0);
    check_end();
}

check_static_begin(flat_buffer_test_manual_counts) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[5]){});
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_count_plus(&b, 1), CCC_RESULT_OK);
    check(flat_buffer_count(&b).count, 1);
    check(flat_buffer_count_minus(&b, 1), CCC_RESULT_OK);
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_count_minus(&b, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_count_set(&b, 5), CCC_RESULT_OK);
    check(flat_buffer_count_plus(&b, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_buffer_count_set(&b, 6), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_full) {
    Flat_buffer b = flat_buffer_with_storage(5, (int[5]){0, 1, 2, 3, 4});
    check(flat_buffer_count(&b).count, 5);
    check(flat_buffer_capacity(&b).count, 5);
    int const *const i = flat_buffer_at(&b, 2);
    check(i != NULL, CCC_TRUE);
    check(*i, 2);
    check_end();
}

check_static_begin(flat_buffer_test_reserve) {
    Flat_buffer b = flat_buffer_default(int);
    check(flat_buffer_reserve(&b, 8, &std_allocator), CCC_RESULT_OK);
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_capacity(&b).count, 8);
    check_end(
        flat_buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator);
    );
}

check_static_begin(flat_buffer_test_copy_no_allocate) {
    Flat_buffer source = flat_buffer_with_storage(5, (int[5]){0, 1, 2, 3, 4});
    Flat_buffer destination = flat_buffer_with_storage(0, (int[10]){});
    check(flat_buffer_count(&destination).count, 0);
    check(flat_buffer_capacity(&destination).count, 10);
    CCC_Result const r
        = flat_buffer_copy(&destination, &source, &(CCC_Allocator){});
    check(r, CCC_RESULT_OK);
    check(flat_buffer_count(&destination).count, 5);
    check(flat_buffer_capacity(&destination).count, 10);
    check_end();
}

check_static_begin(flat_buffer_test_copy_no_allocate_fail) {
    Flat_buffer source = flat_buffer_with_storage(3, (int[3]){0, 1, 2});
    Flat_buffer bad_destination = flat_buffer_with_storage(0, (int[2]){});
    check(flat_buffer_count(&source).count, 3);
    check(flat_buffer_is_empty(&bad_destination), CCC_TRUE);
    CCC_Result res = flat_buffer_copy(&bad_destination, &source, NULL);
    check(res != CCC_RESULT_OK, CCC_TRUE);
    check_end();
}

check_static_begin(flat_buffer_test_copy_allocate) {
    Flat_buffer source = flat_buffer_default(int);
    Flat_buffer destination = flat_buffer_default(int);
    check(flat_buffer_is_empty(&destination), CCC_TRUE);
    enum : size_t {
        PUSHCAP = 5,
    };
    int push[PUSHCAP] = {0, 1, 2, 3, 4};
    for (size_t i = 0; i < PUSHCAP; ++i) {
        check(
            flat_buffer_push_back(&source, &push[i], &std_allocator) != NULL,
            CCC_TRUE
        );
    }
    CCC_Result const res
        = flat_buffer_copy(&destination, &source, &std_allocator);
    check(res, CCC_RESULT_OK);
    check(*(int *)flat_buffer_begin(&source), 0);
    check(flat_buffer_count(&destination).count, 5);
    while (!flat_buffer_is_empty(&source)
           && !flat_buffer_is_empty(&destination)) {
        int const a = *(int *)flat_buffer_back(&source);
        int const b = *(int *)flat_buffer_back(&destination);
        (void)flat_buffer_pop_back(&source);
        (void)flat_buffer_pop_back(&destination);
        check(a, b);
    }
    check(flat_buffer_is_empty(&source), flat_buffer_is_empty(&destination));
    check_end({
        (void)flat_buffer_clear_and_free(
            &source, &(CCC_Destructor){}, &std_allocator
        );
        (void)flat_buffer_clear_and_free(
            &destination, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(flat_buffer_test_copy_allocate_fail) {
    Flat_buffer source = flat_buffer_default(int);
    Flat_buffer destination = flat_buffer_default(int);
    check(
        flat_buffer_push_back(&source, &(int){88}, &std_allocator) != NULL,
        CCC_TRUE
    );
    CCC_Result const res
        = flat_buffer_copy(&destination, &source, &(CCC_Allocator){});
    check(res != CCC_RESULT_OK, CCC_TRUE);
    check_end({
        (void)flat_buffer_clear_and_free(
            &source, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(flat_buffer_test_init_from) {
    CCC_Flat_buffer b
        = CCC_flat_buffer_from(std_allocator, 8, (int[]){1, 2, 3, 4, 5, 6, 7});
    int elem = 1;
    for (int const *i = CCC_flat_buffer_begin(&b); i != CCC_flat_buffer_end(&b);
         i = CCC_flat_buffer_next(&b, i)) {
        check(elem, *i);
        ++elem;
    }
    check(elem, 8);
    check(CCC_flat_buffer_count(&b).count, elem - 1);
    check(CCC_flat_buffer_capacity(&b).count, elem);
    check_end((void)flat_buffer_clear_and_free(
                  &b, &(CCC_Destructor){}, &std_allocator
    ););
}

check_static_begin(flat_buffer_test_init_from_fail) {
    /* Whoops forgot allocation function. */
    CCC_Flat_buffer b = CCC_flat_buffer_from(
        (CCC_Allocator){}, 0, (int[]){1, 2, 3, 4, 5, 6, 7}
    );
    int elem = 1;
    for (int const *i = CCC_flat_buffer_begin(&b); i != CCC_flat_buffer_end(&b);
         i = CCC_flat_buffer_next(&b, i)) {
        check(elem, *i);
        ++elem;
    }
    check(elem, 1);
    check(CCC_flat_buffer_count(&b).count, 0);
    check(CCC_flat_buffer_capacity(&b).count, 0);
    check_end((void)flat_buffer_clear_and_free(
                  &b, &(CCC_Destructor){}, &std_allocator
    ););
}

check_static_begin(flat_buffer_test_init_with_capacity) {
    CCC_Flat_buffer b = CCC_flat_buffer_with_capacity(int, std_allocator, 8);
    check(flat_buffer_capacity(&b).count, 8);
    check(flat_buffer_data(&b) != NULL, CCC_TRUE);
    size_t count = 0;
    while (flat_buffer_push_back(&b, &(int){9}, &(CCC_Allocator){})) {
        ++count;
    }
    check(count, 8);
    check_end((void)flat_buffer_clear_and_free(
                  &b, &(CCC_Destructor){}, &std_allocator
    ););
}

check_static_begin(flat_buffer_test_default) {
    CCC_Flat_buffer b = CCC_flat_buffer_default(int);
    check(CCC_flat_buffer_is_empty(&b), CCC_TRUE);
    check_end();
}

check_static_begin(flat_buffer_test_context_with_allocator) {
    CCC_Allocator const stack_allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    CCC_Flat_buffer b = flat_buffer_default(int);
    check(CCC_flat_buffer_is_empty(&b), CCC_TRUE);
    check(CCC_flat_buffer_reserve(&b, 8, &stack_allocator), CCC_RESULT_OK);
    int const *const i
        = CCC_flat_buffer_push_back(&b, &(int){2}, &stack_allocator);
    check(i != NULL, CCC_TRUE);
    check(*i, 2);
    check(CCC_flat_buffer_count(&b).count, 1);
    check_end(CCC_flat_buffer_clear_and_free(
                  &b, &(CCC_Destructor){}, &stack_allocator
    ););
}

check_static_begin(flat_buffer_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    CCC_Flat_buffer b
        = CCC_flat_buffer_with_capacity(int, (CCC_Allocator){}, 8);
    check(flat_buffer_capacity(&b).count, 0);
    check(flat_buffer_data(&b) == NULL, CCC_TRUE);
    size_t count = 0;
    while (flat_buffer_push_back(&b, &(int){9}, &(CCC_Allocator){})) {
        ++count;
    }
    check(count, 0);
    check_end((void)flat_buffer_clear_and_free(
                  &b, &(CCC_Destructor){}, &std_allocator
    ););
}

int
main(void) {
    return check_run(
        flat_buffer_test_static_global_const(),
        flat_buffer_test_fixed(),
        flat_buffer_test_manual_counts(),
        flat_buffer_test_full(),
        flat_buffer_test_reserve(),
        flat_buffer_test_copy_no_allocate(),
        flat_buffer_test_copy_no_allocate_fail(),
        flat_buffer_test_copy_allocate(),
        flat_buffer_test_copy_allocate_fail(),
        flat_buffer_test_init_from(),
        flat_buffer_test_init_from_fail(),
        flat_buffer_test_init_with_capacity(),
        flat_buffer_test_init_with_capacity_fail(),
        flat_buffer_test_default(),
        flat_buffer_test_context_with_allocator()
    );
}
