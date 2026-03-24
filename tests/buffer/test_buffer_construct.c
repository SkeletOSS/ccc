#include <stddef.h>
#include <stdint.h>

#define BUFFER_USING_NAMESPACE_CCC

#include "ccc/buffer.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/allocate.h"
#include "utility/stack_allocator.h"

static Buffer const rgb
    = buffer_with_storage(3, (uint8_t[3]){0xFF, 0x00, 0xFF});

check_static_begin(buffer_test_static_global_const) {
    check(buffer_count(&rgb).count, 3);
    check(buffer_capacity(&rgb).count, 3);
    check(buffer_count_bytes(&rgb).count, 3);
    check(buffer_capacity_bytes(&rgb).count, 3);
    uint8_t const *const i = buffer_at(&rgb, 0);
    check(i != NULL, CCC_TRUE);
    check(*i, 0xFF);
    check_end();
}

check_static_begin(buffer_test_fixed) {
    Buffer b = buffer_with_storage(0, (int[5]){});
    check(buffer_count(&b).count, 0);
    check(buffer_capacity(&b).count, 5);
    int const *const i = buffer_at(&b, 0);
    check(i != NULL, CCC_TRUE);
    check(*i, 0);
    check_end();
}

check_static_begin(buffer_test_manual_counts) {
    Buffer b = buffer_with_storage(0, (int[5]){});
    check(buffer_count(&b).count, 0);
    check(buffer_count_plus(&b, 1), CCC_RESULT_OK);
    check(buffer_count(&b).count, 1);
    check(buffer_count_minus(&b, 1), CCC_RESULT_OK);
    check(buffer_count(&b).count, 0);
    check(buffer_count_minus(&b, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(buffer_count_set(&b, 5), CCC_RESULT_OK);
    check(buffer_count_plus(&b, 1), CCC_RESULT_ARGUMENT_ERROR);
    check(buffer_count_set(&b, 6), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(buffer_test_full) {
    Buffer b = buffer_with_storage(5, (int[5]){0, 1, 2, 3, 4});
    check(buffer_count(&b).count, 5);
    check(buffer_capacity(&b).count, 5);
    int const *const i = buffer_at(&b, 2);
    check(i != NULL, CCC_TRUE);
    check(*i, 2);
    check_end();
}

check_static_begin(buffer_test_reserve) {
    Buffer b = buffer_default(int);
    check(buffer_reserve(&b, 8, &std_allocator), CCC_RESULT_OK);
    check(buffer_count(&b).count, 0);
    check(buffer_capacity(&b).count, 8);
    check_end(buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator););
}

check_static_begin(buffer_test_copy_no_allocate) {
    Buffer source = buffer_with_storage(5, (int[5]){0, 1, 2, 3, 4});
    Buffer destination = buffer_with_storage(0, (int[10]){});
    check(buffer_count(&destination).count, 0);
    check(buffer_capacity(&destination).count, 10);
    CCC_Result const r = buffer_copy(&destination, &source, &(CCC_Allocator){});
    check(r, CCC_RESULT_OK);
    check(buffer_count(&destination).count, 5);
    check(buffer_capacity(&destination).count, 10);
    check_end();
}

check_static_begin(buffer_test_copy_no_allocate_fail) {
    Buffer source = buffer_with_storage(3, (int[3]){0, 1, 2});
    Buffer bad_destination = buffer_with_storage(0, (int[2]){});
    check(buffer_count(&source).count, 3);
    check(buffer_is_empty(&bad_destination), CCC_TRUE);
    CCC_Result res = buffer_copy(&bad_destination, &source, NULL);
    check(res != CCC_RESULT_OK, CCC_TRUE);
    check_end();
}

check_static_begin(buffer_test_copy_allocate) {
    Buffer source = buffer_default(int);
    Buffer destination = buffer_default(int);
    check(buffer_is_empty(&destination), CCC_TRUE);
    enum : size_t {
        PUSHCAP = 5,
    };
    int push[PUSHCAP] = {0, 1, 2, 3, 4};
    for (size_t i = 0; i < PUSHCAP; ++i) {
        check(
            buffer_push_back(&source, &push[i], &std_allocator) != NULL,
            CCC_TRUE
        );
    }
    CCC_Result const res = buffer_copy(&destination, &source, &std_allocator);
    check(res, CCC_RESULT_OK);
    check(*(int *)buffer_begin(&source), 0);
    check(buffer_count(&destination).count, 5);
    while (!buffer_is_empty(&source) && !buffer_is_empty(&destination)) {
        int const a = *(int *)buffer_back(&source);
        int const b = *(int *)buffer_back(&destination);
        (void)buffer_pop_back(&source);
        (void)buffer_pop_back(&destination);
        check(a, b);
    }
    check(buffer_is_empty(&source), buffer_is_empty(&destination));
    check_end({
        (void)buffer_clear_and_free(
            &source, &(CCC_Destructor){}, &std_allocator
        );
        (void)buffer_clear_and_free(
            &destination, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(buffer_test_copy_allocate_fail) {
    Buffer source = buffer_default(int);
    Buffer destination = buffer_default(int);
    check(
        buffer_push_back(&source, &(int){88}, &std_allocator) != NULL, CCC_TRUE
    );
    CCC_Result const res
        = buffer_copy(&destination, &source, &(CCC_Allocator){});
    check(res != CCC_RESULT_OK, CCC_TRUE);
    check_end({
        (void)buffer_clear_and_free(
            &source, &(CCC_Destructor){}, &std_allocator
        );
    });
}

check_static_begin(buffer_test_init_from) {
    CCC_Buffer b
        = CCC_buffer_from(std_allocator, 8, (int[]){1, 2, 3, 4, 5, 6, 7});
    int elem = 1;
    for (int const *i = CCC_buffer_begin(&b); i != CCC_buffer_end(&b);
         i = CCC_buffer_next(&b, i)) {
        check(elem, *i);
        ++elem;
    }
    check(elem, 8);
    check(CCC_buffer_count(&b).count, elem - 1);
    check(CCC_buffer_capacity(&b).count, elem);
    check_end(
        (void)buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator);
    );
}

check_static_begin(buffer_test_init_from_fail) {
    /* Whoops forgot allocation function. */
    CCC_Buffer b
        = CCC_buffer_from((CCC_Allocator){}, 0, (int[]){1, 2, 3, 4, 5, 6, 7});
    int elem = 1;
    for (int const *i = CCC_buffer_begin(&b); i != CCC_buffer_end(&b);
         i = CCC_buffer_next(&b, i)) {
        check(elem, *i);
        ++elem;
    }
    check(elem, 1);
    check(CCC_buffer_count(&b).count, 0);
    check(CCC_buffer_capacity(&b).count, 0);
    check_end(
        (void)buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator);
    );
}

check_static_begin(buffer_test_init_with_capacity) {
    CCC_Buffer b = CCC_buffer_with_capacity(int, std_allocator, 8);
    check(buffer_capacity(&b).count, 8);
    check(buffer_data(&b) != NULL, CCC_TRUE);
    size_t count = 0;
    while (buffer_push_back(&b, &(int){9}, &(CCC_Allocator){})) {
        ++count;
    }
    check(count, 8);
    check_end(
        (void)buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator);
    );
}

check_static_begin(buffer_test_default) {
    CCC_Buffer b = CCC_buffer_default(int);
    check(CCC_buffer_is_empty(&b), CCC_TRUE);
    check_end();
}

check_static_begin(buffer_test_context_with_allocator) {
    CCC_Allocator const stack_allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    CCC_Buffer b = buffer_default(int);
    check(CCC_buffer_is_empty(&b), CCC_TRUE);
    check(CCC_buffer_reserve(&b, 8, &stack_allocator), CCC_RESULT_OK);
    int const *const i = CCC_buffer_push_back(&b, &(int){2}, &stack_allocator);
    check(i != NULL, CCC_TRUE);
    check(*i, 2);
    check(CCC_buffer_count(&b).count, 1);
    check_end(
        CCC_buffer_clear_and_free(&b, &(CCC_Destructor){}, &stack_allocator);
    );
}

check_static_begin(buffer_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    CCC_Buffer b = CCC_buffer_with_capacity(int, (CCC_Allocator){}, 8);
    check(buffer_capacity(&b).count, 0);
    check(buffer_data(&b) == NULL, CCC_TRUE);
    size_t count = 0;
    while (buffer_push_back(&b, &(int){9}, &(CCC_Allocator){})) {
        ++count;
    }
    check(count, 0);
    check_end(
        (void)buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator);
    );
}

int
main(void) {
    return check_run(
        buffer_test_static_global_const(),
        buffer_test_fixed(),
        buffer_test_manual_counts(),
        buffer_test_full(),
        buffer_test_reserve(),
        buffer_test_copy_no_allocate(),
        buffer_test_copy_no_allocate_fail(),
        buffer_test_copy_allocate(),
        buffer_test_copy_allocate_fail(),
        buffer_test_init_from(),
        buffer_test_init_from_fail(),
        buffer_test_init_with_capacity(),
        buffer_test_init_with_capacity_fail(),
        buffer_test_default(),
        buffer_test_context_with_allocator()
    );
}
