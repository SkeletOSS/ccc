#include <stddef.h>

#define BITSET_USING_NAMESPACE_CCC
#define FLAT_DOUBLE_ENDED_QUEUE_USING_NAMESPACE_CCC

#include "ccc/bitset.h"
#include "ccc/flat_double_ended_queue.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/stack_allocator.h"

check_static_begin(flat_double_ended_queue_test_iterate_errors) {
    Flat_double_ended_queue q = flat_double_ended_queue_default(int);
    check(CCC_flat_double_ended_queue_begin(NULL), NULL);
    check(CCC_flat_double_ended_queue_reverse_begin(NULL), NULL);
    check(CCC_flat_double_ended_queue_end(NULL), NULL);
    check(CCC_flat_double_ended_queue_reverse_end(NULL), NULL);
    check(CCC_flat_double_ended_queue_next(NULL, &(int){}), NULL);
    check(CCC_flat_double_ended_queue_next(&q, NULL), NULL);
    check(CCC_flat_double_ended_queue_reverse_next(NULL, &(int){}), NULL);
    check(CCC_flat_double_ended_queue_reverse_next(&q, NULL), NULL);
    check_end();
}

static void
destroy_element(CCC_Arguments const arguments) {
    int const *const i = arguments.type;
    Bitset *const is_destroyed_buffer = arguments.context;
    (void)bitset_set(is_destroyed_buffer, (size_t)(*i), CCC_TRUE);
}

check_static_begin(flat_double_ended_queue_test_clear) {
    enum : size_t {
        COUNT = 4,
    };
    Flat_double_ended_queue q = flat_double_ended_queue_with_storage(
        COUNT,
        (int[COUNT]){
            0,
            1,
            2,
            3,
        }
    );
    check(CCC_flat_double_ended_queue_capacity(&q).count >= COUNT, CCC_TRUE);
    check(CCC_flat_double_ended_queue_count(&q).count, COUNT);
    check(
        CCC_flat_double_ended_queue_clear(&q, &(CCC_Destructor){}),
        CCC_RESULT_OK
    );
    check(CCC_flat_double_ended_queue_capacity(&q).count >= COUNT, CCC_TRUE);
    check(CCC_flat_double_ended_queue_count(&q).count, 0);
    check_end();
}

check_static_begin(flat_double_ended_queue_test_clear_destructor) {
    enum : size_t {
        COUNT = 4,
    };
    Flat_double_ended_queue q = flat_double_ended_queue_with_storage(
        COUNT,
        (int[COUNT]){
            0,
            1,
            2,
            3,
        }
    );
    Bitset is_destroyed = bitset_with_storage(COUNT, (Bit[COUNT]){});
    check(CCC_flat_double_ended_queue_capacity(&q).count >= COUNT, CCC_TRUE);
    check(CCC_flat_double_ended_queue_count(&q).count, COUNT);
    check(
        CCC_flat_double_ended_queue_clear(NULL, &(CCC_Destructor){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_double_ended_queue_clear(&q, NULL), CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_double_ended_queue_clear(
            &q,
            &(CCC_Destructor){
                .destroy = destroy_element,
                .context = &is_destroyed,
            }
        ),
        CCC_RESULT_OK
    );
    size_t i = 0;
    while (!bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(i, COUNT);
    check_end();
}

check_static_begin(flat_double_ended_queue_test_clear_and_free) {
    enum : size_t {
        COUNT = 4,
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[COUNT * 2]){}),
    };
    Flat_double_ended_queue q = flat_double_ended_queue_from(
        allocator,
        0,
        (int[COUNT]){
            0,
            1,
            2,
            3,
        }
    );
    check(CCC_flat_double_ended_queue_capacity(&q).count >= COUNT, CCC_TRUE);
    check(CCC_flat_double_ended_queue_count(&q).count, COUNT);
    check(
        CCC_flat_double_ended_queue_clear_and_free(
            &q, &(CCC_Destructor){}, &allocator
        ),
        CCC_RESULT_OK
    );
    check(CCC_flat_double_ended_queue_capacity(&q).count, 0);
    check(CCC_flat_double_ended_queue_count(&q).count, 0);
    check_end();
}

check_static_begin(flat_double_ended_queue_test_clear_and_free_destructor) {
    enum : size_t {
        COUNT = 4,
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[COUNT * 2]){}),
    };
    Flat_double_ended_queue q = flat_double_ended_queue_from(
        allocator,
        0,
        (int[COUNT]){
            0,
            1,
            2,
            3,
        }
    );
    Bitset is_destroyed = bitset_with_storage(COUNT, (Bit[COUNT]){});
    check(CCC_flat_double_ended_queue_capacity(&q).count >= COUNT, CCC_TRUE);
    check(CCC_flat_double_ended_queue_count(&q).count, COUNT);
    check(
        CCC_flat_double_ended_queue_clear_and_free(
            NULL, &(CCC_Destructor){}, &allocator
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_double_ended_queue_clear_and_free(&q, NULL, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_double_ended_queue_clear_and_free(
            &q, &(CCC_Destructor){}, NULL
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_double_ended_queue_clear_and_free(
            &q,
            &(CCC_Destructor){
                .destroy = destroy_element,
                .context = &is_destroyed,
            },
            &allocator
        ),
        CCC_RESULT_OK
    );
    size_t i = 0;
    while (!bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(i, COUNT);
    check_end();
}

int
main(void) {
    return check_run(
        flat_double_ended_queue_test_iterate_errors(),
        flat_double_ended_queue_test_clear(),
        flat_double_ended_queue_test_clear_destructor(),
        flat_double_ended_queue_test_clear_and_free(),
        flat_double_ended_queue_test_clear_and_free_destructor(),
    );
}
