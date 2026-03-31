#include <stddef.h>

#define TRAITS_USING_NAMESPACE_CCC
#define DOUBLY_LINKED_LIST_USING_NAMESPACE_CCC

#include "checkers.h"
#include "doubly_linked_list.h"
#include "doubly_linked_list_utility.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

check_static_begin(doubly_linked_list_test_push_erase) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    Doubly_linked_list doubly_linked_list
        = doubly_linked_list_for(struct Val, e);
    check(
        CCC_doubly_linked_list_erase(NULL, &(struct Val){}.e, &allocator), NULL
    );
    check(
        CCC_doubly_linked_list_erase(&doubly_linked_list, NULL, &allocator),
        NULL
    );
    check(
        CCC_doubly_linked_list_erase(
            &doubly_linked_list, &(struct Val){}.e, NULL
        ),
        NULL
    );
    check(
        push_back(&doubly_linked_list, &(struct Val){}.e, &allocator) != NULL,
        true
    );
    check(validate(&doubly_linked_list), true);
    struct Val *const v = push_back(
        &doubly_linked_list, &(struct Val){.id = 1, .val = 1}.e, &allocator
    );
    check(v != NULL, true);
    check(validate(&doubly_linked_list), true);
    struct Val *v2 = CCC_doubly_linked_list_insert(
        &doubly_linked_list,
        &v->e,
        &(struct Val){.id = 2, .val = 2}.e,
        &allocator
    );
    check(v2 != NULL, true);
    check(validate(&doubly_linked_list), true);
    check(count(&doubly_linked_list).count, 3);
    check(
        CCC_doubly_linked_list_erase(&doubly_linked_list, &v2->e, &allocator)
            != NULL,
        true
    );
    check(validate(&doubly_linked_list), true);
    struct Val *const b = begin(&doubly_linked_list);
    check(b != NULL, true);
    check(
        CCC_doubly_linked_list_erase_range(
            &doubly_linked_list, &b->e, &v->e, &allocator
        ) != NULL,
        true
    );
    check(validate(&doubly_linked_list), true);
    check_end();
}

check_static_begin(doubly_linked_list_test_push_erase_range) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[9]){}),
    };
    enum : size_t {
        CAP = 9,
    };
    Doubly_linked_list doubly_linked_list = doubly_linked_list_from(
        e,
        allocator,
        (CCC_Destructor){},
        (struct Val[CAP]){
            {.val = 0},
            {.val = 1},
            {.val = 2},
            {.val = 3},
            {.val = 4},
            {.val = 5},
            {.val = 6},
            {.val = 7},
            {.val = 8},
        }
    );
    check(
        check_order(
            &doubly_linked_list, CAP, (int[CAP]){0, 1, 2, 3, 4, 5, 6, 7, 8}
        ),
        CHECK_PASS
    );
    check(
        CCC_doubly_linked_list_erase_range(
            NULL, &(struct Val){}.e, &(struct Val){}.e, &allocator
        ),
        NULL
    );
    check(
        CCC_doubly_linked_list_erase_range(
            &doubly_linked_list, NULL, &(struct Val){}.e, &allocator
        ),
        NULL
    );
    check(
        CCC_doubly_linked_list_erase_range(
            &doubly_linked_list, &(struct Val){}.e, &(struct Val){}.e, NULL
        ),
        NULL
    );
    struct Val *begin = begin(&doubly_linked_list);
    for (size_t i = 0; i < CAP / 3; ++i) {
        begin = next(&doubly_linked_list, &begin->e);
    }
    struct Val *end = reverse_begin(&doubly_linked_list);
    for (size_t i = CAP; i > CAP - 2; --i) {
        end = reverse_next(&doubly_linked_list, &end->e);
    }
    struct Val *const six = CCC_doubly_linked_list_erase_range(
        &doubly_linked_list, &begin->e, &end->e, &allocator
    );
    check(six != NULL, CCC_TRUE);
    check(six->val, 6);
    check(validate(&doubly_linked_list), true);
    check(
        check_order(&doubly_linked_list, 6, (int[6]){0, 1, 2, 6, 7, 8}),
        CHECK_PASS
    );
    struct Val *const one
        = next(&doubly_linked_list, begin(&doubly_linked_list));
    check(one != NULL, CCC_TRUE);
    check(one->val, 1);
    struct Val *iter = begin(&doubly_linked_list);
    check(iter != NULL, CCC_TRUE);
    iter = CCC_doubly_linked_list_erase_range(
        &doubly_linked_list, &iter->e, &six->e, &allocator
    );
    check(iter != NULL, CCC_TRUE);
    check(iter->val, 6);
    check(check_order(&doubly_linked_list, 3, (int[3]){6, 7, 8}), CHECK_PASS);
    check(
        CCC_doubly_linked_list_erase_range(
            &doubly_linked_list,
            begin(&doubly_linked_list),
            end(&doubly_linked_list),
            &allocator
        ),
        NULL
    );
    check(validate(&doubly_linked_list), true);
    check(is_empty(&doubly_linked_list), true);
    check_end();
}

check_static_begin(doubly_linked_list_test_pop_empty) {
    Doubly_linked_list doubly_linked_list
        = doubly_linked_list_for(struct Val, e);
    check(is_empty(&doubly_linked_list), true);
    check(
        doubly_linked_list_pop_front(&doubly_linked_list, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(doubly_linked_list_validate(&doubly_linked_list), true);
    check(
        doubly_linked_list_pop_back(&doubly_linked_list, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(doubly_linked_list_validate(&doubly_linked_list), true);
    check(doubly_linked_list_front(&doubly_linked_list), NULL);
    check(doubly_linked_list_back(&doubly_linked_list), NULL);
    check(is_empty(&doubly_linked_list), true);
    check_end();
}

check_static_begin(doubly_linked_list_test_push_pop_front) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[12]){}),
    };
    Doubly_linked_list doubly_linked_list = doubly_linked_list_from(
        e,
        allocator,
        (CCC_Destructor){},
        (struct Val[3]){
            {.val = 0},
            {.val = 1},
            {.val = 2},
        }
    );
    check(count(&doubly_linked_list).count, 3);
    struct Val *v = doubly_linked_list_front(&doubly_linked_list);
    check(v == NULL, false);
    check(v->val, 0);
    check(pop_front(&doubly_linked_list, &allocator), CCC_RESULT_OK);
    check(validate(&doubly_linked_list), true);
    v = doubly_linked_list_front(&doubly_linked_list);
    check(v == NULL, false);
    check(v->val, 1);
    check(pop_front(&doubly_linked_list, &allocator), CCC_RESULT_OK);
    v = doubly_linked_list_front(&doubly_linked_list);
    check(validate(&doubly_linked_list), true);
    check(v == NULL, false);
    check(v->val, 2);
    check(pop_front(&doubly_linked_list, &allocator), CCC_RESULT_OK);
    check(is_empty(&doubly_linked_list), true);
    check_end();
}

check_static_begin(doubly_linked_list_test_push_pop_back) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[12]){}),
    };
    Doubly_linked_list doubly_linked_list = doubly_linked_list_from(
        e,
        allocator,
        (CCC_Destructor){},
        (struct Val[3]){
            {.val = 0},
            {.val = 1},
            {.val = 2},
        }
    );
    check(count(&doubly_linked_list).count, 3);
    struct Val *v = doubly_linked_list_back(&doubly_linked_list);
    check(v == NULL, false);
    check(v->val, 2);
    check(pop_back(&doubly_linked_list, &allocator), CCC_RESULT_OK);
    check(validate(&doubly_linked_list), true);
    v = doubly_linked_list_back(&doubly_linked_list);
    check(v == NULL, false);
    check(v->val, 1);
    check(pop_back(&doubly_linked_list, &allocator), CCC_RESULT_OK);
    check(validate(&doubly_linked_list), true);
    v = doubly_linked_list_back(&doubly_linked_list);
    check(v == NULL, false);
    check(v->val, 0);
    check(pop_back(&doubly_linked_list, &allocator), CCC_RESULT_OK);
    check(is_empty(&doubly_linked_list), true);
    check_end();
}

check_static_begin(doubly_linked_list_test_push_pop_middle) {
    Doubly_linked_list doubly_linked_list
        = doubly_linked_list_for(struct Val, e);
    check(doubly_linked_list_extract(NULL, &(struct Val){}.e), NULL);
    check(doubly_linked_list_extract(&doubly_linked_list, NULL), NULL);
    struct Val vals[4] = {{.val = 0}, {.val = 1}, {.val = 2}, {.val = 3}};
    enum Check_result const t = push_list(
        &doubly_linked_list, UTIL_PUSH_BACK, 4, vals, &(CCC_Allocator){}
    );
    check(t, CHECK_PASS);
    check(extract(&doubly_linked_list, &vals[2].e) != NULL, true);
    check(validate(&doubly_linked_list), true);
    check(check_order(&doubly_linked_list, 3, (int[3]){0, 1, 3}), CHECK_PASS);
    (void)extract(&doubly_linked_list, &vals[1].e);
    check(validate(&doubly_linked_list), true);
    check(check_order(&doubly_linked_list, 2, (int[2]){0, 3}), CHECK_PASS);
    (void)extract(&doubly_linked_list, &vals[3].e);
    check(validate(&doubly_linked_list), true);
    check(check_order(&doubly_linked_list, 1, (int[1]){}), CHECK_PASS);
    (void)extract(&doubly_linked_list, &vals[0].e);
    check(validate(&doubly_linked_list), true);
    check(is_empty(&doubly_linked_list), true);
    check_end();
}

check_static_begin(doubly_linked_list_test_push_pop_middle_range) {
    Doubly_linked_list doubly_linked_list
        = doubly_linked_list_default(struct Val, e);
    struct Val vals[5]
        = {{.val = 0}, {.val = 1}, {.val = 2}, {.val = 3}, {.val = 4}};
    enum Check_result const t = push_list(
        &doubly_linked_list, UTIL_PUSH_BACK, 5, vals, &(CCC_Allocator){}
    );
    check(t, CHECK_PASS);
    (void)doubly_linked_list_extract_range(
        &doubly_linked_list, &vals[1].e, &vals[4].e
    );
    check(validate(&doubly_linked_list), true);
    check(count(&doubly_linked_list).count, 2);
    check(check_order(&doubly_linked_list, 2, (int[2]){0, 4}), CHECK_PASS);
    (void)doubly_linked_list_extract_range(
        &doubly_linked_list,
        &vals[0].e,
        doubly_linked_list_end(&doubly_linked_list)
    );
    check(validate(&doubly_linked_list), true);
    check(count(&doubly_linked_list).count, 0);
    check_end();
}

check_static_begin(doubly_linked_list_test_splice_two_lists) {
    Doubly_linked_list to_lose = doubly_linked_list_for(struct Val, e);
    struct Val to_lose_vals[5]
        = {{.val = 0}, {.val = 1}, {.val = 2}, {.val = 3}, {.val = 4}};
    enum Check_result t = push_list(
        &to_lose, UTIL_PUSH_BACK, 5, to_lose_vals, &(CCC_Allocator){}
    );
    check(t, CHECK_PASS);
    Doubly_linked_list to_gain = doubly_linked_list_for(struct Val, e);
    struct Val to_gain_vals[2] = {{.val = 0}, {.val = 1}};
    t = push_list(
        &to_gain, UTIL_PUSH_BACK, 2, to_gain_vals, &(CCC_Allocator){}
    );
    check(t, CHECK_PASS);
    check(check_order(&to_lose, 5, (int[5]){0, 1, 2, 3, 4}), CHECK_PASS);
    check(
        doubly_linked_list_splice(
            &to_gain,
            doubly_linked_list_end(&to_gain),
            &to_lose,
            &to_lose_vals[0].e
        ),
        CCC_RESULT_OK
    );
    check(validate(&to_gain), true);
    check(validate(&to_lose), true);
    check(count(&to_gain).count, 3);
    check(count(&to_lose).count, 4);
    check(check_order(&to_gain, 3, (int[3]){0, 1, 0}), CHECK_PASS);
    check(check_order(&to_lose, 4, (int[4]){1, 2, 3, 4}), CHECK_PASS);
    check(
        doubly_linked_list_splice_range(
            &to_gain,
            doubly_linked_list_end(&to_gain),
            &to_lose,
            doubly_linked_list_node_begin(&to_lose),
            doubly_linked_list_end(&to_lose)
        ),
        CCC_RESULT_OK
    );
    check(validate(&to_gain), true);
    check(validate(&to_lose), true);
    check(count(&to_gain).count, 7);
    check(count(&to_lose).count, 0);
    check(check_order(&to_gain, 7, (int[7]){0, 1, 0, 1, 2, 3, 4}), CHECK_PASS);
    check_end();
}

int
main(void) {
    return check_run(
        doubly_linked_list_test_push_erase(),
        doubly_linked_list_test_push_erase_range(),
        doubly_linked_list_test_pop_empty(),
        doubly_linked_list_test_push_pop_front(),
        doubly_linked_list_test_push_pop_back(),
        doubly_linked_list_test_push_pop_middle(),
        doubly_linked_list_test_push_pop_middle_range(),
        doubly_linked_list_test_splice_two_lists()
    );
}
