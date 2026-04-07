#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define FLAT_BUFFER_USING_NAMESPACE_CCC
#define FLAT_BITSET_USING_NAMESPACE_CCC

#include "ccc/flat_bitset.h"
#include "ccc/flat_buffer.h"
#include "ccc/types.h"
#include "checkers.h"
#include "flat_buffer_utility.h"
#include "utility/random.h"
#include "utility/stack_allocator.h"
#include "utility/std_allocator.h"

check_static_begin(flat_buffer_test_push_pop_fixed) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    int const push[8] = {7, 6, 5, 4, 3, 2, 1, 0};
    size_t count = 0;
    for (size_t i = 0; i < sizeof(push) / sizeof(*push); ++i) {
        int const *const p
            = flat_buffer_push_back(&b, &push[i], &(CCC_Allocator){});
        check(p != NULL, CCC_TRUE);
        check(*p, push[i]);
        ++count;
    }
    check(flat_buffer_count(&b).count, sizeof(push) / sizeof(*push));
    check(flat_buffer_count(&b).count, count);
    check(
        flat_buffer_push_back(&b, &(int){99}, &(CCC_Allocator){}) == NULL,
        CCC_TRUE
    );
    while (!flat_buffer_is_empty(&b)) {
        int const v = *(int *)flat_buffer_back(&b);
        check(flat_buffer_pop_back(&b), CCC_RESULT_OK);
        --count;
        check(v, push[count]);
    }
    check(flat_buffer_count(&b).count, count);
    check(count, 0);
    check_end();
}

check_static_begin(flat_buffer_test_push_erase_one) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    check(
        flat_buffer_push_back(&b, &(int){99}, &(CCC_Allocator){}) == NULL,
        CCC_FALSE
    );
    check(flat_buffer_erase(&b, 0), CCC_RESULT_OK);
    check(flat_buffer_count(&b).count, 0);
    check_end();
}

check_static_begin(flat_buffer_test_push_resize_pop) {
    Flat_buffer b = flat_buffer_default(int);
    size_t const cap = 32;
    int *const many = malloc(sizeof(int) * cap);
    iota(many, cap, 0);
    check(many != NULL, CCC_TRUE);
    size_t count = 0;
    for (size_t i = 0; i < cap; ++i) {
        int *p = flat_buffer_push_back(&b, &many[i], &std_allocator);
        check(p != NULL, CCC_TRUE);
        check(*p, many[i]);
        ++count;
    }
    check(count, cap);
    check(flat_buffer_count(&b).count, cap);
    check(flat_buffer_capacity(&b).count >= cap, CCC_TRUE);
    while (!flat_buffer_is_empty(&b)) {
        int const v = *flat_buffer_back_as(&b, int);
        check(flat_buffer_pop_back(&b), CCC_RESULT_OK);
        --count;
        check(v, many[count]);
    }
    check(flat_buffer_count(&b).count, count);
    check(count, 0);
    check_end({
        (void)flat_buffer_clear_and_free(
            &b, &(CCC_Destructor){}, &std_allocator
        );
        free(many);
    });
}

check_static_begin(flat_buffer_test_clear_then_clear_and_free) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    int i = 0;
    while (!flat_buffer_is_full(&b)) {
        int *pushed = flat_buffer_push_back(&b, &i, &(CCC_Allocator){});
        check(pushed != NULL, CCC_TRUE);
    }
    check(flat_buffer_count(&b).count, 8);
    check(flat_buffer_capacity(&b).count, 8);
    CCC_Result const result = flat_buffer_clear(&b, &(CCC_Destructor){});
    check(result, CCC_RESULT_OK);
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_capacity(&b).count, 8);
    check_end({
        (void)flat_buffer_clear_and_free(
            &b, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

static void
destroy_element(CCC_Arguments const arguments) {
    int *const i = arguments.type;
    Flat_bitset *const is_destroyed_buffer = arguments.context;
    (void)flat_bitset_set(is_destroyed_buffer, (size_t)*i, CCC_TRUE);
}

check_static_begin(flat_buffer_test_clear_with_destructor) {
    Flat_buffer b = flat_buffer_with_storage(0, (int[8]){});
    Flat_bitset is_destroyed = flat_bitset_with_storage(0, (Bit[8]){});
    int i = 0;
    while (!flat_buffer_is_full(&b)) {
        CCC_Result const bit_push = flat_bitset_push_back(
            &is_destroyed, CCC_FALSE, &(CCC_Allocator){}
        );
        check(bit_push, CCC_RESULT_OK);
        int *pushed = flat_buffer_push_back(&b, &i, &(CCC_Allocator){});
        check(pushed != NULL, CCC_TRUE);
        ++i;
    }
    check(flat_buffer_count(&b).count, 8);
    check(flat_buffer_capacity(&b).count, 8);
    CCC_Result result = flat_buffer_clear(
        &b,
        &(CCC_Destructor){
            .destroy = destroy_element,
            .context = &is_destroyed,
        }
    );
    check(result, CCC_RESULT_OK);
    i = 0;
    while (!flat_bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = flat_bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_capacity(&b).count, 8);
    check(flat_buffer_capacity(&b).count, i);
    check_end();
}

check_static_begin(flat_buffer_test_clear_and_free_with_destructor) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((int[8]){}),
    };
    Flat_buffer b = flat_buffer_with_capacity(int, allocator, 8);
    Flat_bitset is_destroyed = flat_bitset_with_storage(0, (Bit[8]){});
    int i = 0;
    while (!flat_buffer_is_full(&b)) {
        CCC_Result const bit_push = flat_bitset_push_back(
            &is_destroyed, CCC_FALSE, &(CCC_Allocator){}
        );
        check(bit_push, CCC_RESULT_OK);
        int *pushed = flat_buffer_push_back(&b, &i, &(CCC_Allocator){});
        check(pushed != NULL, CCC_TRUE);
        ++i;
    }
    check(flat_buffer_count(&b).count, 8);
    check(flat_buffer_capacity(&b).count, 8);
    CCC_Result result = flat_buffer_clear_and_free(
        &b,
        &(CCC_Destructor){
            .destroy = destroy_element,
            .context = &is_destroyed,
        },
        &allocator
    );
    check(result, CCC_RESULT_OK);
    i = 0;
    while (!flat_bitset_is_empty(&is_destroyed)) {
        CCC_Tribool const was_destroyed = flat_bitset_pop_back(&is_destroyed);
        check(was_destroyed, CCC_TRUE);
        ++i;
    }
    check(flat_buffer_count(&b).count, 0);
    check(flat_buffer_capacity(&b).count, 0);
    check(i, 8);
    check_end();
}

check_static_begin(flat_buffer_test_daily_temperatures) {
    enum : size_t {
        TMPCAP = 8,
    };
    Flat_buffer const temps = flat_buffer_with_storage(
        TMPCAP, (int[TMPCAP]){73, 74, 75, 71, 69, 72, 76, 73}
    );
    Flat_buffer const correct = flat_buffer_with_storage(
        TMPCAP, (int[TMPCAP]){1, 1, 4, 2, 1, 1, 0, 0}
    );
    Flat_buffer res = flat_buffer_with_storage(TMPCAP, (int[TMPCAP]){});
    Flat_buffer idx_stack = flat_buffer_with_storage(0, (int[TMPCAP]){});
    for (int i = 0, end = (int)flat_buffer_count(&temps).count; i < end; ++i) {
        while (!flat_buffer_is_empty(&idx_stack)
               && *flat_buffer_as(&temps, int, (size_t)i) > *flat_buffer_as(
                      &temps, int, (size_t)*flat_buffer_back_as(&idx_stack, int)
                  )) {
            int const *const pointer = flat_buffer_emplace(
                &res,
                ((size_t)*flat_buffer_back_as(&idx_stack, int)),
                i - *flat_buffer_back_as(&idx_stack, int)
            );
            check(pointer != NULL, CCC_TRUE);
            CCC_Result const r = flat_buffer_pop_back(&idx_stack);
            check(r, CCC_RESULT_OK);
        }
        int const *const pointer
            = flat_buffer_push_back(&idx_stack, &i, &(CCC_Allocator){});
        check(pointer != NULL, CCC_TRUE);
    }
    check(
        memcmp(
            flat_buffer_begin(&res),
            flat_buffer_begin(&correct),
            flat_buffer_count_bytes(&correct).count
        ),
        0
    );
    check_end();
}

static CCC_Order
order_car_idx(CCC_Comparator_arguments const order) {
    Flat_buffer const *const int_positions = order.context;
    int const *const left_pos
        = flat_buffer_at(int_positions, (size_t)*(int *)order.type_left);
    int const *const right_pos
        = flat_buffer_at(int_positions, (size_t)*(int *)order.type_right);
    /* Reversed sort. We want descending not ascending order. We ask how many
       car fleets there will be by starting at the cars furthest away that may
       catch up to those ahead. */
    return (*left_pos < *right_pos) - (*left_pos > *right_pos);
}

check_static_begin(flat_buffer_test_car_fleet) {
    enum : size_t {
        CARCAP = 5,
    };
    Flat_buffer positions
        = flat_buffer_with_storage(CARCAP, (int[CARCAP]){10, 8, 0, 5, 3});
    Flat_buffer const speeds
        = flat_buffer_with_storage(CARCAP, (int[CARCAP]){2, 4, 1, 1, 3});
    int const correct_fleet_count = 3;
    Flat_buffer car_idx = flat_buffer_with_storage(CARCAP, (int[CARCAP]){});
    iota(flat_buffer_begin(&car_idx), CARCAP, 0);
    quicksort(
        &car_idx,
        &(int){},
        CCC_ORDER_LESSER,
        &(CCC_Comparator){
            .compare = order_car_idx,
            .context = &positions,
        }
    );
    int target = 12;
    int fleets = 1;
    double slowest_time_to_target
        = ((double)(target - *flat_buffer_as(&positions, int, 0)))
        / *flat_buffer_as(&speeds, int, 0);
    for (int const *iterator = flat_buffer_begin(&car_idx);
         iterator != flat_buffer_end(&car_idx);
         iterator = flat_buffer_next(&car_idx, iterator)) {
        double const time_of_closer_car
            = ((double)(target
                        - *flat_buffer_as(&positions, int, (size_t)*iterator)))
            / *flat_buffer_as(&speeds, int, (size_t)*iterator);
        if (time_of_closer_car > slowest_time_to_target) {
            ++fleets;
            slowest_time_to_target = time_of_closer_car;
        }
    }
    check(fleets, correct_fleet_count);
    check_end();
}

check_static_begin(flat_buffer_test_largest_rectangle_in_histogram) {
    enum : size_t {
        HCAP = 6,
    };
    Flat_buffer const heights
        = flat_buffer_with_storage(HCAP, (int[HCAP]){2, 1, 5, 6, 2, 3});
    int const correct_max_rectangle = 10;
    int max_rectangle = 0;
    Flat_buffer bar_indices = flat_buffer_with_storage(0, (int[HCAP]){});
    for (int i = 0, end = (int)flat_buffer_count(&heights).count; i <= end;
         ++i) {
        while (
            !flat_buffer_is_empty(&bar_indices)
            && (i == end
                || *flat_buffer_as(&heights, int, (size_t)i) < *flat_buffer_as(
                       &heights,
                       int,
                       (size_t)*flat_buffer_back_as(&bar_indices, int)
                   ))) {
            int const stack_top_i = *flat_buffer_back_as(&bar_indices, int);
            int const stack_top_height
                = *flat_buffer_as(&heights, int, (size_t)stack_top_i);
            CCC_Result const r = flat_buffer_pop_back(&bar_indices);
            check(r, CCC_RESULT_OK);
            int const w = flat_buffer_is_empty(&bar_indices)
                            ? i
                            : i - *flat_buffer_back_as(&bar_indices, int) - 1;
            max_rectangle = maxint(max_rectangle, stack_top_height * w);
        }
        int const *const pointer
            = flat_buffer_push_back(&bar_indices, &i, &(CCC_Allocator){});
        check(pointer != NULL, CCC_TRUE);
    }
    check(max_rectangle, correct_max_rectangle);
    check_end();
}

check_static_begin(flat_buffer_test_erase) {
    enum : size_t {
        BECAP = 8,
    };
    Flat_buffer b
        = flat_buffer_with_storage(BECAP, (int[BECAP]){0, 1, 2, 3, 4, 5, 6, 7});
    check(flat_buffer_count(&b).count, BECAP);
    CCC_Result r = flat_buffer_erase(&b, 4);
    check(r, CCC_RESULT_OK);
    CCC_Order order
        = buforder(&b, BECAP - 1, (int[BECAP - 1]){0, 1, 2, 3, 5, 6, 7});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, BECAP - 1);
    r = flat_buffer_erase(&b, 0);
    check(r, CCC_RESULT_OK);
    order = buforder(&b, BECAP - 2, (int[BECAP - 2]){1, 2, 3, 5, 6, 7});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, BECAP - 2);
    r = flat_buffer_erase(&b, BECAP - 3);
    check(r, CCC_RESULT_OK);
    order = buforder(&b, BECAP - 3, (int[BECAP - 3]){1, 2, 3, 5, 6});
    check(order, CCC_ORDER_EQUAL);
    check(flat_buffer_count(&b).count, BECAP - 3);
    check_end();
}

int
main(void) {
    return check_run(
        flat_buffer_test_push_pop_fixed(),
        flat_buffer_test_push_erase_one(),
        flat_buffer_test_push_resize_pop(),
        flat_buffer_test_clear_then_clear_and_free(),
        flat_buffer_test_clear_with_destructor(),
        flat_buffer_test_clear_and_free_with_destructor(),
        flat_buffer_test_daily_temperatures(),
        flat_buffer_test_car_fleet(),
        flat_buffer_test_largest_rectangle_in_histogram(),
        flat_buffer_test_erase()
    );
}
