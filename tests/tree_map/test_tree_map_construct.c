#include <stdbool.h>
#include <stddef.h>

#include "checkers.h"
#include "tree_map.h"
#include "tree_map_utility.h"
#include "types.h"
#include "utility/stack_allocator.h"

static CCC_Tree_map
construct_empty(void) {
    CCC_Tree_map map = CCC_tree_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    return map;
}

check_static_begin(tree_map_test_empty) {
    CCC_Tree_map s = CCC_tree_map_for(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(CCC_tree_map_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_tree_map_contains(NULL, &(int){}), CCC_TRIBOOL_ERROR);
    check(CCC_tree_map_contains(&s, NULL), CCC_TRIBOOL_ERROR);
    check(CCC_tree_map_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(
        CCC_tree_map_clear(NULL, &(CCC_Destructor){}, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_tree_map_clear(&s, NULL, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_tree_map_clear(&s, &(CCC_Destructor){}, NULL),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(CCC_tree_map_is_empty(&s), true);
    check(CCC_tree_map_validate(NULL), CCC_TRIBOOL_ERROR);
    check_end();
}

/** If the user constructs a node style map from a helper function, the map
cannot have any self referential fields, such as nil or sentinel nodes. If
the map is initialized on the stack those self referential fields will become
invalidated after the constructing function ends. This leads to a dangling
reference to stack memory that no longer exists. Disastrous. The solution is
to never implement sentinels that refer to a memory address on the map struct
itself. */
check_static_begin(tree_map_test_construct) {
    struct Val push = {};
    CCC_Tree_map map = construct_empty();
    CCC_Entry entry
        = CCC_tree_map_insert_or_assign(&map, &push.elem, &(CCC_Allocator){});
    check(
        CCC_entry_status(CCC_tree_map_insert_or_assign_wrap(
            NULL, &(struct Val){}.elem, &(CCC_Allocator){}
        )),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            CCC_tree_map_insert_or_assign_wrap(&map, NULL, &(CCC_Allocator){})
        ),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(
        CCC_entry_status(
            CCC_tree_map_insert_or_assign_wrap(&map, &(struct Val){}.elem, NULL)
        ),
        CCC_ENTRY_ARGUMENT_ERROR
    );
    check(CCC_tree_map_validate(&map), true);
    check(CCC_entry_insert_error(&entry), false);
    check(CCC_entry_occupied(&entry), false);
    check(CCC_tree_map_count(&map).count, 1);
    check_end();
}

check_static_begin(tree_map_test_construct_from) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    CCC_Tree_map map = CCC_tree_map_from(
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
        CCC_entry_status(CCC_tree_map_swap_entry_wrap(
            &map, &(struct Val){.key = 3}.elem, &(struct Val){}.elem, &allocator
        )),
        CCC_ENTRY_INSERT_ERROR
    );
    check(
        CCC_entry_status(CCC_tree_map_try_insert_wrap(
            &map, &(struct Val){.key = 3}.elem, &allocator
        )),
        CCC_ENTRY_INSERT_ERROR
    );
    check(
        CCC_entry_status(CCC_tree_map_insert_or_assign_wrap(
            &map, &(struct Val){.key = 3}.elem, &allocator
        )),
        CCC_ENTRY_INSERT_ERROR
    );
    check(
        CCC_tree_map_insert_entry(
            CCC_tree_map_entry_wrap(&map, &(int){3}),
            &(struct Val){.key = 3}.elem,
            &allocator
        ),
        NULL
    );
    check(
        CCC_tree_map_or_insert(
            CCC_tree_map_entry_wrap(&map, &(int){3}),
            &(struct Val){.key = 3}.elem,
            &allocator
        ),
        NULL
    );
    check(CCC_tree_map_validate(&map), true);
    check(CCC_tree_map_count(&map).count, 3);
    check_end({
        (void)CCC_tree_map_clear(&map, &(CCC_Destructor){}, &allocator);
    });
}

check_static_begin(tree_map_test_construct_from_overwrite) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[3]){}),
    };
    CCC_Tree_map map = CCC_tree_map_from(
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
    check(CCC_tree_map_validate(&map), true);
    check(CCC_tree_map_count(&map).count, 2);
    struct Val const *const v = CCC_tree_map_reverse_begin(&map);
    check(v != NULL, true);
    check(v->key, 1);
    check(v->val, 2);
    check_end({
        (void)CCC_tree_map_clear(&map, &(CCC_Destructor){}, &allocator);
    });
}

check_static_begin(tree_map_test_construct_from_fail) {
    CCC_Tree_map map = CCC_tree_map_from(
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
    check(CCC_tree_map_validate(&map), true);
    check(CCC_tree_map_is_empty(&map), true);
    check_end({
        (void)CCC_tree_map_clear(&map, &(CCC_Destructor){}, &(CCC_Allocator){});
    });
}

int
main(void) {
    return check_run(
        tree_map_test_empty(),
        tree_map_test_construct(),
        tree_map_test_construct_from(),
        tree_map_test_construct_from_overwrite(),
        tree_map_test_construct_from_fail()
    );
}
