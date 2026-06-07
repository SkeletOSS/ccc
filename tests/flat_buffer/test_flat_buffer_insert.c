#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FLAT_BUFFER_USING_NAMESPACE_CCC

#include "ccc/flat_buffer.h"
#include "ccc/types.h"
#include "checkers.h"
#include "flat_buffer_utility.h"
#include "utility/random.h"
#include "utility/std_allocator.h"

static int
std_order_ints(void const *const left, void const *const right) {
    int const left_int = *(int *)left;
    int const right_int = *(int *)right;
    return (left_int > right_int) - (left_int < right_int);
}

static CCC_Order
ccc_order_ints(CCC_Comparator_arguments const order) {
    int const left_int = *(int *)order.type_left;
    int const right_int = *(int *)order.type_right;
    return (left_int > right_int) - (left_int < right_int);
}

check_static_begin(flat_buffer_test_push_fixed) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    int const push[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    for (size_t i = 0; i < sizeof(push) / sizeof(*push); ++i) {
        int *p = flat_buffer_push_back(&b, &push[i], &(CCC_Allocator){});
        check(p != NULL, CCC_TRUE);
        check(*p, push[i]);
    }
    check(flat_buffer_count(&b).count, sizeof(push) / sizeof(*push));
    check(
        flat_buffer_push_back(&b, &(int){99}, &(CCC_Allocator){}) == NULL,
        CCC_TRUE
    );
    check_end();
}

check_static_begin(flat_buffer_test_push_resize) {
    Flat_buffer b = flat_buffer_default(int);
    size_t const cap = 32;
    int *const many = malloc(sizeof(int) * cap);
    iota(many, cap, 0);
    check(many != NULL, CCC_TRUE);
    for (size_t i = 0; i < cap; ++i) {
        int *p = flat_buffer_push_back(&b, &many[i], &std_allocator);
        check(p != NULL, CCC_TRUE);
        check(*p, many[i]);
    }
    check(flat_buffer_count(&b).count, cap);
    check(flat_buffer_capacity(&b).count >= cap, CCC_TRUE);
    check_end({
        (void)flat_buffer_clear_and_free(
            &b, &(CCC_Destructor){}, &std_allocator
        );
        free(many);
    });
}

check_static_begin(flat_buffer_test_push_qsort) {
    enum : size_t {
        BUF_SORT_CAP = 32,
    };
    Flat_buffer b
        = flat_buffer_with_storage(BUF_SORT_CAP, (int[BUF_SORT_CAP]){});
    int ref[BUF_SORT_CAP] = {};
    iota(ref, BUF_SORT_CAP, 0);
    iota(flat_buffer_begin(&b), BUF_SORT_CAP, 0);
    check(
        memcmp(ref, flat_buffer_begin(&b), BUF_SORT_CAP * sizeof(*ref)),
        CCC_ORDER_EQUAL
    );
    rand_shuffle(sizeof(*ref), ref, BUF_SORT_CAP, &(int){});
    rand_shuffle(
        flat_buffer_sizeof_type(&b).count,
        flat_buffer_begin(&b),
        flat_buffer_count(&b).count,
        &(int){}
    );
    qsort(ref, BUF_SORT_CAP, sizeof(*ref), std_order_ints);
    qsort(
        flat_buffer_begin(&b),
        flat_buffer_capacity(&b).count,
        flat_buffer_sizeof_type(&b).count,
        std_order_ints
    );
    check(
        memcmp(ref, flat_buffer_begin(&b), BUF_SORT_CAP * sizeof(*ref)),
        CCC_ORDER_EQUAL
    );
    int prev = INT_MIN;
    size_t count = 0;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        check(*i >= prev, CCC_TRUE);
        prev = *i;
        ++count;
    }
    check(count, BUF_SORT_CAP);
    check_end();
}

check_static_begin(flat_buffer_test_push_sort) {
    enum : size_t {
        BUF_SORT_CAP = 32,
    };
    Flat_buffer b
        = flat_buffer_with_storage(BUF_SORT_CAP, (int[BUF_SORT_CAP]){});
    iota(flat_buffer_begin(&b), BUF_SORT_CAP, 0);
    rand_shuffle(
        flat_buffer_sizeof_type(&b).count,
        flat_buffer_begin(&b),
        flat_buffer_count(&b).count,
        &(int){}
    );
    (void)quicksort(
        &b,
        &(int){},
        CCC_ORDER_LESSER,
        &(CCC_Comparator){.compare = ccc_order_ints}
    );
    int prev = INT_MIN;
    size_t count = 0;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        check(*i >= prev, CCC_TRUE);
        prev = *i;
        ++count;
    }
    check(count, BUF_SORT_CAP);
    check_end();
}

check_static_begin(flat_buffer_test_insert_no_allocate) {
    enum : size_t {
        BUFINSCAP = 8,
    };
    Flat_buffer b
        = flat_buffer_with_storage(BUFINSCAP - 3, (int[BUFINSCAP]){1, 2, 4, 5});
    check(flat_buffer_count(&b).count, BUFINSCAP - 3);
    int const *const three
        = flat_buffer_insert(&b, 2, &(int){3}, &(CCC_Allocator){});
    check(three != NULL, CCC_TRUE);
    check(*three, 3);
    CCC_Order order
        = buforder(&b, BUFINSCAP - 2, (int[BUFINSCAP - 2]){1, 2, 3, 4, 5});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, BUFINSCAP - 2);
    int const *const zero
        = flat_buffer_insert(&b, 0, &(int){}, &(CCC_Allocator){});
    check(zero != NULL, CCC_TRUE);
    check(*zero, 0);
    order = buforder(&b, BUFINSCAP - 1, (int[BUFINSCAP - 1]){0, 1, 2, 3, 4, 5});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, BUFINSCAP - 1);
    int const *const six
        = flat_buffer_insert(&b, 6, &(int){6}, &(CCC_Allocator){});
    check(six != NULL, CCC_TRUE);
    check(*six, 6);
    order = buforder(&b, BUFINSCAP, (int[BUFINSCAP]){0, 1, 2, 3, 4, 5, 6});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, BUFINSCAP);
    check_end();
}

check_static_begin(flat_buffer_test_insert_no_allocate_fail) {
    enum : size_t {
        BUFINSCAP = 8,
    };
    Flat_buffer b = flat_buffer_with_storage(
        BUFINSCAP, (int[BUFINSCAP]){0, 1, 2, 3, 4, 5, 6}
    );
    check(flat_buffer_count(&b).count, BUFINSCAP);
    int const *const three
        = flat_buffer_insert(&b, 3, &(int){3}, &(CCC_Allocator){});
    check(three == NULL, CCC_TRUE);
    check(flat_buffer_count(&b).count, BUFINSCAP);
    check_end();
}

/* Force a resize when inserting in middle forces shuffle down. */
check_static_begin(flat_buffer_test_insert_allocate) {
    Flat_buffer b = flat_buffer_default(int);
    CCC_Result r = flat_buffer_reserve(&b, 6, &std_allocator);
    check(r, CCC_RESULT_OK);
    r = append_range(&b, 6, (int[6]){1, 2, 4, 5, 6, 7}, &std_allocator);
    check(r, CCC_RESULT_OK);
    check(flat_buffer_count(&b).count, 6);
    int const *const three
        = flat_buffer_insert(&b, 2, &(int){3}, &std_allocator);
    check(three != NULL, CCC_TRUE);
    check(*three, 3);
    CCC_Order order = buforder(&b, 7, (int[7]){1, 2, 3, 4, 5, 6, 7});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, 7);
    int const *const zero = flat_buffer_insert(&b, 0, &(int){}, &std_allocator);
    check(zero != NULL, CCC_TRUE);
    check(*zero, 0);
    order = buforder(&b, 8, (int[8]){0, 1, 2, 3, 4, 5, 6, 7});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, 8);
    int const *const eight
        = flat_buffer_insert(&b, 8, &(int){8}, &std_allocator);
    check(eight != NULL, CCC_TRUE);
    check(*eight, 8);
    order = buforder(&b, 9, (int[9]){0, 1, 2, 3, 4, 5, 6, 7, 8});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, 9);
    check_end(
        flat_buffer_clear_and_free(&b, &(CCC_Destructor){}, &std_allocator);
    );
}

check_static_begin(flat_buffer_test_write) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    check(flat_buffer_write(&b, 0, NULL), CCC_RESULT_ARGUMENT_ERROR);
    CCC_Result write = flat_buffer_write(&b, 0, &(int){0});
    check(write, CCC_RESULT_OK);
    write = flat_buffer_write(&b, 0, flat_buffer_at(&b, 0));
    check(write, CCC_RESULT_ARGUMENT_ERROR);
    write = flat_buffer_write(&b, 7, &(int){8});
    check(write, CCC_RESULT_OK);
    int *i = flat_buffer_at(&b, 7);
    check(i != NULL, CCC_TRUE);
    check(*i, 8);
    write = flat_buffer_write(&b, 7, flat_buffer_at(&b, 0));
    check(write, CCC_RESULT_OK);
    check(i != NULL, CCC_TRUE);
    check(*i, 0);
    write = flat_buffer_write(&b, 8, flat_buffer_at(&b, 0));
    check(write, CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_swap) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    CCC_Result write = flat_buffer_write(&b, 0, &(int){0});
    check(write, CCC_RESULT_OK);
    write = flat_buffer_write(&b, 7, &(int){8});
    check(write, CCC_RESULT_OK);
    int const *x = flat_buffer_at(&b, 7);
    check(x != NULL, CCC_TRUE);
    check(*x, 8);
    check(flat_buffer_swap(&b, &(int){}, 0, 7), CCC_RESULT_OK);
    int const *y = flat_buffer_at(&b, 0);
    check(y != NULL, CCC_TRUE);
    check(*y, 8);
    check(*x, 0);
    check(flat_buffer_swap(&b, &(int){}, 0, 0), CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

check_static_begin(flat_buffer_test_move) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    CCC_Result result = flat_buffer_write(&b, 0, &(int){0});
    check(result, CCC_RESULT_OK);
    result = flat_buffer_write(&b, 7, &(int){8});
    check(result, CCC_RESULT_OK);
    int *i = flat_buffer_at(&b, 7);
    check(i != NULL, CCC_TRUE);
    check(*i, 8);
    i = flat_buffer_at(&b, 0);
    check(i != NULL, CCC_TRUE);
    check(*i, 0);
    i = flat_buffer_move(&b, 0, 7);
    check(i != NULL, CCC_TRUE);
    check(flat_buffer_index(&b, i).count, 0);
    check(*i, 8);
    i = flat_buffer_move(&b, 0, 0);
    check(flat_buffer_index(&b, i).count, 0);
    check(*i, 8);
    i = flat_buffer_move(&b, 0, 8);
    check(i == NULL, CCC_TRUE);
    check(flat_buffer_index(&b, i + 1).error, CCC_RESULT_ARGUMENT_ERROR);
    check_end();
}

int
main(void) {
    return check_run(
        flat_buffer_test_push_fixed(),
        flat_buffer_test_push_resize(),
        flat_buffer_test_push_qsort(),
        flat_buffer_test_push_sort(),
        flat_buffer_test_insert_no_allocate(),
        flat_buffer_test_insert_no_allocate_fail(),
        flat_buffer_test_insert_allocate(),
        flat_buffer_test_write(),
        flat_buffer_test_swap(),
        flat_buffer_test_move(),
    );
}
