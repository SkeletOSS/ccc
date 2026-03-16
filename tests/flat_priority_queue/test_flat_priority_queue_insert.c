#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define TRAITS_USING_NAMESPACE_CCC

#include "checkers.h"
#include "flat_priority_queue.h"
#include "flat_priority_queue_utility.h"
#include "traits.h"
#include "types.h"
#include "utility/allocate.h"
#include "utility/stack_allocator.h"

check_static_begin(flat_priority_queue_test_insert_one) {
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[8]){}),
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_capacity(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order}, &allocator, 8);
    (void)push(&flat_priority_queue, &(struct Val){.val = 1}, &(struct Val){},
               &allocator);
    check(CCC_flat_priority_queue_is_empty(&flat_priority_queue), false);
    check_end();
}

check_static_begin(flat_priority_queue_test_insert_three) {
    size_t size = 3;
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[8]){}),
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_capacity(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order}, &allocator, 8);
    for (size_t i = 0; i < size; ++i) {
        (void)push(&flat_priority_queue,
                   &(struct Val){
                       .val = (int)i,
                   },
                   &(struct Val){}, &allocator);
        check(validate(&flat_priority_queue), true);
        check(CCC_flat_priority_queue_count(&flat_priority_queue).count, i + 1);
    }
    check(CCC_flat_priority_queue_count(&flat_priority_queue).count, size);
    check_end();
}

check_static_begin(flat_priority_queue_test_insert_three_dups) {
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[8]){}),
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_capacity(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order}, &allocator, 8);
    for (int i = 0; i < 3; ++i) {
        (void)push(&flat_priority_queue,
                   &(struct Val){
                       .val = 0,
                   },
                   &(struct Val){}, &allocator);
        check(validate(&flat_priority_queue), true);
        check(CCC_flat_priority_queue_count(&flat_priority_queue).count,
              (size_t)i + 1);
    }
    check(CCC_flat_priority_queue_count(&flat_priority_queue).count, (size_t)3);
    check_end();
}

check_static_begin(flat_priority_queue_test_insert_shuffle) {
    size_t const size = 50;
    int const prime = 53;
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[50]){}),
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_capacity(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order}, &allocator, 50);
    check(insert_shuffled(&flat_priority_queue, size, prime, &allocator),
          CHECK_PASS);

    struct Val const *min = front(&flat_priority_queue);
    check(min->val, 0);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &flat_priority_queue), CHECK_PASS);
    int prev = sorted_check[0];
    for (size_t i = 0; i < size; ++i) {
        check(prev <= sorted_check[i], true);
        prev = sorted_check[i];
    }
    check_end();
}

/** This should remain dynamic allocator test to test resizing logic. */
check_static_begin(flat_priority_queue_test_insert_shuffle_grow) {
    size_t const size = 50;
    int const prime = 53;
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_default(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order});
    check(insert_shuffled(&flat_priority_queue, size, prime, &std_allocator),
          CHECK_PASS);

    struct Val const *min = front(&flat_priority_queue);
    check(min->val, 0);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &flat_priority_queue), CHECK_PASS);
    int prev = sorted_check[0];
    for (size_t i = 0; i < size; ++i) {
        check(prev <= sorted_check[i], true);
        prev = sorted_check[i];
    }
    check_end((void)CCC_flat_priority_queue_clear_and_free(
                  &flat_priority_queue, &(CCC_Destructor_context){},
                  &std_allocator););
}

check_static_begin(flat_priority_queue_test_insert_shuffle_reserve) {
    size_t const size = 50;
    int const prime = 53;
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[50]){}),
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_capacity(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order}, &allocator, 50);
    check(capacity(&flat_priority_queue).count, 50);
    check(insert_shuffled(&flat_priority_queue, size, prime,
                          &(CCC_Allocator_context){}),
          CHECK_PASS);
    struct Val const *min = front(&flat_priority_queue);
    check(min->val, 0);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &flat_priority_queue), CHECK_PASS);
    int prev = sorted_check[0];
    for (size_t i = 0; i < size; ++i) {
        check(prev <= sorted_check[i], true);
        prev = sorted_check[i];
    }
    check_end(clear_and_free(&flat_priority_queue, &(CCC_Destructor_context){},
                             &allocator););
}

check_static_begin(flat_priority_queue_test_read_max_min) {
    size_t const size = 10;
    CCC_Allocator_context const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[10]){}),
    };
    CCC_Flat_priority_queue flat_priority_queue
        = CCC_flat_priority_queue_with_capacity(
            struct Val, CCC_ORDER_LESSER,
            &(CCC_Comparator_context){.compare = val_order}, &allocator, 10);
    for (size_t i = 0; i < size; ++i) {
        (void)push(&flat_priority_queue,
                   &(struct Val){
                       .val = (int)size - (int)i,
                   },
                   &(struct Val){}, &allocator);
        check(validate(&flat_priority_queue), true);
        check(CCC_flat_priority_queue_count(&flat_priority_queue).count, i + 1);
    }
    check(CCC_flat_priority_queue_count(&flat_priority_queue).count,
          (size_t)10);
    struct Val const *min = front(&flat_priority_queue);
    check(min->val, size - (size - 1));
    check_end();
}

int
main(void) {
    return check_run(flat_priority_queue_test_insert_one(),
                     flat_priority_queue_test_insert_three(),
                     flat_priority_queue_test_insert_three_dups(),
                     flat_priority_queue_test_insert_shuffle(),
                     flat_priority_queue_test_insert_shuffle_grow(),
                     flat_priority_queue_test_insert_shuffle_reserve(),
                     flat_priority_queue_test_read_max_min());
}
