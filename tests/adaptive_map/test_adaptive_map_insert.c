#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#define TRAITS_USING_NAMESPACE_CCC
#define ADAPTIVE_MAP_USING_NAMESPACE_CCC

#include "adaptive_map.h"
#include "adaptive_map_utility.h"
#include "checkers.h"
#include "traits.h"
#include "types.h"
#include "utility/stack_allocator.h"

static inline struct Val
adaptive_map_create(int const id, int const val) {
    return (struct Val){.key = id, .val = val};
}

static inline void
adaptive_map_modplus(CCC_Arguments const t) {
    ((struct Val *)t.type)->val++;
}

check_static_begin(adaptive_map_test_insert) {
    Adaptive_map om = adaptive_map_for(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    /* Nothing was there before so nothing is in the entry. */
    CCC_Entry ent = swap_entry(
        &om,
        &(struct Val){.key = 137, .val = 99}.elem,
        &(struct Val){}.elem,
        &(CCC_Allocator){}
    );
    check(occupied(&ent), false);
    check(unwrap(&ent), NULL);
    check(count(&om).count, 1);
    check_end();
}

check_static_begin(adaptive_map_test_insert_macros) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[10]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    struct Val const *ins = adaptive_map_or_insert_with(
        adaptive_map_entry_wrap(&om, &(int){2}),
        &allocator,
        (struct Val){.key = 2, .val = 0}
    );
    check(ins != NULL, true);
    check(validate(&om), true);
    check(count(&om).count, 1);
    ins = adaptive_map_insert_entry_with(
        adaptive_map_entry_wrap(&om, &(int){2}),
        &allocator,
        (struct Val){.key = 2, .val = 0}
    );
    check(validate(&om), true);
    check(ins != NULL, true);
    ins = adaptive_map_insert_entry_with(
        adaptive_map_entry_wrap(&om, &(int){9}),
        &allocator,
        (struct Val){.key = 9, .val = 1}
    );
    check(validate(&om), true);
    check(ins != NULL, true);
    ins = CCC_entry_unwrap(adaptive_map_insert_or_assign_with(
        &om, 3, &allocator, (struct Val){.val = 99}
    ));
    check(validate(&om), true);
    check(ins == NULL, false);
    check(validate(&om), true);
    check(ins->val, 99);
    check(count(&om).count, 3);
    ins = CCC_entry_unwrap(adaptive_map_insert_or_assign_with(
        &om, 3, &allocator, (struct Val){.val = 98}
    ));
    check(validate(&om), true);
    check(ins == NULL, false);
    check(ins->val, 98);
    check(count(&om).count, 3);
    ins = CCC_entry_unwrap(adaptive_map_try_insert_with(
        &om, 3, &allocator, (struct Val){.val = 100}
    ));
    check(ins == NULL, false);
    check(validate(&om), true);
    check(ins->val, 98);
    check(count(&om).count, 3);
    ins = CCC_entry_unwrap(adaptive_map_try_insert_with(
        &om, 4, &allocator, (struct Val){.val = 100}
    ));
    check(ins == NULL, false);
    check(validate(&om), true);
    check(ins->val, 100);
    check(count(&om).count, 4);
    check_end({ adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator); });
}

check_static_begin(adaptive_map_test_insert_overwrite) {
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    struct Val q = {.key = 137, .val = 99};
    CCC_Entry ent
        = swap_entry(&om, &q.elem, &(struct Val){}.elem, &(CCC_Allocator){});
    check(occupied(&ent), false);
    check(unwrap(&ent), NULL);

    struct Val const *v = unwrap(adaptive_map_entry_wrap(&om, &q.key));
    check(v != NULL, true);
    check(v->val, 99);

    /* Now the second insertion will take place and the old occupying value
       will be written into our struct we used to make the query. */
    struct Val r = (struct Val){.key = 137, .val = 100};

    /* The contents of q are now in the table. */
    CCC_Entry old_ent
        = swap_entry(&om, &r.elem, &(struct Val){}.elem, &(CCC_Allocator){});
    check(occupied(&old_ent), true);

    /* The old contents are now in q and the entry is in the table. */
    v = unwrap(&old_ent);
    check(v != NULL, true);
    check(v->val, 99);
    check(r.val, 99);
    v = unwrap(adaptive_map_entry_wrap(&om, &r.key));
    check(v != NULL, true);
    check(v->val, 100);
    check_end();
}

check_static_begin(adaptive_map_test_insert_then_bad_ideas) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[5]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    struct Val q = {.key = 137, .val = 99};
    CCC_Entry ent = swap_entry(&om, &q.elem, &(struct Val){}.elem, &allocator);
    check(occupied(&ent), false);
    check(unwrap(&ent), NULL);
    struct Val const *v = unwrap(adaptive_map_entry_wrap(&om, &q.key));
    check(v != NULL, true);
    check(v->val, 99);

    struct Val r = (struct Val){.key = 137, .val = 100};

    ent = swap_entry(&om, &r.elem, &(struct Val){}.elem, &allocator);
    check(occupied(&ent), true);
    v = unwrap(&ent);
    check(v != NULL, true);
    check(v->val, 99);
    check(r.val, 99);
    r.val -= 9;

    v = get_key_value(&om, &q.key);
    check(v != NULL, true);
    check(v->val, 100);
    check(r.val, 90);
    check_end();
}

check_static_begin(adaptive_map_test_entry_api_functional) {
    /* Over allocate size now because we don't want to worry about resizing. */
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[200]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    size_t const size = 200;

    /* Test entry or insert with for all even values. Default should be
       inserted. All entries are hashed to last digit so many spread out
       collisions. */
    struct Val def = {};
    for (size_t i = 0; i < size / 2; i += 2) {
        def.key = (int)i;
        def.val = (int)i;
        struct Val const *const d = or_insert(
            adaptive_map_entry_wrap(&om, &def.key), &def.elem, &allocator
        );
        check((d != NULL), true);
        check(d->key, i);
        check(d->val, i);
    }
    check(count(&om).count, (size / 2) / 2);
    /* The default insertion should not occur every other element. */
    for (size_t i = 0; i < size / 2; ++i) {
        def.key = (int)i;
        def.val = (int)i;
        struct Val const *const d = or_insert(
            adaptive_map_and_modify_with(
                adaptive_map_entry_wrap(&om, &def.key),
                struct Val,
                { T->val++; }
            ),
            &def.elem,
            &allocator
        );
        /* All values in the array should be odd now */
        check((d != NULL), true);
        check(d->key, i);
        if (i % 2) {
            check(d->val, i);
        } else {
            check(d->val, i + 1);
        }
        check(d->val % 2, true);
    }
    check(count(&om).count, (size / 2));
    /* More simply modifications don't require the and modify function. All
       should be switched back to even now. */
    for (size_t i = 0; i < size / 2; ++i) {
        def.key = (int)i;
        def.val = (int)i;
        struct Val *const in = or_insert(
            adaptive_map_entry_wrap(&om, &def.key), &def.elem, &allocator
        );
        in->val++;
        /* All values in the array should be odd now */
        check((in->val % 2 == 0), true);
    }
    check(count(&om).count, (size / 2));
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

check_static_begin(adaptive_map_test_insert_via_entry) {
    /* Over allocate size now because we don't want to worry about resizing. */
    size_t const size = 200;
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[200]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    /* Test entry or insert with for all even values. Default should be
       inserted. All entries are hashed to last digit so many spread out
       collisions. */
    struct Val def = {};
    for (size_t i = 0; i < size / 2; i += 2) {
        def.key = (int)i;
        def.val = (int)i;
        struct Val const *const d = insert_entry(
            adaptive_map_entry_wrap(&om, &def.key), &def.elem, &allocator
        );
        check((d != NULL), true);
        check(d->key, i);
        check(d->val, i);
    }
    check(count(&om).count, (size / 2) / 2);
    /* The default insertion should not occur every other element. */
    for (size_t i = 0; i < size / 2; ++i) {
        def.key = (int)i;
        def.val = (int)i + 1;
        struct Val const *const d = insert_entry(
            adaptive_map_entry_wrap(&om, &def.key), &def.elem, &allocator
        );
        /* All values in the array should be odd now */
        check((d != NULL), true);
        check(d->val, i + 1);
        if (i % 2) {
            check(d->val % 2 == 0, true);
        } else {
            check(d->val % 2, true);
        }
    }
    check(count(&om).count, (size / 2));
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

check_static_begin(adaptive_map_test_insert_via_entry_macros) {
    /* Over allocate size now because we don't want to worry about resizing. */
    size_t const size = 200;
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[200]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    /* Test entry or insert with for all even values. Default should be
       inserted. All entries are hashed to last digit so many spread out
       collisions. */
    for (size_t i = 0; i < size / 2; i += 2) {
        struct Val const *const d = insert_entry(
            adaptive_map_entry_wrap(&om, &i),
            &(struct Val){
                .key = (int)i,
                .val = (int)i,
            }
                 .elem,
            &allocator
        );
        check((d != NULL), true);
        check(d->key, i);
        check(d->val, i);
    }
    check(count(&om).count, (size / 2) / 2);
    /* The default insertion should not occur every other element. */
    for (size_t i = 0; i < size / 2; ++i) {
        struct Val const *const d = insert_entry(
            adaptive_map_entry_wrap(&om, &i),
            &(struct Val){
                .key = (int)i,
                .val = (int)i + 1,
            }
                 .elem,
            &allocator
        );
        /* All values in the array should be odd now */
        check((d != NULL), true);
        check(d->val, i + 1);
        if (i % 2) {
            check(d->val % 2 == 0, true);
        } else {
            check(d->val % 2, true);
        }
    }
    check(count(&om).count, (size / 2));
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

check_static_begin(adaptive_map_test_entry_api_macros) {
    /* Over allocate size now because we don't want to worry about resizing. */
    int const size = 200;
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[200]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    /* Test entry or insert with for all even values. Default should be
       inserted. All entries are hashed to last digit so many spread out
       collisions. */
    for (int i = 0; i < size / 2; i += 2) {
        /* The macros support functions that will only execute if the or
           insert branch executes. */
        struct Val const *const d = adaptive_map_or_insert_with(
            adaptive_map_entry_wrap(&om, &i),
            &allocator,
            adaptive_map_create(i, i)
        );
        check((d != NULL), true);
        check(d->key, i);
        check(d->val, i);
    }
    check(count(&om).count, (size / 2) / 2);
    /* The default insertion should not occur every other element. */
    for (int i = 0; i < size / 2; ++i) {
        struct Val const *const d = adaptive_map_or_insert_with(
            and_modify(
                adaptive_map_entry_wrap(&om, &i),
                &(CCC_Modifier){
                    .modify = adaptive_map_modplus,
                }
            ),
            &allocator,
            adaptive_map_create(i, i)
        );
        /* All values in the array should be odd now */
        check((d != NULL), true);
        check(d->key, i);
        if (i % 2) {
            check(d->val, i);
        } else {
            check(d->val, i + 1);
        }
        check(d->val % 2, true);
    }
    check(count(&om).count, (size / 2));
    /* More simply modifications don't require the and modify function. All
       should be switched back to even now. */
    for (int i = 0; i < size / 2; ++i) {
        struct Val *v = adaptive_map_or_insert_with(
            adaptive_map_entry_wrap(&om, &i), &allocator, (struct Val){}
        );
        check(v != NULL, true);
        v->val++;
        /* All values in the array should be odd now */
        check(v->val % 2 == 0, true);
    }
    check(count(&om).count, (size / 2));
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

check_static_begin(adaptive_map_test_two_sum) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[10]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    int const addends[10] = {1, 3, -980, 6, 7, 13, 44, 32, 995, -1};
    int const target = 15;
    int solution_indices[2] = {-1, -1};
    for (size_t i = 0; i < (size_t)(sizeof(addends) / sizeof(addends[0]));
         ++i) {
        struct Val const *const other_addend
            = get_key_value(&om, &(int){target - addends[i]});
        if (other_addend) {
            solution_indices[0] = (int)i;
            solution_indices[1] = other_addend->val;
            break;
        }
        CCC_Entry const e = insert_or_assign(
            &om,
            &(struct Val){.key = addends[i], .val = (int)i}.elem,
            &allocator
        );
        check(insert_error(&e), false);
    }
    check(solution_indices[0], 8);
    check(solution_indices[1], 2);
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

check_static_begin(adaptive_map_test_insert_and_find) {
    int const size = 10;
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[100]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );

    for (int i = 0; i < size; i += 2) {
        CCC_Entry e = try_insert(
            &om, &(struct Val){.key = i, .val = i}.elem, &allocator
        );
        check(occupied(&e), false);
        check(validate(&om), true);
        e = try_insert(&om, &(struct Val){.key = i, .val = i}.elem, &allocator);
        check(occupied(&e), true);
        check(validate(&om), true);
        struct Val const *const v = unwrap(&e);
        check(v == NULL, false);
        check(v->key, i);
        check(v->val, i);
    }
    for (int i = 0; i < size; i += 2) {
        check(contains(&om, &i), true);
        check(occupied(adaptive_map_entry_wrap(&om, &i)), true);
        check(validate(&om), true);
    }
    for (int i = 1; i < size; i += 2) {
        check(contains(&om, &i), false);
        check(occupied(adaptive_map_entry_wrap(&om, &i)), false);
        check(validate(&om), true);
    }
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

check_static_begin(adaptive_map_test_insert_shuffle) {
    size_t const size = 50;
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[50]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    check(size > 1, true);
    int const prime = 53;
    check(insert_shuffled(&om, size, prime, &allocator), CHECK_PASS);
    int sorted_check[50];
    check(inorder_fill(sorted_check, size, &om), CHECK_PASS);
    for (size_t i = 1; i < size; ++i) {
        check(sorted_check[i - 1] <= sorted_check[i], true);
    }
    check_end();
}

check_static_begin(adaptive_map_test_insert_weak_srand) {
    int const num_nodes = 100;
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[100]){}),
    };
    Adaptive_map om = adaptive_map_default(
        struct Val, elem, key, (CCC_Key_comparator){.compare = id_order}
    );
    srand((unsigned)time(NULL)); /* NOLINT */
    for (int i = 0; i < num_nodes; ++i) {
        CCC_Entry const e = swap_entry(
            &om,
            &(struct Val){
                .key = (int)rand() /* NOLINT */,
                .val = i,
            }
                 .elem,
            &(struct Val){}.elem,
            &allocator
        );
        check(insert_error(&e), false);
        check(validate(&om), true);
    }
    check(count(&om).count, (size_t)num_nodes);
    check_end(adaptive_map_clear(&om, &(CCC_Destructor){}, &allocator););
}

int
main(void) {
    return check_run(
        adaptive_map_test_insert(),
        adaptive_map_test_insert_macros(),
        adaptive_map_test_insert_and_find(),
        adaptive_map_test_insert_overwrite(),
        adaptive_map_test_insert_then_bad_ideas(),
        adaptive_map_test_insert_via_entry(),
        adaptive_map_test_insert_via_entry_macros(),
        adaptive_map_test_entry_api_functional(),
        adaptive_map_test_entry_api_macros(),
        adaptive_map_test_two_sum(),
        adaptive_map_test_insert_weak_srand(),
        adaptive_map_test_insert_shuffle()
    );
}
