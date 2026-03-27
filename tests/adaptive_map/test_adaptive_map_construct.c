#include <stdbool.h>
#include <stddef.h>

#define TRAITS_USING_NAMESPACE_CCC

#include "adaptive_map.h"
#include "adaptive_map_utility.h"
#include "checkers.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

static CCC_Adaptive_map
construct_empty(void) {
    CCC_Adaptive_map map = CCC_adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    return map;
}

check_static_begin(adaptive_map_test_empty) {
    CCC_Adaptive_map s = CCC_adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(CCC_adaptive_map_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_adaptive_map_contains(NULL, &(int){}), CCC_TRIBOOL_ERROR);
    check(CCC_adaptive_map_contains(&s, NULL), CCC_TRIBOOL_ERROR);
    check(CCC_adaptive_map_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(
        CCC_adaptive_map_clear(NULL, &(CCC_Destructor){}, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_adaptive_map_clear(&s, NULL, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_adaptive_map_clear(&s, &(CCC_Destructor){}, NULL),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(CCC_adaptive_map_validate(NULL), CCC_TRIBOOL_ERROR);
    check(is_empty(&s), true);
    check_end();
}

/** If the user constructs a node style map from a helper function, the map
cannot have any self referential fields, such as nil or sentinel nodes. If
the map is initialized on the stack those self referential fields will become
invalidated after the constructing function ends. This leads to a dangling
reference to stack memory that no longer exists. Disastrous. The solution is
to never implement sentinels that refer to a memory address on the map struct
itself. */
check_static_begin(adaptive_map_test_construct) {
    struct Val push = {};
    CCC_Adaptive_map map = construct_empty();
    CCC_Entry entry = CCC_adaptive_map_insert_or_assign(
        &map, &push.elem, &(CCC_Allocator){}
    );
    check(
        CCC_entry_status(CCC_adaptive_map_insert_or_assign_wrap(
            NULL, &(struct Val){}.elem, &(CCC_Allocator){}
        )),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(CCC_adaptive_map_insert_or_assign_wrap(
            &map, NULL, &(CCC_Allocator){}
        )),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(CCC_adaptive_map_insert_or_assign_wrap(
            &map, &(struct Val){}.elem, NULL
        )),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(CCC_adaptive_map_validate(&map), true);
    check(CCC_entry_insert_error(&entry), false);
    check(CCC_entry_occupied(&entry), false);
    check(CCC_adaptive_map_count(&map).count, 1);
    check_end();
}

check_static_begin(adaptive_map_test_construct_from) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    CCC_Adaptive_map map = CCC_adaptive_map_from(
        elem,
        key,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        (CCC_Destructor){},
        (struct Val[]){
            {.key = 0, .val = 0},
            {.key = 1, .val = 1},
            {.key = 2, .val = 2},
        }
    );
    check(
        CCC_entry_status(CCC_adaptive_map_swap_entry_wrap(
            &map, &(struct Val){.key = 3}.elem, &(struct Val){}.elem, &allocator
        )),
        CCC_ENTRY_INSERT_ERROR
    );
    check(
        CCC_entry_status(CCC_adaptive_map_try_insert_wrap(
            &map, &(struct Val){.key = 3}.elem, &allocator
        )),
        CCC_ENTRY_INSERT_ERROR
    );
    check(
        CCC_entry_status(CCC_adaptive_map_insert_or_assign_wrap(
            &map, &(struct Val){.key = 3}.elem, &allocator
        )),
        CCC_ENTRY_INSERT_ERROR
    );
    check(
        CCC_adaptive_map_insert_entry(
            CCC_adaptive_map_entry_wrap(&map, &(int){3}),
            &(struct Val){.key = 3}.elem,
            &allocator
        ),
        NULL
    );
    check(
        CCC_adaptive_map_or_insert(
            CCC_adaptive_map_entry_wrap(&map, &(int){3}),
            &(struct Val){.key = 3}.elem,
            &allocator
        ),
        NULL
    );
    check(CCC_adaptive_map_validate(&map), true);
    check(CCC_adaptive_map_count(&map).count, 3);
    check_end({
        (void)CCC_adaptive_map_clear(&map, &(CCC_Destructor){}, &allocator);
    });
}

check_static_begin(adaptive_map_test_construct_from_overwrite) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    CCC_Adaptive_map map = CCC_adaptive_map_from(
        elem,
        key,
        (CCC_Key_comparator){.compare = id_order},
        allocator,
        (CCC_Destructor){},
        (struct Val[]){
            {.key = 0, .val = 0},
            {.key = 1, .val = 1},
            {.key = 1, .val = 2},
        }
    );
    check(CCC_adaptive_map_validate(&map), true);
    check(CCC_adaptive_map_count(&map).count, 2);
    struct Val const *const v = CCC_adaptive_map_reverse_begin(&map);
    check(v != NULL, true);
    check(v->key, 1);
    check(v->val, 2);
    check_end({
        (void)CCC_adaptive_map_clear(&map, &(CCC_Destructor){}, &allocator);
    });
}

check_static_begin(adaptive_map_test_construct_from_fail) {
    CCC_Adaptive_map map = CCC_adaptive_map_from(
        elem,
        key,
        (CCC_Key_comparator){.compare = id_order},
        (CCC_Allocator){},
        (CCC_Destructor){},
        (struct Val[]){
            {.key = 0, .val = 0},
            {.key = 1, .val = 1},
            {.key = 2, .val = 2},
        }
    );
    check(CCC_adaptive_map_validate(&map), true);
    check(CCC_adaptive_map_is_empty(&map), true);
    check_end({
        (void)CCC_adaptive_map_clear(
            &map, &(CCC_Destructor){}, &(CCC_Allocator){}
        );
    });
}

int
main(void) {
    return check_run(
        adaptive_map_test_empty(),
        adaptive_map_test_construct(),
        adaptive_map_test_construct_from(),
        adaptive_map_test_construct_from_overwrite(),
        adaptive_map_test_construct_from_fail()
    );
}
