/** @cond
Copyright 2025 Alexander G. Lopez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
@endcond */
/** @file
@brief The Doubly Linked List Interface

A doubly linked list offers efficient push, pop, extract, and erase operations
for elements stored in the list. For single elements, the list can
offer O(1) push front/back, pop front/back, and removal of elements in
arbitrary positions in the list. The cost of this efficiency is higher memory
footprint.

This container offers pointer stability. Also, if the container function
requesting an allocator is not provided one, all insertion code assumes that the
user has allocated memory appropriately for the element to be inserted; it will
not allocate or free in this case. If an allocator is passed to a function
requesting one, the container will manage the memory as expected on insert or
erase operations as defined by the interface; memory is allocated for insertions
and freed for removals.

To shorten names in the interface, define the following preprocessor directive
at the top of your file.

```
#define DOUBLY_LINKED_LIST_USING_NAMESPACE_CCC
```

All types and functions can then be written without the `CCC_` prefix. */
#ifndef CCC_DOUBLY_LINKED_LIST_H
#define CCC_DOUBLY_LINKED_LIST_H

/** @cond */
#include <stddef.h>
/** @endcond */

#include "private/private_doubly_linked_list.h"
#include "types.h"

/** @name Container Types
Types available in the container interface. */
/**@{*/

/** @brief A container offering bidirectional, insert, removal, and iteration.
@warning it is undefined behavior to use an uninitialized container.

A doubly linked list may be stored in the stack, heap, or data segment. Once
initialized it is passed by reference to all functions. A doubly linked list
can be initialized at compile time or runtime. */
typedef struct CCC_Doubly_linked_list CCC_Doubly_linked_list;

/** @brief A doubly linked list intrusive element to embedded in a user type.

It can be used in an allocating or non allocating container. If allocation is
prohibited the container assumes the element is wrapped in pre-allocated
memory with the appropriate lifetime and scope for the user's needs; the
container does not allocate or free in this case. If allocation is allowed
the container will handle copying the data wrapping the element to allocations
and deallocating when necessary. */
typedef struct CCC_Doubly_linked_list_node CCC_Doubly_linked_list_node;

/**@}*/

/** @name Initialization Interface
Initialize the container with memory, callbacks, and permissions. */
/**@{*/

/** @brief Initialize an intrusive doubly linked list for a type with a user
specified intruder field.
@param[in] type_name the type containing the intrusive doubly_linked_list
element.
@param[in] type_intruder_field name of the Doubly_linked_list element in the
containing type.
@return the initialized list. Assign to the list directly on the right hand
side of an equality operator. Initialization can occur at runtime or compile
time. */
#define CCC_doubly_linked_list_default(type_name, type_intruder_field)         \
    CCC_private_doubly_linked_list_for(type_name, type_intruder_field)

/** @brief Initialize an intrusive doubly linked list for a type with a user
specified intruder field.
@param[in] type_name the type containing the intrusive doubly_linked_list
element.
@param[in] type_intruder_field name of the Doubly_linked_list element in the
containing type.
@return the initialized list. Assign to the list directly on the right hand
side of an equality operator. Initialization can occur at runtime or compile
time. */
#define CCC_doubly_linked_list_for(type_name, type_intruder_field)             \
    CCC_private_doubly_linked_list_for(type_name, type_intruder_field)

/** @brief Initialize a doubly linked list at runtime from a compound literal
array.
@param[in] type_intruder_field the name of the field intruding on user's type.
@param[in] allocator the required CCC_Allocator for allocation.
@param[in] destructor the optional destructor to run over all allocated
elements if memory exhaustion occurs at any point during construction.
@param[in] compound_literal_array the array of user types to insert into the
map (e.g. (struct My_type[]){ {.val = 1}, {.val = 2}}).
@return the initialized doubly linked list on the right side of an equality
operator.
@note The list is constructed to the specification of compound literal array
provided. The list will be constructed with the element at index 0 of the array
as the front of the list and the final index element at the back of the list. */
#define CCC_doubly_linked_list_from(                                           \
    type_intruder_field, allocator, destructor, compound_literal_array...      \
)                                                                              \
    CCC_private_doubly_linked_list_from(                                       \
        type_intruder_field, allocator, destructor, compound_literal_array     \
    )

/**@}*/

/** @name Insert and Remove Interface
Add or remove elements from the doubly linked list. */
/**@{*/

/** @brief  writes contents of type initializer directly to allocated memory at
the back of the list. O(1).
@param[in] list_pointer the address of the doubly linked list.
@param[in] allocator_pointer the required CCC_Allocator for allocation.
@param[in] type_compound_literal the r-value initializer of the type to be
inserted in the list. This should match the type containing Doubly_linked_list
elements as a struct member for this list.
@return a reference to the inserted element or NULL if allocation is not
allowed or fails.

Note that it does not make sense to use this method if the list has been
initialized without an allocation function. If the user does not allow
allocation, the contents of new elements to be inserted has been determined by
the user prior to any inserts into the list. */
#define CCC_doubly_linked_list_emplace_back(                                   \
    list_pointer, allocator_pointer, type_compound_literal...                  \
)                                                                              \
    CCC_private_doubly_linked_list_emplace_back(                               \
        list_pointer, allocator_pointer, type_compound_literal                 \
    )

/** @brief  writes contents of type initializer directly to allocated memory at
the front of the list. O(1).
@param[in] list_pointer the address of the doubly linked list.
@param[in] allocator_pointer the required CCC_Allocator for allocation.
@param[in] type_compound_literal the r-value initializer of the type to be
inserted in the list. This should match the type containing Doubly_linked_list
elements as a struct member for this list.
@return a reference to the inserted element or NULL if allocation is not
allowed or fails.

Note that it does not make sense to use this method if the list has been
initialized without an allocation function. If the user does not allow
allocation, the contents of new elements to be inserted has been determined by
the user prior to any inserts into the list. */
#define CCC_doubly_linked_list_emplace_front(                                  \
    list_pointer, allocator_pointer, type_compound_literal...                  \
)                                                                              \
    CCC_private_doubly_linked_list_emplace_front(                              \
        list_pointer, allocator_pointer, type_compound_literal                 \
    )

/** @brief Push user type wrapping type_intruder to the front of the list. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder a pointer to the list element.
@param[in] allocator the CCC_Allocator for allocating a user type.
@return a pointer to the element inserted or NULL if bad input is provided
or allocation fails. */
[[nodiscard]] void *CCC_doubly_linked_list_push_front(
    CCC_Doubly_linked_list *list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *allocator
);

/** @brief Push user type wrapping type_intruder to the back of the list. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder a pointer to the list element.
@param[in] allocator the CCC_Allocator for allocating a user type.
@return a pointer to the element inserted or NULL if bad input is provided
or allocation fails. */
[[nodiscard]] void *CCC_doubly_linked_list_push_back(
    CCC_Doubly_linked_list *list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *allocator
);

/** @brief Insert user type wrapping type_intruder before position_node. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] position_node a pointer to the list element before which
type_intruder inserts.
@param[in] type_intruder a pointer to the list element.
@param[in] allocator the CCC_Allocator for allocating a user type.
@return a pointer to the element inserted or NULL if bad input is provided
or allocation fails. */
[[nodiscard]] void *CCC_doubly_linked_list_insert(
    CCC_Doubly_linked_list *list,
    CCC_Doubly_linked_list_node *position_node,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *allocator
);

/** @brief Pop the user type at the front of the list. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] allocator the CCC_Allocator for freeing a user type.
@return an ok result if the pop was successful or an error if bad input is
provided or the list is empty.*/
CCC_Result CCC_doubly_linked_list_pop_front(
    CCC_Doubly_linked_list *list, CCC_Allocator const *allocator
);

/** @brief Pop the user type at the back of the list. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] allocator the CCC_Allocator for freeing a user type.
@return an ok result if the pop was successful or an error if bad input is
provided or the list is empty.*/
CCC_Result CCC_doubly_linked_list_pop_back(
    CCC_Doubly_linked_list *list, CCC_Allocator const *allocator
);

/** @brief Returns the element following an extracted element from the list
without deallocating regardless of allocation permission provided to the
container. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder the handle of an element known to be in the list.
@return a reference to the element in the list following type_intruder or NULL
if the element is the last. NULL is returned if bad input is provided or the
type_intruder is not in the list. */
void *CCC_doubly_linked_list_extract(
    CCC_Doubly_linked_list *list, CCC_Doubly_linked_list_node *type_intruder
);

/** @brief Returns the element following an erased element from the list. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder the handle of an element known to be in the list.
@param[in] allocator the CCC_Allocator for freeing a user type.
@return a reference to the element in the list following type_intruder or NULL
if the element is the last. NULL is returned if bad input is provided or the
type_intruder is not in the list. */
void *CCC_doubly_linked_list_erase(
    CCC_Doubly_linked_list *list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Allocator const *allocator
);

/** @brief Returns the element following an extracted range of elements from the
list. O(N).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder_begin the handle of an element known to be in the list
at the start of the range.
@param[in] type_intruder_end the handle of an element known to be in the list at
the end of the range following type_intruder_begin.
@param[in] allocator the CCC_Allocator for freeing a user type.
@return a reference to the element in the list following type_intruder_end or
NULL if the element is the last. NULL is returned if bad input is provided or
the type_intruder is not in the list.

Note that if the user does not permit the container to allocate they may iterate
through the extracted range in the same way one iterates through a normal list
using the iterator function. If allocation is allowed, all elements from
type_intruder_begin to type_intruder_end will be erased and references
invalidated. */
void *CCC_doubly_linked_list_erase_range(
    CCC_Doubly_linked_list *list,
    CCC_Doubly_linked_list_node *type_intruder_begin,
    CCC_Doubly_linked_list_node *type_intruder_end,
    CCC_Allocator const *allocator
);

/** @brief Returns the element following an extracted range of elements from the
list without deallocating regardless of allocation permission provided to the
container. O(N).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder_begin the handle of an element known to be in the list
at the start of the range.
@param[in] type_intruder_end the handle of an element known to be in the list at
the end of the range following type_intruder_begin.
@return a reference to the element in the list following type_intruder_end or
NULL if the element is the last. NULL is returned if bad input is provided or
the type_intruder is not in the list.

Note that the user may iterate through the extracted range in the same way one
iterates through a normal list using the iterator function. */
void *CCC_doubly_linked_list_extract_range(
    CCC_Doubly_linked_list *list,
    CCC_Doubly_linked_list_node *type_intruder_begin,
    CCC_Doubly_linked_list_node *type_intruder_end
);

/** @brief Repositions to_cut before pos. Only list pointers are modified. O(1).
@param[in] position_doubly_linked_list the list to which position belongs.
@param[in] type_intruder_position the position before which to_cut will be
moved.
@param[in] to_cut_doubly_linked_list the list to which to_cut belongs.
@param[in] type_intruder_to_cut the element to cut.
@return ok if the splice is successful or an error if bad input is provided. */
CCC_Result CCC_doubly_linked_list_splice(
    CCC_Doubly_linked_list *position_doubly_linked_list,
    CCC_Doubly_linked_list_node *type_intruder_position,
    CCC_Doubly_linked_list *to_cut_doubly_linked_list,
    CCC_Doubly_linked_list_node *type_intruder_to_cut
);

/** @brief Splices the list to cut before the specified position. The range
being cut is exclusive from [start, end), meaning the final element provided is
not move. This is an O(N) operation.
@param[in] position_doubly_linked_list the list to which position belongs.
@param[in] type_intruder_position the position before which the list is moved.
@param[in] to_cut_doubly_linked_list the list to which the range belongs.
@param[in] type_intruder_to_cut_begin the start of the list to splice.
@param[in] type_intruder_to_cut_exclusive_end the exclusive end of the list to
splice, not included in the splice operation.
@return OK if the splice is successful or an error if bad input is provided. */
CCC_Result CCC_doubly_linked_list_splice_range(
    CCC_Doubly_linked_list *position_doubly_linked_list,
    CCC_Doubly_linked_list_node *type_intruder_position,
    CCC_Doubly_linked_list *to_cut_doubly_linked_list,
    CCC_Doubly_linked_list_node *type_intruder_to_cut_begin,
    CCC_Doubly_linked_list_node *type_intruder_to_cut_exclusive_end
);

/**@}*/

/** @name Sorting Interface
Sort the container. */
/**@{*/

/** @brief Inserts type_intruder in sorted position according to the
non-decreasing order of the list determined by the user provided comparison
function. `O(1)`.
@param[in] doubly_linked_list a pointer to the doubly linked list.
@param[in] type_intruder a pointer to the element to be inserted in order.
@param[in] order the order by which the list should be sorted. CCC_ORDER_LESSER
means the list should be in non-increasing order from [0, count).
CCC_ORDER_GREATER means the list should be in non-decreasing order from
[0, count).
@param[in] comparator the CCC_Comparator for comparing list elements.
@param[in] allocator the CCC_Allocator for allocating a user type.
@return a pointer to the element that has been inserted or NULL if allocation
is required and has failed. NULL is also returned if the list has never been
formally sorted from sort.h.
@warning This function assumes the list is sorted and inserts according to the
last sorted ordering. If the list has never been sorted NULL is returned.

If a non-increasing order is desired, return opposite results from the user
comparison function. If an element is CCC_ORDER_LESSERERS return
CCC_ORDER_GREATER and vice versa. If elements are equal, CCC_ORDER_EQUAL. */
void *CCC_doubly_linked_list_insert_sorted(
    CCC_Doubly_linked_list *doubly_linked_list,
    CCC_Doubly_linked_list_node *type_intruder,
    CCC_Order order,
    CCC_Comparator const *comparator,
    CCC_Allocator const *allocator
);

/** @brief Returns true if the list is sorted in non-decreasing order according
to the user provided comparison function.
@param[in] doubly_linked_list a pointer to the singly linked list.
@param[in] order the assumed order checked against last sorted order of list.
@param[in] comparator the comparator context for comparing list elements.
@return CCC_TRUE if the list has been previously sorted and all elements remain
in the assumed input sorted order. CCC_FALSE is returned if the list is not
completely sorted in the assumed input order. CCC_TRIBOOL_ERROR is returned if
the input list pointer is NULL.

If a non-increasing order is desired, return opposite results from the user
comparison function. If an element is CCC_ORDER_LESSER return CCC_ORDER_GREATER
and vice versa. If elements are equal, return CCC_ORDER_EQUAL. */
CCC_Tribool CCC_doubly_linked_list_is_sorted(
    CCC_Doubly_linked_list const *doubly_linked_list,
    CCC_Order order,
    CCC_Comparator const *comparator
);

/**@}*/

/** @name Deallocation Interface
Deallocate the container. */
/**@{*/

/** @brief Clear the contents of the list freeing elements, if given allocation
permission. O(N).
@param[in] list a pointer to the doubly linked list.
@param[in] destructor an optional CCC_Destructor to run on each element.
@param[in] allocator the CCC_Allocator for freeing a user type.
@return ok if the clearing was a success or an input error if list or destroy is
NULL.

Note that if an allocator is provided it will be called to free each element in
the list. Be mindful that the destructor does not free as well, instead taking
care of any destructor-like book keeping. If no destructor is needed provide
the default destructor `&(CCC_Destructor){}`.

If the list is not provided with an allocator, `&(CCC_Allocator){}`, the user
should free the list elements with the destructor if they wish to do so. The
implementation ensures the function is called after the element is removed.
Otherwise, the user must manage their elements at their discretion after the
list is emptied in this function. */
CCC_Result CCC_doubly_linked_list_clear(
    CCC_Doubly_linked_list *list,
    CCC_Destructor const *destructor,
    CCC_Allocator const *allocator
);

/**@}*/

/** @name Iteration Interface
Iterate through the doubly linked list. */
/**@{*/

/** @brief Return the user type at the start of the list or NULL if empty. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the user type or NULL if empty or bad input. */
[[nodiscard]] void *
CCC_doubly_linked_list_begin(CCC_Doubly_linked_list const *list);

/** @brief Return the user type at the end of the list or NULL if empty. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the user type or NULL if empty or bad input. */
[[nodiscard]] void *
CCC_doubly_linked_list_reverse_begin(CCC_Doubly_linked_list const *list);

/** @brief Return the user type following the element known to be in the list.
O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder a handle to the list element known to be in the list.
@return a pointer to the element following type_intruder or NULL if no elements
follow or bad input is provided. */
[[nodiscard]] void *CCC_doubly_linked_list_next(
    CCC_Doubly_linked_list const *list,
    CCC_Doubly_linked_list_node const *type_intruder
);

/** @brief Return the user type following the element known to be in the list
moving from back to front. O(1).
@param[in] list a pointer to the doubly linked list.
@param[in] type_intruder a handle to the list element known to be in the list.
@return a pointer to the element following type_intruder from back to front or
NULL if no elements follow or bad input is provided. */
[[nodiscard]] void *CCC_doubly_linked_list_reverse_next(
    CCC_Doubly_linked_list const *list,
    CCC_Doubly_linked_list_node const *type_intruder
);

/** @brief Return the end sentinel with no accessible fields. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the end sentinel with no accessible fields. */
[[nodiscard]] void *
CCC_doubly_linked_list_end(CCC_Doubly_linked_list const *list);

/** @brief Return the start sentinel with no accessible fields. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the start sentinel with no accessible fields. */
[[nodiscard]] void *
CCC_doubly_linked_list_reverse_end(CCC_Doubly_linked_list const *list);

/**@}*/

/** @name State Interface
Obtain state from the doubly linked list. */
/**@{*/

/** @brief Returns the user type at the front of the list. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the user type at the front of the list. NULL if empty. */
[[nodiscard]] void *
CCC_doubly_linked_list_front(CCC_Doubly_linked_list const *list);

/** @brief Returns the user type at the back of the list. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the user type at the back of the list. NULL if empty. */
[[nodiscard]] void *
CCC_doubly_linked_list_back(CCC_Doubly_linked_list const *list);

/** @brief Return a handle to the list element at the front of the list which
may be the sentinel. O(1).
@param[in] list a pointer to the doubly linked list.
@return a pointer to the list element at the beginning of the list which may be
the sentinel but will not be NULL unless a NULL pointer is provided as l. */
[[nodiscard]] CCC_Doubly_linked_list_node *
CCC_doubly_linked_list_node_begin(CCC_Doubly_linked_list const *list);

/** @brief Return the count of elements in the list. O(1).
@param[in] list a pointer to the doubly linked list.
@return the size of the list. An argument error is set if list is NULL. */
[[nodiscard]] CCC_Count
CCC_doubly_linked_list_count(CCC_Doubly_linked_list const *list);

/** @brief Return if the size of the list is equal to 0. O(1).
@param[in] list a pointer to the doubly linked list.
@return true if the size is 0, else false. Error if list is NULL. */
[[nodiscard]] CCC_Tribool
CCC_doubly_linked_list_is_empty(CCC_Doubly_linked_list const *list);

/** @brief Validates internal state of the list.
@param[in] list a pointer to the doubly linked list.
@return true if invariants hold, false if not. Error if list is NULL. */
[[nodiscard]] CCC_Tribool
CCC_doubly_linked_list_validate(CCC_Doubly_linked_list const *list);

/**@}*/

/** Define this preprocessor directive before including this header if shorter
names are desired for the doubly linked list container. Ensure no namespace
clashes exist prior to name shortening. */
#ifdef DOUBLY_LINKED_LIST_USING_NAMESPACE_CCC
/* NOLINTBEGIN(readability-identifier-naming) */
typedef CCC_Doubly_linked_list_node Doubly_linked_list_node;
typedef CCC_Doubly_linked_list Doubly_linked_list;
#    define doubly_linked_list_default(arguments...)                           \
        CCC_doubly_linked_list_default(arguments)
#    define doubly_linked_list_for(arguments...)                               \
        CCC_doubly_linked_list_for(arguments)
#    define doubly_linked_list_from(arguments...)                              \
        CCC_doubly_linked_list_from(arguments)
#    define doubly_linked_list_emplace_back(arguments...)                      \
        CCC_doubly_linked_list_emplace_back(arguments)
#    define doubly_linked_list_emplace_front(arguments...)                     \
        CCC_doubly_linked_list_emplace_front(arguments)
#    define doubly_linked_list_push_front(arguments...)                        \
        CCC_doubly_linked_list_push_front(arguments)
#    define doubly_linked_list_push_back(arguments...)                         \
        CCC_doubly_linked_list_push_back(arguments)
#    define doubly_linked_list_front(arguments...)                             \
        CCC_doubly_linked_list_front(arguments)
#    define doubly_linked_list_back(arguments...)                              \
        CCC_doubly_linked_list_back(arguments)
#    define doubly_linked_list_pop_front(arguments...)                         \
        CCC_doubly_linked_list_pop_front(arguments)
#    define doubly_linked_list_pop_back(arguments...)                          \
        CCC_doubly_linked_list_pop_back(arguments)
#    define doubly_linked_list_extract(arguments...)                           \
        CCC_doubly_linked_list_extract(arguments)
#    define doubly_linked_list_extract_range(arguments...)                     \
        CCC_doubly_linked_list_extract_range(arguments)
#    define doubly_linked_list_erase(arguments...)                             \
        CCC_doubly_linked_list_erase(arguments)
#    define doubly_linked_list_erase_range(arguments...)                       \
        CCC_doubly_linked_list_erase_range(arguments)
#    define doubly_linked_list_splice(arguments...)                            \
        CCC_doubly_linked_list_splice(arguments)
#    define doubly_linked_list_splice_range(arguments...)                      \
        CCC_doubly_linked_list_splice_range(arguments)
#    define doubly_linked_list_sort(arguments...)                              \
        CCC_doubly_linked_list_sort(arguments)
#    define doubly_linked_list_insert_sorted(arguments...)                     \
        CCC_doubly_linked_list_insert_sorted(arguments)
#    define doubly_linked_list_is_sorted(arguments...)                         \
        CCC_doubly_linked_list_is_sorted(arguments)
#    define doubly_linked_list_begin(arguments...)                             \
        CCC_doubly_linked_list_begin(arguments)
#    define doubly_linked_list_next(arguments...)                              \
        CCC_doubly_linked_list_next(arguments)
#    define doubly_linked_list_reverse_begin(arguments...)                     \
        CCC_doubly_linked_list_reverse_begin(arguments)
#    define doubly_linked_list_reverse_next(arguments...)                      \
        CCC_doubly_linked_list_reverse_next(arguments)
#    define doubly_linked_list_end(arguments...)                               \
        CCC_doubly_linked_list_end(arguments)
#    define doubly_linked_list_reverse_end(arguments...)                       \
        CCC_doubly_linked_list_reverse_end(arguments)
#    define doubly_linked_list_node_begin(arguments...)                        \
        CCC_doubly_linked_list_node_begin(arguments)
#    define doubly_linked_list_count(arguments...)                             \
        CCC_doubly_linked_list_count(arguments)
#    define doubly_linked_list_is_empty(arguments...)                          \
        CCC_doubly_linked_list_is_empty(arguments)
#    define doubly_linked_list_clear(arguments...)                             \
        CCC_doubly_linked_list_clear(arguments)
#    define doubly_linked_list_validate(arguments...)                          \
        CCC_doubly_linked_list_validate(arguments)
/* NOLINTEND(readability-identifier-naming) */
#endif /* DOUBLY_LINKED_LIST_USING_NAMESPACE_CCC */

#endif /* CCC_LIST_H */
