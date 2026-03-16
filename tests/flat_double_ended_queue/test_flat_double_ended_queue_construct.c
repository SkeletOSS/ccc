#include <stddef.h>

#define BUFFER_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC
#define FLAT_DOUBLE_ENDED_QUEUE_USING_NAMESPACE_CCC

#include "checkers.h"
#include "flat_double_ended_queue.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

check_static_begin(flat_double_ended_queue_test_construct) {
    int vals[2];
    Flat_double_ended_queue q
        = flat_double_ended_queue_for(int, sizeof(vals) / sizeof(int), 0, vals);
    check(is_empty(&q), true);
    check_end();
}

check_static_begin(flat_double_ended_queue_test_construct_with_storage) {
    Flat_double_ended_queue q1
        = flat_double_ended_queue_with_storage(0, (int[4]){});
    Flat_double_ended_queue q2
        = flat_double_ended_queue_with_storage(4, (int[4]){});
    check(is_empty(&q1), true);
    check(is_empty(&q2), false);
    check_end();
}

check_static_begin(flat_double_ended_queue_test_copy_no_allocate) {
    Flat_double_ended_queue q1
        = flat_double_ended_queue_with_storage(3, (int[3]){0, 1, 2});
    Flat_double_ended_queue q2
        = CCC_flat_double_ended_queue_with_storage(0, ((int[5]){}));
    check(count(&q1).count, 3);
    check(*(int *)front(&q1), 0);
    check(is_empty(&q2), true);
    CCC_Result const res
        = flat_double_ended_queue_copy(&q2, &q1, &(CCC_Allocator_context){});
    check(res, CCC_RESULT_OK);
    check(count(&q2).count, 3);
    while (!is_empty(&q1) && !is_empty(&q2)) {
        int f1 = *(int *)front(&q1);
        int f2 = *(int *)front(&q2);
        (void)pop_front(&q1);
        (void)pop_front(&q2);
        check(f1, f2);
    }
    check(is_empty(&q1), is_empty(&q2));
    check_end();
}

check_static_begin(flat_double_ended_queue_test_copy_no_allocate_fail) {
    Flat_double_ended_queue q1
        = flat_double_ended_queue_with_storage(3, (int[3]){0, 1, 2});
    Flat_double_ended_queue q2
        = CCC_flat_double_ended_queue_with_storage(0, (int[2]){});
    check(count(&q1).count, 3);
    check(*(int *)front(&q1), 0);
    check(is_empty(&q2), true);
    CCC_Result const res
        = flat_double_ended_queue_copy(&q2, &q1, &(CCC_Allocator_context){});
    check(res != CCC_RESULT_OK, true);
    check_end();
}

check_static_begin(flat_double_ended_queue_test_copy_allocate) {
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[16]){}),
    };
    Flat_double_ended_queue q1
        = flat_double_ended_queue_with_capacity(int, &allocator, 8);
    Flat_double_ended_queue q2 = CCC_flat_double_ended_queue_default(int);
    CCC_Result res = flat_double_ended_queue_push_back_range(
        &q1, &buffer_with_storage(5, (int[5]){0, 1, 2, 3, 4}), &allocator);
    check(res, CCC_RESULT_OK);
    check(*(int *)front(&q1), 0);
    check(is_empty(&q2), true);
    res = flat_double_ended_queue_copy(&q2, &q1, &allocator);
    check(res, CCC_RESULT_OK);
    check(count(&q2).count, 5);
    while (!is_empty(&q1) && !is_empty(&q2)) {
        int const f1 = *(int *)front(&q1);
        int const f2 = *(int *)front(&q2);
        (void)pop_front(&q1);
        (void)pop_front(&q2);
        check(f1, f2);
    }
    check(is_empty(&q1), is_empty(&q2));
    check_end({
        (void)flat_double_ended_queue_clear_and_free(
            &q1, &(CCC_Destructor_context){}, &allocator);
        (void)flat_double_ended_queue_clear_and_free(
            &q2, &(CCC_Destructor_context){}, &allocator);
    });
}

check_static_begin(flat_double_ended_queue_test_copy_allocate_fail) {
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[16]){}),
    };
    Flat_double_ended_queue q1
        = flat_double_ended_queue_with_capacity(int, &allocator, 8);
    Flat_double_ended_queue q2 = CCC_flat_double_ended_queue_default(int);
    CCC_Result res = flat_double_ended_queue_push_back_range(
        &q1, &buffer_with_storage(5, (int[5]){0, 1, 2, 3, 4}), &allocator);
    check(res, CCC_RESULT_OK);
    check(*(int *)front(&q1), 0);
    check(is_empty(&q2), true);
    res = flat_double_ended_queue_copy(&q2, &q1, NULL);
    check(res != CCC_RESULT_OK, true);
    check_end({
        (void)flat_double_ended_queue_clear_and_free(
            &q1, &(CCC_Destructor_context){}, &allocator);
    });
}

check_static_begin(flat_double_ended_queue_test_init_from) {
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    CCC_Flat_double_ended_queue queue = CCC_flat_double_ended_queue_from(
        &allocator, 8, (int[7]){1, 2, 3, 4, 5, 6, 7});
    int elem = 1;
    for (int const *i = CCC_flat_double_ended_queue_begin(&queue);
         i != CCC_flat_double_ended_queue_end(&queue);
         i = CCC_flat_double_ended_queue_next(&queue, i)) {
        check(*i, elem);
        ++elem;
    }
    check(elem, 8);
    check(CCC_flat_double_ended_queue_count(&queue).count, elem - 1);
    check(CCC_flat_double_ended_queue_capacity(&queue).count, elem);
    check_end({
        (void)flat_double_ended_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &allocator);
    });
}

check_static_begin(flat_double_ended_queue_test_init_from_fail) {
    /* Whoops forgot allocation function. */
    CCC_Flat_double_ended_queue queue = CCC_flat_double_ended_queue_from(
        &(CCC_Allocator_context){}, 0, (int[]){1, 2, 3, 4, 5, 6, 7});
    int elem = 1;
    for (int const *i = CCC_flat_double_ended_queue_begin(&queue);
         i != CCC_flat_double_ended_queue_end(&queue);
         i = CCC_flat_double_ended_queue_next(&queue, i)) {
        check(elem, *i);
        ++elem;
    }
    check(elem, 1);
    check(CCC_flat_double_ended_queue_count(&queue).count, 0);
    check(CCC_flat_double_ended_queue_capacity(&queue).count, 0);
    check(CCC_flat_double_ended_queue_push_back(&queue, &(int){},
                                                &(CCC_Allocator_context){}),
          NULL);
    check_end({
        (void)flat_double_ended_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &(CCC_Allocator_context){});
    });
}

check_static_begin(flat_double_ended_queue_test_init_with_capacity) {
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    CCC_Flat_double_ended_queue queue
        = CCC_flat_double_ended_queue_with_capacity(int, &allocator, 8);
    check(CCC_flat_double_ended_queue_capacity(&queue).count, 8);
    check(CCC_flat_double_ended_queue_push_back(&queue, &(int){9}, &allocator)
              != NULL,
          CCC_TRUE);
    check_end(CCC_flat_double_ended_queue_clear_and_free(
                  &queue, &(CCC_Destructor_context){}, &allocator););
}

check_static_begin(flat_double_ended_queue_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    CCC_Flat_double_ended_queue queue
        = CCC_flat_double_ended_queue_with_capacity(
            int, &(CCC_Allocator_context){}, 8);
    check(CCC_flat_double_ended_queue_capacity(&queue).count, 0);
    check(CCC_flat_double_ended_queue_push_back(&queue, &(int){9},
                                                &(CCC_Allocator_context){}),
          NULL);
    check_end({
        CCC_flat_double_ended_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &(CCC_Allocator_context){});
    });
}

int
main(void) {
    return check_run(flat_double_ended_queue_test_construct(),
                     flat_double_ended_queue_test_construct_with_storage(),
                     flat_double_ended_queue_test_copy_no_allocate(),
                     flat_double_ended_queue_test_copy_no_allocate_fail(),
                     flat_double_ended_queue_test_copy_allocate(),
                     flat_double_ended_queue_test_copy_allocate_fail(),
                     flat_double_ended_queue_test_init_from(),
                     flat_double_ended_queue_test_init_from_fail(),
                     flat_double_ended_queue_test_init_with_capacity(),
                     flat_double_ended_queue_test_init_with_capacity_fail());
}
