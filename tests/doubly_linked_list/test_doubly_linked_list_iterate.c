#include <stddef.h>

#define BITSET_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC
#define DOUBLY_LINKED_LIST_USING_NAMESPACE_CCC

#include "ccc/bitset.h"
#include "ccc/doubly_linked_list.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "checkers.h"
#include "doubly_linked_list_utility.h"
#include "utility/stack_allocator.h"

check_static_begin(doubly_linked_list_test_insert_iterate) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[9]){}),
    };
    enum : size_t {
        CAP = 9,
    };
    Doubly_linked_list list = doubly_linked_list_from(
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
    check(CCC_doubly_linked_list_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_doubly_linked_list_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_doubly_linked_list_begin(NULL), NULL);
    check(CCC_doubly_linked_list_node_begin(NULL), NULL);
    check(CCC_doubly_linked_list_reverse_begin(NULL), NULL);
    size_t const o_n_count = CCC_doubly_linked_list_count(&list).count;
    size_t count = 0;
    for (struct Val const *v = begin(&list); v != end(&list);
         v = next(&list, &v->e)) {
        ++count;
    }
    check(count, o_n_count);
    check_end();
}

check_static_begin(doubly_linked_list_test_is_sorted) {
    enum : size_t {
        CAP = 8
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[CAP]){}),
    };
    Doubly_linked_list list = doubly_linked_list_from(
        e,
        allocator,
        (CCC_Destructor){},
        (struct Val[CAP]){
            {.val = 0},
            {.val = 6},
            {.val = 4},
            {.val = 3},
            {.val = 2},
            {.val = 1},
            {.val = 5},
            {.val = 7},
        }
    );
    check(
        CCC_doubly_linked_list_is_sorted(
            NULL, CCC_ORDER_GREATER, &(CCC_Comparator){}
        ),
        CCC_TRIBOOL_ERROR
    );
    check(
        CCC_doubly_linked_list_is_sorted(
            &list, CCC_ORDER_EQUAL, &(CCC_Comparator){}
        ),
        CCC_TRIBOOL_ERROR
    );
    check(
        CCC_doubly_linked_list_is_sorted(&list, CCC_ORDER_GREATER, NULL),
        CCC_TRIBOOL_ERROR
    );
    check(
        CCC_doubly_linked_list_is_sorted(
            &list, CCC_ORDER_GREATER, &(CCC_Comparator){.compare = val_order}
        ),
        CCC_FALSE
    );
    check(
        CCC_doubly_linked_list_is_sorted(
            &list, CCC_ORDER_LESSER, &(CCC_Comparator){.compare = val_order}
        ),
        CCC_FALSE
    );
    check(
        CCC_doubly_linked_list_is_sorted(
            &doubly_linked_list_default(struct Val, e),
            CCC_ORDER_GREATER,
            &(CCC_Comparator){
                .compare = val_order,
            }
        ),
        CCC_TRUE
    );
    check_end();
}

static void
destroy_element(CCC_Arguments const arguments) {
    struct Val const *const i = arguments.type;
    Bitset *const is_destroyed_buffer = arguments.context;
    (void)bitset_set(is_destroyed_buffer, (size_t)i->val, CCC_TRUE);
}

check_static_begin(doubly_linked_list_test_clear_with_destructor) {
    enum : size_t {
        CAP = 8
    };
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[CAP]){}),
    };
    Doubly_linked_list list = doubly_linked_list_from(
        e,
        allocator,
        (CCC_Destructor){},
        (struct Val[CAP]){
            {.val = 0},
            {.val = 6},
            {.val = 4},
            {.val = 3},
            {.val = 2},
            {.val = 1},
            {.val = 5},
            {.val = 7},
        }
    );
    check(
        CCC_doubly_linked_list_clear(NULL, &(CCC_Destructor){}, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_doubly_linked_list_clear(&list, NULL, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_doubly_linked_list_clear(&list, &(CCC_Destructor){}, NULL),
        CCC_RESULT_ARGUMENT_ERROR
    );
    Bitset is_destroyed = bitset_with_storage(CAP, (Bit[CAP]){});
    check(
        CCC_doubly_linked_list_clear(
            &list,
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
    check(i, CAP);
    check_end();
}

int
main(void) {
    return check_run(
        doubly_linked_list_test_insert_iterate(),
        doubly_linked_list_test_is_sorted(),
        doubly_linked_list_test_clear_with_destructor(),
    );
}
