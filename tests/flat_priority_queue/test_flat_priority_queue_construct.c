#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
#define BUFFER_USING_NAMESPACE_CCC

#include "buffer.h"
#include "checkers.h"
#include "flat_priority_queue.h"
#include "flat_priority_queue_utility.h"
#include "sort.h"
#include "traits.h"
#include "types.h"
#include "utility/allocate.h"
#include "utility/stack_allocator.h"

static CCC_Order
int_order(CCC_Comparator_arguments const order) {
    int a = *((int const *const)order.type_left);
    int b = *((int const *const)order.type_right);
    return (a > b) - (a < b);
}

check_static_begin(flat_priority_queue_test_empty) {
    struct Val vals[2] = {};
    Flat_priority_queue priority_queue = flat_priority_queue_for(
        struct Val, CCC_ORDER_LESSER,
        &(CCC_Comparator_context){.compare = val_order},
        (sizeof(vals) / sizeof(struct Val)), vals);
    check(flat_priority_queue_is_empty(&priority_queue), true);
    check_end();
}

check_static_begin(flat_priority_queue_test_with_storage) {
    Flat_priority_queue priority_queue = flat_priority_queue_with_storage(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = val_order},
        (struct Val[3]){});
    check(flat_priority_queue_is_empty(&priority_queue), true);
    check(flat_priority_queue_capacity(&priority_queue).count, 3);
    check_end();
}

check_static_begin(flat_priority_queue_test_macro) {
    struct Val vals[2] = {};
    Flat_priority_queue priority_queue = flat_priority_queue_for(
        struct Val, CCC_ORDER_LESSER,
        &(CCC_Comparator_context){.compare = val_order},
        (sizeof(vals) / sizeof(struct Val)), vals);
    struct Val *res = flat_priority_queue_emplace(
        &priority_queue, &(CCC_Allocator_context){},
        (struct Val){.val = 0, .id = 0});
    check(res != NULL, true);
    check(flat_priority_queue_is_empty(&priority_queue), false);
    struct Val *res2 = flat_priority_queue_emplace(
        &priority_queue, &(CCC_Allocator_context){},
        (struct Val){.val = 0, .id = 0});
    check(res2 != NULL, true);
    check_end();
}

check_static_begin(flat_priority_queue_test_macro_grow) {
    Flat_priority_queue priority_queue = flat_priority_queue_for(
        struct Val, CCC_ORDER_LESSER,
        &(CCC_Comparator_context){.compare = val_order}, 0, CCC_DEFAULT);
    struct Val *res = flat_priority_queue_emplace(
        &priority_queue, &std_allocator, (struct Val){.val = 0, .id = 0});
    check(res != NULL, true);
    check(flat_priority_queue_is_empty(&priority_queue), false);
    struct Val *res2 = flat_priority_queue_emplace(
        &priority_queue, &std_allocator, (struct Val){.val = 0, .id = 0});
    check(res2 != NULL, true);
    check_end(
        (void)CCC_flat_priority_queue_clear_and_free(
            &priority_queue, &(CCC_Destructor_context){}, &std_allocator););
}

check_static_begin(flat_priority_queue_test_push) {
    struct Val vals[3] = {};
    Flat_priority_queue priority_queue = flat_priority_queue_for(
        struct Val, CCC_ORDER_LESSER,
        &(CCC_Comparator_context){.compare = val_order},
        (sizeof(vals) / sizeof(struct Val)), vals);
    struct Val *res = push(&priority_queue, &vals[0], &(struct Val){},
                           &(CCC_Allocator_context){});
    check(res != NULL, true);
    check(flat_priority_queue_is_empty(&priority_queue), false);
    check_end();
}

check_static_begin(flat_priority_queue_test_raw_type) {
    int vals[4] = {};
    Flat_priority_queue priority_queue = flat_priority_queue_for(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = val_order},
        (sizeof(vals) / sizeof(int)), vals);
    int val = 1;
    int *res
        = push(&priority_queue, &val, &(int){}, &(CCC_Allocator_context){});
    check(res != NULL, true);
    check(flat_priority_queue_is_empty(&priority_queue), false);
    res = flat_priority_queue_emplace(&priority_queue,
                                      &(CCC_Allocator_context){}, -1);
    check(res != NULL, true);
    check(flat_priority_queue_count(&priority_queue).count, 2);
    int *popped = front(&priority_queue);
    check(*popped, -1);
    check_end();
}

check_static_begin(flat_priority_queue_test_heapify) {
    srand(time(NULL)); /* NOLINT */
    enum : size_t {
        HEAPIFY_CAP = 100,
    };
    int heap[HEAPIFY_CAP] = {};
    for (size_t i = 0; i < HEAPIFY_CAP; ++i) {
        heap[i] = rand_range(-99, (int)HEAPIFY_CAP); /* NOLINT */
    }
    Flat_priority_queue priority_queue = flat_priority_queue_heapify(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        HEAPIFY_CAP, HEAPIFY_CAP, heap);
    int prev = *((int *)flat_priority_queue_front(&priority_queue));
    (void)pop(&priority_queue, &(int){});
    while (!flat_priority_queue_is_empty(&priority_queue)) {
        int cur = *((int *)flat_priority_queue_front(&priority_queue));
        (void)pop(&priority_queue, &(int){});
        check(cur >= prev, true);
        prev = cur;
    }
    check_end();
}

check_static_begin(flat_priority_queue_test_heapify_copy) {
    srand(time(NULL)); /* NOLINT */
    enum : size_t {
        HEAPIFY_COPY_CAP = 100,
    };
    Flat_priority_queue priority_queue = CCC_flat_priority_queue_with_storage(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        (int[HEAPIFY_COPY_CAP]){});
    Buffer input
        = buffer_with_storage(HEAPIFY_COPY_CAP, (int[HEAPIFY_COPY_CAP]){});
    for (int *i = buffer_begin(&input); i != buffer_end(&input);
         i = buffer_next(&input, i)) {
        *i = rand_range(-99, 99); /* NOLINT */
    }
    check(flat_priority_queue_copy_heapify(&priority_queue, &input, &(int){},
                                           &std_allocator),
          CCC_RESULT_OK);
    check(flat_priority_queue_count(&priority_queue).count, HEAPIFY_COPY_CAP);
    int prev = *((int *)flat_priority_queue_front(&priority_queue));
    (void)pop(&priority_queue, &(int){});
    while (!flat_priority_queue_is_empty(&priority_queue)) {
        int cur = *((int *)flat_priority_queue_front(&priority_queue));
        (void)pop(&priority_queue, &(int){});
        check(cur >= prev, true);
        prev = cur;
    }
    check_end();
}

check_static_begin(flat_priority_queue_test_heapsort) {
    enum : int {
        HPSORTCAP = 100,
    };
    srand(time(NULL)); /* NOLINT */
    Buffer storage = buffer_with_storage(HPSORTCAP, (int[HPSORTCAP]){});
    for (int *i = buffer_begin(&storage); i != buffer_end(&storage);
         i = buffer_next(&storage, i)) {
        *i = rand_range(-99, HPSORTCAP); /* NOLINT */
    }
    CCC_Result const result
        = CCC_sort_heapsort(&storage, &(int){}, CCC_ORDER_GREATER,
                            &(CCC_Comparator_context){.compare = int_order});
    check(result, CCC_RESULT_OK);
    int const *prev = begin(&storage);
    check(prev != NULL, true);
    check(CCC_buffer_count(&storage).count, HPSORTCAP);
    size_t count = 1;
    for (int const *cur = next(&storage, prev); cur != end(&storage);
         cur = next(&storage, cur)) {
        check(*prev >= *cur, true);
        prev = cur;
        ++count;
    }
    check(count, HPSORTCAP);
    check_end();
}

check_static_begin(flat_priority_queue_test_copy_no_allocate) {
    Flat_priority_queue source = flat_priority_queue_with_storage(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        (int[4]){});
    Flat_priority_queue destination = flat_priority_queue_with_storage(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        (int[5]){});
    (void)push(&source, &(int){}, &(int){}, &(CCC_Allocator_context){});
    (void)push(&source, &(int){1}, &(int){}, &(CCC_Allocator_context){});
    (void)push(&source, &(int){2}, &(int){}, &(CCC_Allocator_context){});
    check(count(&source).count, 3);
    check(*(int *)front(&source), 0);
    check(is_empty(&destination), true);
    CCC_Result res = flat_priority_queue_copy(&destination, &source,
                                              &(CCC_Allocator_context){});
    check(res, CCC_RESULT_OK);
    check(count(&destination).count, 3);
    while (!is_empty(&source) && !is_empty(&destination)) {
        int f1 = *(int *)front(&source);
        int f2 = *(int *)front(&destination);
        (void)pop(&source, &(int){});
        (void)pop(&destination, &(int){});
        check(f1, f2);
    }
    check(is_empty(&source), is_empty(&destination));
    check_end();
}

check_static_begin(flat_priority_queue_test_copy_no_allocate_fail) {
    Flat_priority_queue source = flat_priority_queue_with_storage(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        (int[4]){});
    Flat_priority_queue destination = flat_priority_queue_with_storage(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        (int[2]){});
    (void)push(&source, &(int){}, &(int){}, &(CCC_Allocator_context){});
    (void)push(&source, &(int){1}, &(int){}, &(CCC_Allocator_context){});
    (void)push(&source, &(int){2}, &(int){}, &(CCC_Allocator_context){});
    check(count(&source).count, 3);
    check(*(int *)front(&source), 0);
    check(is_empty(&destination), true);
    CCC_Result res = flat_priority_queue_copy(&destination, &source,
                                              &(CCC_Allocator_context){});
    check(res != CCC_RESULT_OK, true);
    check_end();
}

check_static_begin(flat_priority_queue_test_copy_allocate) {
    CCC_Allocator_context const stack_allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[16]){}),
    };
    Flat_priority_queue source = flat_priority_queue_with_capacity(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        &stack_allocator, 8);
    Flat_priority_queue destination = flat_priority_queue_default(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order});
    (void)push(&source, &(int){}, &(int){}, &stack_allocator);
    (void)push(&source, &(int){1}, &(int){}, &stack_allocator);
    (void)push(&source, &(int){2}, &(int){}, &stack_allocator);
    check(*(int *)front(&source), 0);
    check(is_empty(&destination), true);
    CCC_Result res
        = flat_priority_queue_copy(&destination, &source, &stack_allocator);
    check(res, CCC_RESULT_OK);
    check(count(&destination).count, 3);
    while (!is_empty(&source) && !is_empty(&destination)) {
        int f1 = *(int *)front(&source);
        int f2 = *(int *)front(&destination);
        (void)pop(&source, &(int){});
        (void)pop(&destination, &(int){});
        check(f1, f2);
    }
    check(is_empty(&source), is_empty(&destination));
    check_end({
        (void)flat_priority_queue_clear_and_free(
            &source, &(CCC_Destructor_context){}, &stack_allocator);
        (void)flat_priority_queue_clear_and_free(
            &destination, &(CCC_Destructor_context){}, &stack_allocator);
    });
}

check_static_begin(flat_priority_queue_test_copy_allocate_fail) {
    CCC_Allocator_context const stack_allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[16]){}),
    };
    Flat_priority_queue source = flat_priority_queue_with_capacity(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        &stack_allocator, 8);
    Flat_priority_queue destination = flat_priority_queue_default(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order});
    (void)push(&source, &(int){}, &(int){}, &stack_allocator);
    (void)push(&source, &(int){1}, &(int){}, &stack_allocator);
    (void)push(&source, &(int){2}, &(int){}, &stack_allocator);
    check(*(int *)front(&source), 0);
    check(is_empty(&destination), true);
    CCC_Result res = flat_priority_queue_copy(&destination, &source,
                                              &(CCC_Allocator_context){});
    check(res != CCC_RESULT_OK, true);
    check_end({
        (void)flat_priority_queue_clear_and_free(
            &source, &(CCC_Destructor_context){}, &stack_allocator);
    });
}

check_static_begin(flat_priority_queue_test_init_from) {
    CCC_Allocator_context const stack_allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    CCC_Flat_priority_queue queue = CCC_flat_priority_queue_from(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        &stack_allocator, 8, (int[7]){8, 6, 7, 5, 3, 0, 9});
    int count = 0;
    int prev = INT_MIN;
    check(CCC_flat_priority_queue_count(&queue).count, 7);
    while (!CCC_flat_priority_queue_is_empty(&queue)) {
        int const front = *(int *)CCC_flat_priority_queue_front(&queue);
        check(front > prev, true);
        CCC_Result const pop = CCC_flat_priority_queue_pop(&queue, &(int){});
        check(pop, CCC_RESULT_OK);
        ++count;
    }
    check(count, 7);
    check(CCC_flat_priority_queue_capacity(&queue).count >= 7, true);
    check_end({
        (void)flat_priority_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &stack_allocator);
    });
}

check_static_begin(flat_priority_queue_test_init_from_fail) {
    /* Whoops forgot allocation function. */
    CCC_Flat_priority_queue queue = CCC_flat_priority_queue_from(
        CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        &(CCC_Allocator_context){}, 0, (int[]){8, 6, 7, 5, 3, 0, 9});
    int count = 0;
    int prev = INT_MIN;
    check(CCC_flat_priority_queue_count(&queue).count, 0);
    while (!CCC_flat_priority_queue_is_empty(&queue)) {
        int const front = *(int *)CCC_flat_priority_queue_front(&queue);
        check(front > prev, true);
        ++count;
        CCC_Result const pop = CCC_flat_priority_queue_pop(&queue, &(int){});
        check(pop, CCC_RESULT_OK);
    }
    check(count, 0);
    check(CCC_flat_priority_queue_capacity(&queue).count, 0);
    check(CCC_flat_priority_queue_push(&queue, &(int){12}, &(int){},
                                       &(CCC_Allocator_context){}),
          NULL);
    check_end({
        (void)CCC_flat_priority_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &(CCC_Allocator_context){});
    });
}

check_static_begin(flat_priority_queue_test_init_with_capacity) {
    CCC_Allocator_context const stack_allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    CCC_Flat_priority_queue queue = CCC_flat_priority_queue_with_capacity(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        &stack_allocator, 8);
    check(CCC_flat_priority_queue_capacity(&queue).count, 8);
    check(CCC_flat_priority_queue_push(&queue, &(int){9}, &(int){},
                                       &stack_allocator)
              != NULL,
          CCC_TRUE);
    check_end({
        (void)flat_priority_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &stack_allocator);
    });
}

check_static_begin(flat_priority_queue_test_init_with_capacity_fail) {
    /* Forgot allocation function. */
    CCC_Flat_priority_queue queue = CCC_flat_priority_queue_with_capacity(
        int, CCC_ORDER_LESSER, &(CCC_Comparator_context){.compare = int_order},
        &(CCC_Allocator_context){}, 8);
    check(CCC_flat_priority_queue_capacity(&queue).count, 0);
    check(CCC_flat_priority_queue_push(&queue, &(int){9}, &(int){},
                                       &(CCC_Allocator_context){}),
          NULL);
    check_end({
        (void)CCC_flat_priority_queue_clear_and_free(
            &queue, &(CCC_Destructor_context){}, &(CCC_Allocator_context){});
    });
}

int
main(void) {
    return check_run(
        flat_priority_queue_test_empty(),
        flat_priority_queue_test_with_storage(),
        flat_priority_queue_test_macro(), flat_priority_queue_test_macro_grow(),
        flat_priority_queue_test_push(), flat_priority_queue_test_raw_type(),
        flat_priority_queue_test_heapify(),
        flat_priority_queue_test_heapify_copy(),
        flat_priority_queue_test_copy_no_allocate(),
        flat_priority_queue_test_copy_no_allocate_fail(),
        flat_priority_queue_test_copy_allocate(),
        flat_priority_queue_test_copy_allocate_fail(),
        flat_priority_queue_test_heapsort(),
        flat_priority_queue_test_init_from(),
        flat_priority_queue_test_init_from_fail(),
        flat_priority_queue_test_init_with_capacity(),
        flat_priority_queue_test_init_with_capacity_fail());
}
