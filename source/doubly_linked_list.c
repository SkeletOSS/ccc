/** Copyright 2025 Alexander G. Lopez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */
/* Citation:
[1] See the sort methods for citations and change lists regarding the pintOS
educational operating system natural merge sort algorithm used for linked lists.
Code in the pintOS source is at  `source/lib/kernel.list.c`, but this may change
if they refactor. */
/** C23 provided headers. */
#include <stddef.h>

/** CCC provided headers. */
#include "ccc/configuration.h"
#include "ccc/doubly_linked_list.h"
#include "ccc/private/private_doubly_linked_list.h"
#include "ccc/sort.h"
#include "ccc/types.h"

/*===========================   Prototypes    ===============================*/

static void push_back(
    struct CCC_Doubly_linked_list *, struct CCC_Doubly_linked_list_node *
);
static void push_front(
    struct CCC_Doubly_linked_list *, struct CCC_Doubly_linked_list_node *
);
static void remove_node(
    struct CCC_Doubly_linked_list *, struct CCC_Doubly_linked_list_node *
);
static void insert_node(
    struct CCC_Doubly_linked_list *,
    struct CCC_Doubly_linked_list_node *,
    struct CCC_Doubly_linked_list_node *
);
static void *struct_base(
    struct CCC_Doubly_linked_list const *,
    struct CCC_Doubly_linked_list_node const *
);
static void erase_inclusive_range(
    struct CCC_Doubly_linked_list const *,
    struct CCC_Doubly_linked_list_node *,
    struct CCC_Doubly_linked_list_node *,
    CCC_Allocator const *
);
static size_t
len(struct CCC_Doubly_linked_list_node const *,
    struct CCC_Doubly_linked_list_node const *);
static struct CCC_Doubly_linked_list_node *
type_intruder_in(CCC_Doubly_linked_list const *, void const *);
static struct CCC_Doubly_linked_list_node *first_out_of_order(
    struct CCC_Doubly_linked_list const *,
    struct CCC_Doubly_linked_list_node *,
    CCC_Order,
    CCC_Comparator const *
);
static struct CCC_Doubly_linked_list_node *merge(
    struct CCC_Doubly_linked_list *,
    struct CCC_Doubly_linked_list_node *,
    struct CCC_Doubly_linked_list_node *,
    struct CCC_Doubly_linked_list_node *,
    CCC_Order,
    CCC_Comparator const *
);
static CCC_Order get_order(
    struct CCC_Doubly_linked_list const *,
    struct CCC_Doubly_linked_list_node const *,
    struct CCC_Doubly_linked_list_node const *,
    CCC_Comparator const *
);

/*===========================     Interface   ===============================*/

void *
CCC_doubly_linked_list_push_front(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *const allocator
) {
    if (!list || !type_intruder || !allocator) {
        return NULL;
    }
    if (allocator->allocate) {
        void *const copy = allocator->allocate((CCC_Allocator_arguments){
            .input = NULL,
            .bytes = list->sizeof_type,
            .context = allocator->context,
        });
        if (!copy) {
            return NULL;
        }
        memcpy(copy, struct_base(list, type_intruder), list->sizeof_type);
        type_intruder = type_intruder_in(list, copy);
    }

    push_front(list, type_intruder);
    return struct_base(list, type_intruder);
}

void *
CCC_doubly_linked_list_push_back(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *const allocator
) {
    if (!list || !type_intruder || !allocator) {
        return NULL;
    }
    if (allocator->allocate) {
        void *const node = allocator->allocate((CCC_Allocator_arguments){
            .input = NULL,
            .bytes = list->sizeof_type,
            .context = allocator->context,
        });
        if (!node) {
            return NULL;
        }
        (void)memcpy(node, struct_base(list, type_intruder), list->sizeof_type);
        type_intruder = type_intruder_in(list, node);
    }
    push_back(list, type_intruder);
    return struct_base(list, type_intruder);
}

void *
CCC_doubly_linked_list_front(CCC_Doubly_linked_list const *list) {
    if (!list) {
        return NULL;
    }
    return struct_base(list, list->head);
}

void *
CCC_doubly_linked_list_back(CCC_Doubly_linked_list const *list) {
    if (!list) {
        return NULL;
    }
    return struct_base(list, list->tail);
}

CCC_Result
CCC_doubly_linked_list_pop_front(
    CCC_Doubly_linked_list *const list, CCC_Allocator const *const allocator
) {
    if (!list || !list->head || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    struct CCC_Doubly_linked_list_node *const r = list->head;
    remove_node(list, r);
    if (allocator->allocate) {
        assert(r);
        (void)allocator->allocate((CCC_Allocator_arguments){
            .input = struct_base(list, r),
            .bytes = 0,
            .context = allocator->context,
        });
    }
    return CCC_RESULT_OK;
}

CCC_Result
CCC_doubly_linked_list_pop_back(
    CCC_Doubly_linked_list *const list, CCC_Allocator const *const allocator
) {
    if (!list || !list->head || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    struct CCC_Doubly_linked_list_node *const r = list->tail;
    remove_node(list, r);
    if (allocator->allocate) {
        (void)allocator->allocate((CCC_Allocator_arguments){
            .input = struct_base(list, r),
            .bytes = 0,
            .context = allocator->context,
        });
    }
    return CCC_RESULT_OK;
}

void *
CCC_doubly_linked_list_insert(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *const position,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *const allocator
) {
    if (!list || !type_intruder || !allocator) {
        return NULL;
    }
    if (allocator->allocate) {
        void *const node = allocator->allocate((CCC_Allocator_arguments){
            .input = NULL,
            .bytes = list->sizeof_type,
            .context = allocator->context,
        });
        if (!node) {
            return NULL;
        }
        (void)memcpy(node, struct_base(list, type_intruder), list->sizeof_type);
        type_intruder = type_intruder_in(list, node);
    }
    insert_node(list, position, type_intruder);
    return struct_base(list, type_intruder);
}

void *
CCC_doubly_linked_list_erase(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *const allocator
) {
    if (!list || !type_intruder || !allocator || !list->head) {
        return NULL;
    }
    void *const ret = struct_base(list, type_intruder->next);
    remove_node(list, type_intruder);
    if (allocator->allocate) {
        (void)allocator->allocate((CCC_Allocator_arguments){
            .input = struct_base(list, type_intruder),
            .bytes = 0,
            .context = allocator->context,
        });
    }
    return ret;
}

void *
CCC_doubly_linked_list_erase_range(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *const type_intruder_begin,
    CCC_Doubly_linked_list_node *type_intruder_end,
    CCC_Allocator const *const allocator
) {
    if (!list || !allocator || !list->head || !type_intruder_begin
        || type_intruder_begin == type_intruder_end) {
        return NULL;
    }
    if (type_intruder_end) {
        type_intruder_end = type_intruder_end->previous;
    }
    if (type_intruder_begin == type_intruder_end) {
        return CCC_doubly_linked_list_erase(
            list, type_intruder_begin, allocator
        );
    }
    CCC_Doubly_linked_list_node *const previous = type_intruder_begin->previous;
    CCC_Doubly_linked_list_node *const next
        = type_intruder_end ? type_intruder_end->next : NULL;

    if (previous) {
        previous->next = next;
    } else {
        list->head = next;
    }

    if (next) {
        next->previous = previous;
    } else {
        list->tail = previous;
    }

    type_intruder_begin->previous = NULL;
    if (type_intruder_end) {
        type_intruder_end->next = NULL;
    }
    erase_inclusive_range(
        list, type_intruder_begin, type_intruder_end, allocator
    );

    return struct_base(list, next);
}

CCC_Doubly_linked_list_node *
CCC_doubly_linked_list_node_begin(CCC_Doubly_linked_list const *const list) {
    return list ? list->head : NULL;
}

void *
CCC_doubly_linked_list_extract(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *type_intruder
) {
    if (!list || !type_intruder) {
        return NULL;
    }
    remove_node(list, type_intruder);
    return struct_base(list, type_intruder);
}

void *
CCC_doubly_linked_list_extract_range(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *type_intruder_begin,
    CCC_Doubly_linked_list_node *type_intruder_end
) {
    if (!list || !list->head || !type_intruder_begin
        || type_intruder_begin == type_intruder_end) {
        return NULL;
    }
    /* We extract exclusive ranges [start, end). Step back one. */
    if (type_intruder_end) {
        type_intruder_end = type_intruder_end->previous;
    }
    if (type_intruder_begin == type_intruder_end) {
        remove_node(list, type_intruder_begin);
        return struct_base(list, type_intruder_begin);
    }

    CCC_Doubly_linked_list_node *const previous = type_intruder_begin->previous;
    CCC_Doubly_linked_list_node *const next
        = type_intruder_end ? type_intruder_end->next : NULL;

    if (previous) {
        previous->next = next;
    } else {
        list->head = next;
    }

    if (next) {
        next->previous = previous;
    } else {
        list->tail = previous;
    }

    type_intruder_begin->previous = NULL;
    if (type_intruder_end) {
        type_intruder_end->next = NULL;
    }
    return struct_base(list, next);
}

CCC_Result
CCC_doubly_linked_list_splice(
    CCC_Doubly_linked_list *const position_doubly_linked_list,
    CCC_Doubly_linked_list_node *position,
    CCC_Doubly_linked_list *const to_cut_doubly_linked_list,
    CCC_Doubly_linked_list_node *to_cut
) {
    if (!position_doubly_linked_list || !to_cut_doubly_linked_list || !to_cut) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if ((to_cut_doubly_linked_list == position_doubly_linked_list)
        && (to_cut == position || (to_cut && to_cut->next == position))) {
        return CCC_RESULT_OK;
    }
    remove_node(to_cut_doubly_linked_list, to_cut);
    insert_node(position_doubly_linked_list, position, to_cut);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_doubly_linked_list_splice_range(
    CCC_Doubly_linked_list *const position_doubly_linked_list,
    CCC_Doubly_linked_list_node *const type_intruder_position,
    CCC_Doubly_linked_list *const to_cut_doubly_linked_list,
    CCC_Doubly_linked_list_node *const type_intruder_to_cut_begin,
    CCC_Doubly_linked_list_node *const type_intruder_to_cut_exclusive_end
) {
    if (!position_doubly_linked_list || !to_cut_doubly_linked_list
        || !type_intruder_to_cut_begin
        || type_intruder_to_cut_begin == type_intruder_to_cut_exclusive_end) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    CCC_Doubly_linked_list_node *const to_cut_inclusive_end
        = type_intruder_to_cut_exclusive_end
            ? type_intruder_to_cut_exclusive_end->previous
            : to_cut_doubly_linked_list->tail;
    if (type_intruder_to_cut_begin == to_cut_inclusive_end) {
        return CCC_doubly_linked_list_splice(
            position_doubly_linked_list,
            type_intruder_position,
            to_cut_doubly_linked_list,
            type_intruder_to_cut_begin
        );
    }

    CCC_Doubly_linked_list_node *const to_cut_previous
        = type_intruder_to_cut_begin->previous;

    if (to_cut_previous) {
        to_cut_previous->next = type_intruder_to_cut_exclusive_end;
    } else {
        to_cut_doubly_linked_list->head = type_intruder_to_cut_exclusive_end;
    }

    if (type_intruder_to_cut_exclusive_end) {
        type_intruder_to_cut_exclusive_end->previous = to_cut_previous;
    } else {
        to_cut_doubly_linked_list->tail = to_cut_previous;
    }
    CCC_Doubly_linked_list_node *const position_previous
        = type_intruder_position ? type_intruder_position->previous
                                 : position_doubly_linked_list->tail;

    if (position_previous == position_doubly_linked_list->tail) {
        position_doubly_linked_list->tail = to_cut_inclusive_end;
    }
    if (position_previous) {
        position_previous->next = type_intruder_to_cut_begin;
    } else {
        position_doubly_linked_list->head = type_intruder_to_cut_begin;
    }

    type_intruder_to_cut_begin->previous = position_previous;

    if (to_cut_inclusive_end) {
        to_cut_inclusive_end->next = type_intruder_position;
    }

    if (type_intruder_position) {
        type_intruder_position->previous = to_cut_inclusive_end;
    }

    return CCC_RESULT_OK;
}

void *
CCC_doubly_linked_list_begin(CCC_Doubly_linked_list const *const list) {
    if (!list || list->head == NULL) {
        return NULL;
    }
    return struct_base(list, list->head);
}

void *
CCC_doubly_linked_list_reverse_begin(CCC_Doubly_linked_list const *const list) {
    if (!list || list->tail == NULL) {
        return NULL;
    }
    return struct_base(list, list->tail);
}

void *
CCC_doubly_linked_list_end(CCC_Doubly_linked_list const *const) {
    return NULL;
}

void *
CCC_doubly_linked_list_reverse_end(CCC_Doubly_linked_list const *const) {
    return NULL;
}

void *
CCC_doubly_linked_list_next(
    CCC_Doubly_linked_list const *const list,
    CCC_Doubly_linked_list_node const *type_intruder
) {
    if (!list || !type_intruder || type_intruder->next == NULL) {
        return NULL;
    }
    return struct_base(list, type_intruder->next);
}

void *
CCC_doubly_linked_list_reverse_next(
    CCC_Doubly_linked_list const *const list,
    CCC_Doubly_linked_list_node const *type_intruder
) {
    if (!list || !type_intruder || type_intruder->previous == NULL) {
        return NULL;
    }
    return struct_base(list, type_intruder->previous);
}

CCC_Count
CCC_doubly_linked_list_count(CCC_Doubly_linked_list const *const list) {
    if (!list) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return (CCC_Count){.count = len(list->head, NULL)};
}

CCC_Tribool
CCC_doubly_linked_list_is_empty(CCC_Doubly_linked_list const *const list) {
    if (!list) {
        return CCC_TRIBOOL_ERROR;
    }
    return !list->head;
}

CCC_Result
CCC_doubly_linked_list_clear(
    CCC_Doubly_linked_list *const list,
    CCC_Destructor const *const destructor,
    CCC_Allocator const *const allocator
) {
    if (!list || !destructor || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!destructor->destroy && !allocator->allocate) {
        list->head = list->tail = NULL;
        return CCC_RESULT_OK;
    }
    while (list->head) {
        struct CCC_Doubly_linked_list_node *const removed = list->head;
        remove_node(list, removed);
        void *const node = struct_base(list, removed);
        if (destructor->destroy) {
            destructor->destroy((CCC_Arguments){
                .type = node,
                .context = destructor->context,
            });
        }
        if (allocator->allocate) {
            (void)allocator->allocate((CCC_Allocator_arguments){
                .input = node,
                .bytes = 0,
                .context = allocator->context,
            });
        }
    }
    list->tail = NULL;
    return CCC_RESULT_OK;
}

CCC_Tribool
CCC_doubly_linked_list_validate(CCC_Doubly_linked_list const *const list) {
    if (!list) {
        return CCC_TRIBOOL_ERROR;
    }
    if (!list->head && !list->tail) {
        return CCC_TRUE;
    }
    if (!list->head || !list->tail) {
        return CCC_FALSE;
    }
    struct CCC_Doubly_linked_list_node const *forward = list->head;
    struct CCC_Doubly_linked_list_node const *reverse = list->tail;
    while (forward && reverse && forward != list->tail
           && reverse != list->head) {
        if (!forward || forward->next == forward
            || forward->previous == forward) {
            return CCC_FALSE;
        }
        if (!reverse || reverse->next == reverse
            || reverse->previous == reverse) {
            return CCC_FALSE;
        }
        forward = forward->next;
        reverse = reverse->previous;
    }
    return CCC_TRUE;
}

/*==========================     Sorting     ================================*/

/** Returns true if the list is sorted in non-decreasing order. The user should
flip the return values of their comparison function if they want a different
order for elements.*/
CCC_Tribool
CCC_doubly_linked_list_is_sorted(
    CCC_Doubly_linked_list const *const list,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    if (!list || !comparator || !comparator->compare
        || (order != CCC_ORDER_LESSER && order != CCC_ORDER_GREATER)) {
        return CCC_TRIBOOL_ERROR;
    }
    if (list->head == list->tail) {
        return CCC_TRUE;
    }
    CCC_Order const wrong_order
        = order == CCC_ORDER_LESSER ? CCC_ORDER_GREATER : CCC_ORDER_LESSER;
    for (struct CCC_Doubly_linked_list_node const *cur = list->head->next;
         cur != NULL;
         cur = cur->next) {
        if (get_order(list, cur->previous, cur, comparator) == wrong_order) {
            return CCC_FALSE;
        }
    }
    return CCC_TRUE;
}

/** Inserts an element in non-decreasing order. This means an element will go
to the end of a section of duplicate values which is good for round-robin style
list use. */
void *
CCC_doubly_linked_list_insert_sorted(
    CCC_Doubly_linked_list *const list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Order const order,
    CCC_Comparator const *const comparator,
    CCC_Allocator const *const allocator
) {
    if (!list || !allocator || !comparator || !comparator->compare
        || !type_intruder) {
        return NULL;
    }
    if (allocator->allocate) {
        void *const node = allocator->allocate((CCC_Allocator_arguments){
            .input = NULL,
            .bytes = list->sizeof_type,
            .context = allocator->context,
        });
        if (!node) {
            return NULL;
        }
        (void)memcpy(node, struct_base(list, type_intruder), list->sizeof_type);
        type_intruder = type_intruder_in(list, node);
    }
    struct CCC_Doubly_linked_list_node *pos = list->head;
    for (; pos != NULL
           && get_order(list, type_intruder, pos, comparator) != order;
         pos = pos->next) {}
    insert_node(list, pos, type_intruder);
    return struct_base(list, type_intruder);
}

/** Sorts the list into non-decreasing order according to the user comparison
callback function in `O(N * log(N))` time and `O(1)` space.

The following merging algorithm and associated helper functions are based on
the iterative natural merge sort used in the list module of the pintOS project
for learning operating systems. Currently the original is located at the
following path in the pintOS source code:

`src/lib/kernel/list.c`

However, if refactors change this location, seek the list intrusive container
module for original implementations. The code has been changed for the C
Container Collection as follows:

- there is no sentinel node, only NULL pointer.
- splicing in the merge operation has been simplified along with other tweaks.
- comparison callbacks are handled with three way comparison.

If the runtime is not obvious from the code, consider that this algorithm runs
bottom up on sorted sub-ranges. It roughly "halves" the remaining sub-ranges
that need to be sorted by roughly "doubling" the length of a sorted range on
each merge step. Therefore the number of times we must perform the merge step is
`O(log(N))`. The most elements we would have to merge in the merge step is all
`N` elements so together that gives us the runtime of `O(N * log(N))`. */
CCC_Result
CCC_sort_doubly_linked_list_mergesort(
    CCC_Doubly_linked_list *const list,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    if (!list || !comparator || !comparator->compare
        || (order != CCC_ORDER_LESSER && order != CCC_ORDER_GREATER)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    /* Algorithm is one pass if list is sorted. Merging is never true. */
    CCC_Tribool merging = CCC_FALSE;
    do {
        merging = CCC_FALSE;
        /* 0th index of the A list. The start of one list to merge. */
        struct CCC_Doubly_linked_list_node *a_first = list->head;
        while (a_first != NULL) {
            /* The Nth index of list A (its size) aka 0th index of B list. */
            struct CCC_Doubly_linked_list_node *const a_count_b_first
                = first_out_of_order(list, a_first, order, comparator);
            if (a_count_b_first == NULL) {
                break;
            }
            /* A picks up the exclusive end of this merge, B, in order
               to progress the sorting algorithm with the next run that needs
               fixing. Merge returns the end of B to indicate it is the final
               sentinel node yet to be examined. */
            a_first = merge(
                list,
                a_first,
                a_count_b_first,
                first_out_of_order(list, a_count_b_first, order, comparator),
                order,
                comparator
            );
            merging = CCC_TRUE;
        }
    } while (merging);
    return CCC_RESULT_OK;
}

/** Merges lists `[a_first, a_count_b_first)` with `[a_count_b_first, b_count)`
to form `[a_first, b_count)`. Returns the exclusive end of the range, `b_count`,
once the merge sort is complete.

Notice that all ranges treat the end of their range as an exclusive sentinel for
consistency. This function assumes the provided lists are already sorted
separately. */
static inline struct CCC_Doubly_linked_list_node *
merge(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *a_first,
    struct CCC_Doubly_linked_list_node *a_count_b_first,
    struct CCC_Doubly_linked_list_node *const b_count,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    while (a_first && a_first != a_count_b_first && a_count_b_first
           && a_count_b_first != b_count) {
        if (get_order(list, a_count_b_first, a_first, comparator) == order) {
            struct CCC_Doubly_linked_list_node *const merged = a_count_b_first;
            a_count_b_first = merged->next;
            if (merged->next) {
                merged->next->previous = merged->previous;
            } else {
                list->tail = merged->previous;
            }
            assert(
                merged->previous
                && "merged element must always have a previous pointer because "
                   "lists of size 1 or less are not merged and merging "
                   "iterates forward"
            );
            merged->previous->next = merged->next;
            merged->previous = a_first->previous;
            merged->next = a_first;
            if (a_first->previous) {
                a_first->previous->next = merged;
            } else {
                list->head = merged;
            }
            a_first->previous = merged;
        } else {
            a_first = a_first->next;
        }
    }
    return b_count;
}

/** Finds the first element lesser than it's previous element as defined by
the user comparison callback function. If no out of order element can be
found the list sentinel is returned. */
static inline struct CCC_Doubly_linked_list_node *
first_out_of_order(
    struct CCC_Doubly_linked_list const *const list,
    struct CCC_Doubly_linked_list_node *start,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    assert(list && start);
    do {
        start = start->next;
    } while (start != NULL
             && get_order(list, start, start->previous, comparator) != order);
    return start;
}

/*=======================     Private Interface   ===========================*/

void
CCC_private_doubly_linked_list_push_back(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *type_intruder
) {
    push_back(list, type_intruder);
}

void
CCC_private_doubly_linked_list_push_front(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *const type_intruder
) {
    push_front(list, type_intruder);
}

struct CCC_Doubly_linked_list_node *
CCC_private_doubly_linked_list_node_in(
    struct CCC_Doubly_linked_list const *const list,
    void const *const any_struct
) {
    return type_intruder_in(list, any_struct);
}

/*=======================       Static Helpers    ===========================*/

static inline void
push_front(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *const node
) {
    node->previous = NULL;
    node->next = list->head;
    if (list->head) {
        list->head->previous = node;
    } else {
        list->tail = node;
    }
    list->head = node;
}

static inline void
push_back(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *const node
) {
    node->next = NULL;
    node->previous = list->tail;
    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
}

static inline void
insert_node(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *const position,
    struct CCC_Doubly_linked_list_node *const node
) {
    if (!position) {
        return push_back(list, node);
    }
    node->next = position;
    node->previous = position->previous;

    if (position->previous) {
        position->previous->next = node;
    } else {
        list->head = node;
    }
    position->previous = node;
}

static inline void
remove_node(
    struct CCC_Doubly_linked_list *const list,
    struct CCC_Doubly_linked_list_node *const node
) {
    if (node->previous) {
        node->previous->next = node->next;
    } else {
        list->head = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
    } else {
        list->tail = node->previous;
    }
    node->next = node->previous = NULL;
}

/** Operates on each element in the specified range calling the allocation
function to free the nodes, if provided. In either case, the number of nodes
in the inclusive range is returned. */
static void
erase_inclusive_range(
    struct CCC_Doubly_linked_list const *const list,
    struct CCC_Doubly_linked_list_node *begin,
    struct CCC_Doubly_linked_list_node *const inclusive_end,
    CCC_Allocator const *const allocator
) {
    if (!allocator->allocate) {
        return;
    }
    while (begin) {
        CCC_Doubly_linked_list_node *const next = begin->next;
        allocator->allocate((CCC_Allocator_arguments){
            .input = struct_base(list, begin),
            .bytes = 0,
            .context = allocator->context,
        });
        if (begin == inclusive_end) {
            break;
        }
        begin = next;
    }
}

/** Finds the length from [begin, end]. End is counted. */
static size_t
len(struct CCC_Doubly_linked_list_node const *begin,
    struct CCC_Doubly_linked_list_node const *const end) {
    size_t s = 0;
    while (begin != end) {
        begin = begin->next;
        ++s;
    }
    return s;
}

static inline void *
struct_base(
    struct CCC_Doubly_linked_list const *const list,
    struct CCC_Doubly_linked_list_node const *const node
) {
    return node ? ((char *)&node->next) - list->type_intruder_offset : NULL;
}

static inline struct CCC_Doubly_linked_list_node *
type_intruder_in(
    struct CCC_Doubly_linked_list const *const list,
    void const *const struct_base
) {
    return struct_base
             ? (struct CCC_Doubly_linked_list_node
                    *)((char *)struct_base + list->type_intruder_offset)
             : NULL;
}

/** Calls the user provided three way comparison callback function on the user
type wrapping the provided intrusive handles. Returns the three way comparison
result value. */
static inline CCC_Order
get_order(
    struct CCC_Doubly_linked_list const *const list,
    struct CCC_Doubly_linked_list_node const *const left,
    struct CCC_Doubly_linked_list_node const *const right,
    CCC_Comparator const *const comparator
) {
    return comparator->compare((CCC_Comparator_arguments){
        .type_left = struct_base(list, left),
        .type_right = struct_base(list, right),
        .context = comparator->context,
    });
}
