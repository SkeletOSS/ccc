#include <stddef.h>

#define TRAITS_USING_NAMESPACE_CCC
#define SINGLY_LINKED_LIST_USING_NAMESPACE_CCC

#include "checkers.h"
#include "singly_linked_list.h"
#include "singly_linked_list_utility.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

check_static_begin(singly_linked_list_test_pop_empty) {
    Singly_linked_list list = singly_linked_list_for(struct Val, e);
    check(is_empty(&list), true);
    check(
        singly_linked_list_push_front(
            NULL, &(struct Val){}.e, &(CCC_Allocator){}
        ),
        NULL
    );
    check(singly_linked_list_push_front(&list, NULL, &(CCC_Allocator){}), NULL);
    check(singly_linked_list_push_front(&list, &(struct Val){}.e, NULL), NULL);
    check(
        singly_linked_list_pop_front(&list, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(singly_linked_list_validate(&list), true);
    check(singly_linked_list_front(&list), NULL);
    check(is_empty(&list), true);
    check_end();
}

check_static_begin(singly_linked_list_test_push_erase) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    Singly_linked_list list = singly_linked_list_default(struct Val, e);
    check(
        CCC_singly_linked_list_erase(NULL, &(struct Val){}.e, &allocator), NULL
    );
    check(CCC_singly_linked_list_erase(&list, NULL, &allocator), NULL);
    check(CCC_singly_linked_list_erase(&list, &(struct Val){}.e, NULL), NULL);
    struct Val *const back = push_front(&list, &(struct Val){}.e, &allocator);
    check(back != NULL, true);
    check(validate(&list), true);
    struct Val *const v
        = push_front(&list, &(struct Val){.id = 1, .val = 1}.e, &allocator);
    check(v != NULL, true);
    check(validate(&list), true);
    check(count(&list).count, 2);
    struct Val *v2 = CCC_singly_linked_list_push_front(
        &list, &(struct Val){.id = 2, .val = 2}.e, &allocator
    );
    check(v2 != NULL, true);
    check(validate(&list), true);
    check(count(&list).count, 3);
    check(
        CCC_singly_linked_list_erase(&list, &v2->e, &allocator) != NULL, true
    );
    struct Val *const b = begin(&list);
    check(b != NULL, true);
    check(
        CCC_singly_linked_list_erase_range(&list, &b->e, &back->e, &allocator)
            != NULL,
        true
    );
    check(validate(&list), true);
    check(count(&list).count, 1);
    struct Val *front = push_front(&list, &(struct Val){}.e, &allocator);
    check(front, NULL);
    check_end();
}

check_static_begin(singly_linked_list_test_push_pop_three) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    Singly_linked_list singly_linked_list = singly_linked_list_from(
        e,
        allocator,
        (CCC_Destructor){},
        (struct Val[3]){
            {.val = 0},
            {.val = 1},
            {.val = 2},
        }
    );
    size_t const end = count(&singly_linked_list).count;
    for (size_t i = 0; i < end; ++i) {
        (void)pop_front(&singly_linked_list, &allocator);
        check(validate(&singly_linked_list), true);
    }
    check(is_empty(&singly_linked_list), true);
    check_end();
}

check_static_begin(singly_linked_list_test_push_erase_range) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[9]){}),
    };
    enum : size_t {
        CAP = 9,
    };
    Singly_linked_list list = singly_linked_list_from(
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
        check_order(&list, CAP, (int[CAP]){0, 1, 2, 3, 4, 5, 6, 7, 8}),
        CHECK_PASS
    );
    check(
        CCC_singly_linked_list_erase_range(
            NULL, &(struct Val){}.e, &(struct Val){}.e, &allocator
        ),
        NULL
    );
    check(
        CCC_singly_linked_list_erase_range(
            &list, NULL, &(struct Val){}.e, &allocator
        ),
        NULL
    );
    check(
        CCC_singly_linked_list_erase_range(
            &list, &(struct Val){}.e, &(struct Val){}.e, NULL
        ),
        NULL
    );
    struct Val *begin = begin(&list);
    struct Val *end = begin(&list);
    size_t i = 0;
    for (; i < CAP / 3; ++i) {
        begin = next(&list, &begin->e);
        end = next(&list, &end->e);
    }
    for (; i < CAP - 3; ++i) {
        end = next(&list, &end->e);
    }
    struct Val *const six = CCC_singly_linked_list_erase_range(
        &list, &begin->e, &end->e, &(CCC_Allocator){}
    );
    check(six != NULL, CCC_TRUE);
    check(six->val, 6);
    check(validate(&list), true);
    check(check_order(&list, 6, (int[6]){0, 1, 2, 6, 7, 8}), CHECK_PASS);
    struct Val *const one = next(&list, singly_linked_list_node_begin(&list));
    check(one != NULL, CCC_TRUE);
    check(one->val, 1);
    struct Val *iter = begin(&list);
    check(iter != NULL, CCC_TRUE);
    iter = CCC_singly_linked_list_erase_range(
        &list, &iter->e, &six->e, &allocator
    );
    check(iter != NULL, CCC_TRUE);
    check(iter->val, 6);
    check(check_order(&list, 3, (int[3]){6, 7, 8}), CHECK_PASS);
    check(
        CCC_singly_linked_list_erase_range(
            &list, singly_linked_list_node_begin(&list), end(&list), &allocator
        ),
        NULL
    );
    check(validate(&list), true);
    check(is_empty(&list), true);
    check_end();
}

check_static_begin(singly_linked_list_test_push_pop_middle_range) {
    Singly_linked_list list = singly_linked_list_default(struct Val, e);
    check(singly_linked_list_extract(NULL, &(struct Val){}.e), NULL);
    check(singly_linked_list_extract(&list, NULL), NULL);
    struct Val vals[5]
        = {{.val = 0}, {.val = 1}, {.val = 2}, {.val = 3}, {.val = 4}};
    enum Check_result const t = push_list(&list, 5, vals, &(CCC_Allocator){});
    check(t, CHECK_PASS);
    check(
        singly_linked_list_extract_range(
            NULL, &(struct Val){}.e, &(struct Val){}.e
        ),
        NULL
    );
    check(
        singly_linked_list_extract_range(&list, NULL, &(struct Val){}.e), NULL
    );
    (void)singly_linked_list_extract_range(&list, &vals[3].e, &vals[0].e);
    check(validate(&list), true);
    check(count(&list).count, 2);
    check(check_order(&list, 2, (int[2]){4, 0}), CHECK_PASS);
    (void)singly_linked_list_extract_range(&list, &vals[4].e, &vals[0].e);
    check(validate(&list), true);
    (void)singly_linked_list_extract_range(
        &list, singly_linked_list_node_begin(&list), end(&list)
    );
    check(validate(&list), true);
    check(count(&list).count, 0);
    check_end();
}

check_static_begin(singly_linked_list_test_push_extract_middle) {
    Singly_linked_list singly_linked_list
        = singly_linked_list_for(struct Val, e);
    struct Val vals[3] = {{.val = 0}, {.val = 1}, {.val = 2}};
    enum Check_result const t
        = push_list(&singly_linked_list, 3, vals, &(CCC_Allocator){});
    check(t, CHECK_PASS);
    check(check_order(&singly_linked_list, 3, (int[3]){2, 1, 0}), CHECK_PASS);
    struct Val *after_extract = extract(&singly_linked_list, &vals[1].e);
    check(validate(&singly_linked_list), true);
    check(after_extract == NULL, false);
    check(after_extract->val, 0);
    check(check_order(&singly_linked_list, 2, (int[2]){2, 0}), CHECK_PASS);
    after_extract = extract(&singly_linked_list, &vals[0].e);
    check(after_extract, end(&singly_linked_list));
    check(check_order(&singly_linked_list, 1, (int[1]){2}), CHECK_PASS);
    check(count(&singly_linked_list).count, 1);
    check_end();
}

check_static_begin(singly_linked_list_test_push_extract_range) {
    Singly_linked_list singly_linked_list
        = singly_linked_list_for(struct Val, e);
    struct Val vals[5]
        = {{.val = 0}, {.val = 1}, {.val = 2}, {.val = 3}, {.val = 4}};
    enum Check_result const t
        = push_list(&singly_linked_list, 5, vals, &(CCC_Allocator){});
    check(t, CHECK_PASS);
    check(
        check_order(&singly_linked_list, 5, (int[5]){4, 3, 2, 1, 0}), CHECK_PASS
    );
    struct Val *after_extract
        = extract_range(&singly_linked_list, &vals[3].e, &vals[1].e);
    check(count(&singly_linked_list).count, 3);
    check(validate(&singly_linked_list), true);
    check(after_extract == NULL, false);
    check(after_extract->val, 1);
    check(check_order(&singly_linked_list, 3, (int[3]){4, 1, 0}), CHECK_PASS);
    after_extract = extract_range(
        &singly_linked_list,
        singly_linked_list_node_begin(&singly_linked_list),
        singly_linked_list_end(&singly_linked_list)
    );
    check(after_extract, singly_linked_list_end(&singly_linked_list));
    check(is_empty(&singly_linked_list), true);
    check_end();
}

check_static_begin(singly_linked_list_test_splice_two_lists) {
    Singly_linked_list to_lose = singly_linked_list_for(struct Val, e);
    struct Val to_lose_vals[5]
        = {{.val = 0}, {.val = 1}, {.val = 2}, {.val = 3}, {.val = 4}};
    enum Check_result t
        = push_list(&to_lose, 5, to_lose_vals, &(CCC_Allocator){});
    check(t, CHECK_PASS);
    Singly_linked_list to_gain = singly_linked_list_for(struct Val, e);
    check(
        singly_linked_list_splice(
            NULL, singly_linked_list_end(&to_gain), &to_lose, &(struct Val){}.e
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        singly_linked_list_splice(
            &to_gain, singly_linked_list_end(&to_gain), NULL, &(struct Val){}.e
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        singly_linked_list_splice(
            &to_gain, singly_linked_list_end(&to_gain), &to_lose, NULL
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        singly_linked_list_splice_range(
            NULL,
            singly_linked_list_end(&to_gain),
            &to_lose,
            &(struct Val){}.e,
            &(struct Val){}.e
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        singly_linked_list_splice_range(
            &to_gain,
            singly_linked_list_end(&to_gain),
            NULL,
            &(struct Val){}.e,
            &(struct Val){}.e
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        singly_linked_list_splice_range(
            &to_gain,
            singly_linked_list_end(&to_gain),
            &to_lose,
            NULL,
            &(struct Val){}.e
        ),
        CCC_RESULT_ARGUMENT_ERROR
    );
    struct Val to_gain_vals[2] = {{.val = 0}, {.val = 1}};
    t = push_list(&to_gain, 2, to_gain_vals, &(CCC_Allocator){});
    check(t, CHECK_PASS);
    check(check_order(&to_lose, 5, (int[5]){4, 3, 2, 1, 0}), CHECK_PASS);
    check(check_order(&to_gain, 2, (int[2]){1, 0}), CHECK_PASS);
    check(
        splice(
            &to_gain,
            singly_linked_list_node_begin(&to_gain),
            &to_lose,
            singly_linked_list_node_begin(&to_lose)
        ),
        CCC_RESULT_OK
    );
    check(count(&to_gain).count, 3);
    check(count(&to_lose).count, 4);
    check(check_order(&to_lose, 4, (int[4]){3, 2, 1, 0}), CHECK_PASS);
    check(check_order(&to_gain, 3, (int[3]){1, 4, 0}), CHECK_PASS);
    check(
        splice_range(
            &to_gain,
            singly_linked_list_node_begin(&to_gain),
            &to_lose,
            singly_linked_list_node_begin(&to_lose),
            &to_lose_vals[0].e
        ),
        CCC_RESULT_OK
    );
    check(count(&to_gain).count, 6);
    check(count(&to_lose).count, 1);
    check(check_order(&to_gain, 6, (int[6]){1, 3, 2, 1, 4, 0}), CHECK_PASS);
    check(
        singly_linked_list_splice(
            &to_lose,
            singly_linked_list_end(&to_gain),
            &to_lose,
            singly_linked_list_node_begin(&to_lose)
        ),
        CCC_RESULT_OK
    );
    check(validate(&to_gain), true);
    check(validate(&to_lose), true);
    check_end();
}

int
main(void) {
    return check_run(
        singly_linked_list_test_pop_empty(),
        singly_linked_list_test_push_erase(),
        singly_linked_list_test_push_erase_range(),
        singly_linked_list_test_push_pop_three(),
        singly_linked_list_test_push_extract_middle(),
        singly_linked_list_test_push_pop_middle_range(),
        singly_linked_list_test_push_extract_range(),
        singly_linked_list_test_splice_two_lists()
    );
}
