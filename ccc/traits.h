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
@brief The C Container Collection Traits Interface

Many functionalities across containers are similar. These can be described as
traits that each container implements (see Rust Traits for a more pure example
of the topic). Only a selection of shared traits across containers are
represented here because some containers implement unique functionality that
cannot be shared with other containers. These can simplify code greatly at a
slightly higher compilation resource cost. There is no runtime cost to using
traits.

To shorten names in the interface, define the following preprocessor directive
at the top of your file.

```
#define TRAITS_USING_NAMESPACE_CCC
```

All traits can then be written without the `CCC_` prefix. */
#ifndef CCC_TRAITS_H
#define CCC_TRAITS_H

#include "private/private_traits.h"

/** @name Entry Interface
Obtain and operate on container entries for efficient queries when non-trivial
control flow is needed. */
/**@{*/

/** @brief Insert an element and obtain the old value if Occupied.
@param[in] container_pointer a pointer to the container.
@param swap_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_swap_entry(container_pointer, swap_arguments...)                   \
    CCC_private_swap_entry(container_pointer, swap_arguments)

/** @brief Insert an element and obtain the old value if Occupied.
@param[in] container_pointer a pointer to the container.
@param swap_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_swap_entry_wrap(container_pointer, swap_arguments...)              \
    CCC_private_swap_entry_wrap(container_pointer, swap_arguments)

/** @brief Insert an element and obtain the old value if Occupied.
@param[in] container_pointer a pointer to the container.
@param swap_arguments arguments depend on container.
@return a handle depending on container specific context.

See container documentation for specific behavior. */
#define CCC_swap_handle(container_pointer, swap_arguments...)                  \
    CCC_private_swap_handle(container_pointer, swap_arguments)

/** @brief Insert an element and obtain the old value if Occupied.
@param[in] container_pointer a pointer to the container.
@param swap_arguments arguments depend on container.
@return a handle depending on container specific context.

See container documentation for specific behavior. */
#define CCC_swap_handle_wrap(container_pointer, swap_arguments...)             \
    CCC_private_swap_handle_wrap(container_pointer, swap_arguments)

/** @brief Insert an element if the entry is Vacant.
@param[in] container_pointer a pointer to the container.
@param try_insert_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_try_insert(container_pointer, try_insert_arguments...)             \
    CCC_private_try_insert(container_pointer, try_insert_arguments)

/** @brief Insert an element if the entry is Vacant.
@param[in] container_pointer a pointer to the container.
@param try_insert_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_try_insert_wrap(container_pointer, try_insert_arguments...)        \
    CCC_private_try_insert_wrap(container_pointer, try_insert_arguments)

/** @brief Insert an element or overwrite the Occupied entry.
@param[in] container_pointer a pointer to the container.
@param insert_or_assign_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_insert_or_assign(container_pointer, insert_or_assign_arguments...) \
    CCC_private_insert_or_assign(container_pointer, insert_or_assign_arguments)

/** @brief Insert an element or overwrite the Occupied entry.
@param[in] container_pointer a pointer to the container.
@param insert_or_assign_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_insert_or_assign_wrap(container_pointer,                           \
                                  insert_or_assign_arguments...)               \
    CCC_private_insert_or_assign_wrap(container_pointer,                       \
                                      insert_or_assign_arguments)

/** @brief Remove an element and retain access to its value.
@param[in] container_pointer a pointer to the container.
@param remove_key_value_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_remove_key_value(container_pointer, remove_key_value_arguments...) \
    CCC_private_remove_key_value(container_pointer, remove_key_value_arguments)

/** @brief Remove an element and retain access to its value.
@param[in] container_pointer a pointer to the container.
@param remove_key_value_arguments arguments depend on container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_remove_key_value_wrap(container_pointer,                           \
                                  remove_key_value_arguments...)               \
    CCC_private_remove_key_value_wrap(container_pointer,                       \
                                      remove_key_value_arguments)

/** @brief Obtain a container specific entry for the Entry Interface.
@param[in] container_pointer a pointer to the container.
@param[in] key_pointer a pointer to the search key.
@return a container specific entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_entry(container_pointer, key_pointer...)                           \
    CCC_private_entry(container_pointer, key_pointer)

/** @brief Obtain a container specific handle for the handle Interface.
@param[in] container_pointer a pointer to the container.
@param[in] key_pointer a pointer to the search key.
@return a container specific handle depending on container specific context.

See container documentation for specific behavior. */
#define CCC_handle(container_pointer, key_pointer...)                          \
    CCC_private_handle(container_pointer, key_pointer)

/** @brief Obtain a container specific entry for the Entry Interface.
@param[in] container_pointer a pointer to the container.
@param[in] key_pointer a pointer to the search key.
@return a container specific entry reference depending on container specific
context.

See container documentation for specific behavior. */
#define CCC_entry_wrap(container_pointer, key_pointer...)                      \
    CCC_private_entry_wrap(container_pointer, key_pointer)

/** @brief Obtain a container specific handle for the handle Interface.
@param[in] container_pointer a pointer to the container.
@param[in] key_pointer a pointer to the search key.
@return a container specific handle reference depending on container specific
context.

See container documentation for specific behavior. */
#define CCC_handle_wrap(container_pointer, key_pointer...)                     \
    CCC_private_handle_wrap(container_pointer, key_pointer)

/** @brief Modify an entry if Occupied.
@param[in] entry_pointer a pointer to the container.
@param[in] mod_fn a modification function that does not need context.
@return a reference to the modified entry if Occupied or original if Vacant.

See container documentation for specific behavior. */
#define CCC_and_modify(entry_pointer, mod_fn)                                  \
    CCC_private_and_modify(entry_pointer, mod_fn)

/** @brief Modify an entry if Occupied.
@param[in] entry_pointer a pointer to the container.
@param[in] modify a modification function.
@param[in] context_arguments context data for mod_fn.
@return a reference to the modified entry if Occupied or original if Vacant.

See container documentation for specific behavior. */
#define CCC_and_context_modify(entry_pointer, modify, context_arguments...)    \
    CCC_private_and_context_modify(entry_pointer, modify, context_arguments)

/** @brief Insert new element or overwrite old element.
@param[in] entry_pointer a pointer to the container.
@param insert_entry_arguments arguments depend on container.
@return an reference to the inserted element.

See container documentation for specific behavior. */
#define CCC_insert_entry(entry_pointer, insert_entry_arguments...)             \
    CCC_private_insert_entry(entry_pointer, insert_entry_arguments)

/** @brief Insert new element or overwrite old element.
@param[in] array_pointer a pointer to the container.
@param insert_array_arguments arguments depend on container.
@return an reference to the inserted element.

See container documentation for specific behavior. */
#define CCC_insert_handle(array_pointer, insert_array_arguments...)            \
    CCC_private_insert_handle(array_pointer, insert_array_arguments)

/** @brief Insert new element if the entry is Vacant.
@param[in] entry_pointer a pointer to the container.
@param or_insert_arguments arguments depend on container.
@return an reference to the old element or new element if entry was Vacant.

See container documentation for specific behavior. */
#define CCC_or_insert(entry_pointer, or_insert_arguments...)                   \
    CCC_private_or_insert(entry_pointer, or_insert_arguments)

/** @brief Remove the element if the entry is Occupied.
@param[in] entry_pointer a pointer to the container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_remove_entry(entry_pointer) CCC_private_remove_entry(entry_pointer)

/** @brief Remove the element if the entry is Occupied.
@param[in] entry_pointer a pointer to the container.
@return an entry depending on container specific context.

See container documentation for specific behavior. */
#define CCC_remove_entry_wrap(entry_pointer)                                   \
    CCC_private_remove_entry_wrap(entry_pointer)

/** @brief Remove the element if the handle is Occupied.
@param[in] array_pointer a pointer to the container.
@return an handle depending on container specific context.

See container documentation for specific behavior. */
#define CCC_remove_handle(array_pointer)                                       \
    CCC_private_remove_handle(array_pointer)

/** @brief Remove the element if the handle is Occupied.
@param[in] array_pointer a pointer to the container.
@return an handle depending on container specific context.

See container documentation for specific behavior. */
#define CCC_remove_handle_wrap(array_pointer)                                  \
    CCC_private_remove_handle_wrap(array_pointer)

/** @brief Unwrap user type in entry.
@param[in] entry_pointer a pointer to the container.
@return a valid reference if Occupied or NULL if vacant.

See container documentation for specific behavior. */
#define CCC_unwrap(entry_pointer) CCC_private_unwrap(entry_pointer)

/** @brief Check occupancy of entry.
@param[in] entry_pointer a pointer to the container.
@return true if Occupied, false if Vacant.

See container documentation for specific behavior. */
#define CCC_occupied(entry_pointer) CCC_private_occupied(entry_pointer)

/** @brief Check last insert status.
@param[in] entry_pointer a pointer to the container.
@return true if an insert error occurred false if not.

See container documentation for specific behavior. */
#define CCC_insert_error(entry_pointer) CCC_private_insert_error(entry_pointer)

/**@}*/

/**@name Membership Interface
Test membership or obtain references to stored user types directly. */
/**@{*/

/** @brief Obtain a reference to the user type stored at the key.
@param[in] container_pointer a pointer to the container.
@param[in] key_pointer a pointer to the search key.
@return a reference to the stored user type or NULL of absent.

See container documentation for specific behavior. */
#define CCC_get_key_value(container_pointer, key_pointer...)                   \
    CCC_private_get_key_value(container_pointer, key_pointer)

/** @brief Check for membership of the key.
@param[in] container_pointer a pointer to the container.
@param[in] key_pointer a pointer to the search key.
@return true if present false if absent.

See container documentation for specific behavior. */
#define CCC_contains(container_pointer, key_pointer...)                        \
    CCC_private_contains(container_pointer, key_pointer)

/**@}*/

/**@name Push Pop Front Back Interface
Push, pop, and view elements in sorted or unsorted containers. */
/**@{*/

/** @brief Push an element into a container.
@param[in] container_pointer a pointer to the container.
@param push_arguments depend on container.
@return a reference to the pushed element.

See container documentation for specific behavior. */
#define CCC_push(container_pointer, push_arguments...)                         \
    CCC_private_push(container_pointer, push_arguments)

/** @brief Push an element to the back of a container.
@param[in] container_pointer a pointer to the container.
@param push_arguments depend on container.
@return a reference to the pushed element.

See container documentation for specific behavior. */
#define CCC_push_back(container_pointer, push_arguments...)                    \
    CCC_private_push_back(container_pointer, push_arguments)

/** @brief Push an element to the front of a container.
@param[in] container_pointer a pointer to the container.
@param push_arguments depend on container.
@return a reference to the pushed element.

See container documentation for specific behavior. */
#define CCC_push_front(container_pointer, push_arguments...)                   \
    CCC_private_push_front(container_pointer, push_arguments)

/** @brief Pop an element from a container.
@param[in] container_pointer a pointer to the container.
@param[in] pop_arguments any supplementary arguments a container may have for
the pop.
@return a result of the pop operation.

See container documentation for specific behavior. */
#define CCC_pop(container_pointer, pop_arguments...)                           \
    CCC_private_pop(container_pointer, pop_arguments)

/** @brief Pop an element from the front of a container.
@param[in] container_pointer a pointer to the container.
@return a result of the pop operation.

See container documentation for specific behavior. */
#define CCC_pop_front(container_pointer)                                       \
    CCC_private_pop_front(container_pointer)

/** @brief Pop an element from the back of a container.
@param[in] container_pointer a pointer to the container.
@return a result of the pop operation.

See container documentation for specific behavior. */
#define CCC_pop_back(container_pointer) CCC_private_pop_back(container_pointer)

/** @brief Obtain a reference the front element of a container.
@param[in] container_pointer a pointer to the container.
@return a reference to a user type.

See container documentation for specific behavior. */
#define CCC_front(container_pointer) CCC_private_front(container_pointer)

/** @brief Obtain a reference the back element of a container.
@param[in] container_pointer a pointer to the container.
@return a reference to a user type.

See container documentation for specific behavior. */
#define CCC_back(container_pointer) CCC_private_back(container_pointer)

/** @brief Splice an element from one position to another in the same or a
different container.
@param[in] container_pointer a pointer to the container.
@param splice_arguments are container specific.
@return the result of the splice.

See container documentation for specific behavior. */
#define CCC_splice(container_pointer, splice_arguments...)                     \
    CCC_private_splice(container_pointer, splice_arguments)

/** @brief Splice a range of elements from one position to another in the same
or a different container.
@param[in] container_pointer a pointer to the container.
@param splice_arguments are container specific.
@return the result of the splice.

See container documentation for specific behavior. */
#define CCC_splice_range(container_pointer, splice_arguments...)               \
    CCC_private_splice_range(container_pointer, splice_arguments)

/**@}*/

/**@name Priority Queue Interface
Interface to support generic priority queue operations. */
/**@{*/

/** @brief Update the value of an element known to be in a container.
@param[in] container_pointer a pointer to the container.
@param update_arguments depend on the container.

See container documentation for specific behavior. */
#define CCC_update(container_pointer, update_arguments...)                     \
    CCC_private_update(container_pointer, update_arguments)

/** @brief Increase the value of an element known to be in a container.
@param[in] container_pointer a pointer to the container.
@param increase_arguments depend on the container.

See container documentation for specific behavior. */
#define CCC_increase(container_pointer, increase_arguments...)                 \
    CCC_private_increase(container_pointer, increase_arguments)

/** @brief Decrease the value of an element known to be in a container.
@param[in] container_pointer a pointer to the container.
@param decrease_arguments depend on the container.

See container documentation for specific behavior. */
#define CCC_decrease(container_pointer, decrease_arguments...)                 \
    CCC_private_decrease(container_pointer, decrease_arguments)

/** @brief Erase an element known to be in a container.
@param[in] container_pointer a pointer to the container.
@param erase_arguments depend on the container.

See container documentation for specific behavior. */
#define CCC_erase(container_pointer, erase_arguments...)                       \
    CCC_private_erase(container_pointer, erase_arguments)

/** @brief Extract an element known to be in a container (does not free).
@param[in] container_pointer a pointer to the container.
@param extract_arguments depend on the container.

See container documentation for specific behavior. */
#define CCC_extract(container_pointer, extract_arguments...)                   \
    CCC_private_extract(container_pointer, extract_arguments)

/** @brief Extract elements known to be in a container (does not free).
@param[in] container_pointer a pointer to the container.
@param extract_arguments depend on the container.

See container documentation for specific behavior. */
#define CCC_extract_range(container_pointer, extract_arguments...)             \
    CCC_private_extract_range(container_pointer, extract_arguments)

/**@}*/

/** @name Iterator Interface
Obtain and manage iterators over the container. */
/**@{*/

/** @brief Obtain a reference to the start of a container.
@param[in] container_pointer a pointer to the container.
@return a reference to the user type stored at the start.

See container documentation for specific behavior. */
#define CCC_begin(container_pointer) CCC_private_begin(container_pointer)

/** @brief Obtain a reference to the reversed start of a container.
@param[in] container_pointer a pointer to the container.
@return a reference to the user type stored at the reversed start.

See container documentation for specific behavior. */
#define CCC_reverse_begin(container_pointer)                                   \
    CCC_private_reverse_begin(container_pointer)

/** @brief Obtain a reference to the next element in the container.
@param[in] container_pointer a pointer to the container.
@param[in] void_iterator_pointer the user type returned from the last
iteration.
@return a reference to the user type stored next.

See container documentation for specific behavior. */
#define CCC_next(container_pointer, void_iterator_pointer)                     \
    CCC_private_next(container_pointer, void_iterator_pointer)

/** @brief Obtain a reference to the reverse_next element in the container.
@param[in] container_pointer a pointer to the container.
@param[in] void_iterator_pointer the user type returned from the last
iteration.
@return a reference to the user type stored reverse_next.

See container documentation for specific behavior. */
#define CCC_reverse_next(container_pointer, void_iterator_pointer)             \
    CCC_private_reverse_next(container_pointer, void_iterator_pointer)

/** @brief Obtain a reference to the end sentinel of a container.
@param[in] container_pointer a pointer to the container.
@return a reference to the end sentinel.

See container documentation for specific behavior. */
#define CCC_end(container_pointer) CCC_private_end(container_pointer)

/** @brief Obtain a reference to the reverse_end sentinel of a container.
@param[in] container_pointer a pointer to the container.
@return a reference to the reverse_end sentinel.

See container documentation for specific behavior. */
#define CCC_reverse_end(container_pointer)                                     \
    CCC_private_reverse_end(container_pointer)

/** @brief Obtain a range of values from a container.
@param[in] container_pointer a pointer to the container.
@param range_arguments are container specific.
@return the range.

See container documentation for specific behavior. */
#define CCC_equal_range(container_pointer, range_arguments...)                 \
    CCC_private_equal_range(container_pointer, range_arguments)

/** @brief Obtain a range of values from a container.
@param[in] container_pointer a pointer to the container.
@param range_arguments are container specific.
@return a reference to the range.

See container documentation for specific behavior. */
#define CCC_equal_range_wrap(container_pointer, range_arguments...)            \
    CCC_private_equal_range_wrap(container_pointer, range_arguments)

/** @brief Obtain a range_reverse of values from a container.
@param[in] container_pointer a pointer to the container.
@param range_reverse_arguments are container specific.
@return the range_reverse.

See container documentation for specific behavior. */
#define CCC_equal_range_reverse(container_pointer, range_reverse_arguments...) \
    CCC_private_equal_range_reverse(container_pointer, range_reverse_arguments)

/** @brief Obtain a range_reverse of values from a container.
@param[in] container_pointer a pointer to the container.
@param range_reverse_arguments are container specific.
@return a reference to the range_reverse.

See container documentation for specific behavior. */
#define CCC_equal_range_reverse_wrap(container_pointer,                        \
                                     range_reverse_arguments...)               \
    CCC_private_equal_range_reverse_wrap(container_pointer,                    \
                                         range_reverse_arguments)

/** @brief Obtain the beginning of the range iterator.
@param[in] range_pointer a pointer to the type of range.
@return the iterator representing the beginning. May be equal to end. */
#define CCC_range_begin(range_pointer) CCC_private_range_begin(range_pointer)

/** @brief Obtain the end of the range iterator.
@param[in] range_pointer a pointer to the type of range.
@return the iterator representing the end.
@warning Do not access the end. It is an exclusive end. */
#define CCC_range_end(range_pointer) CCC_private_range_end(range_pointer)

/** @brief Obtain the beginning of the reverse range iterator.
@param[in] range_reverse_pointer a pointer to the type of reverse range.
@return the iterator representing the reverse beginning. May be equal to reverse
end. */
#define CCC_range_reverse_begin(range_reverse_pointer)                         \
    CCC_private_range_reverse_begin(range_reverse_pointer)

/** @brief Obtain the end of the reverse range iterator.
@param[in] range_reverse_pointer a pointer to the type of reverse range.
@return the iterator representing the reverse end.
@warning Do not access the end. It is an exclusive reverse end. */
#define CCC_range_reverse_end(range_reverse_pointer)                           \
    CCC_private_range_reverse_end(range_reverse_pointer)

/**@}*/

/** @name Memory Management Interface
Manage underlying buffers for containers. */
/**@{*/

/** @brief Copy source containers memory to destination container.
@param[in] destination_container_pointer a pointer to the destination container.
@param[in] source_container_pointer a pointer to the source container.
@param[in] allocate_pointer the allocation function to use for resizing if
needed.
@return the result of the operation.

See container documentation for specific behavior. */
#define CCC_copy(destination_container_pointer, source_container_pointer,      \
                 allocate_pointer)                                             \
    CCC_private_copy(destination_container_pointer, source_container_pointer,  \
                     allocate_pointer)

/** @brief Reserve capacity for n_to_add new elements to be inserted.
@param[in] container_pointer a pointer to the container.
@param[in] n_to_add the number of elements to add to the container.
@param[in] allocate_pointer the allocation function to use for resizing if
needed.
@return the result of the operation.

See container documentation for specific behavior. */
#define CCC_reserve(container_pointer, n_to_add, allocate_pointer)             \
    CCC_private_reserve(container_pointer, n_to_add, allocate_pointer)

/** @brief Clears the container without freeing the underlying buffer.
@param[in] container_pointer a pointer to the container.
@param[in] destructor_arguments optional function to be called on each element.
@return the result of the operation.

See container documentation for specific behavior. */
#define CCC_clear(container_pointer, destructor_arguments...)                  \
    CCC_private_clear(container_pointer, destructor_arguments)

/** @brief Clears the container and frees the underlying buffer.
@param[in] container_pointer a pointer to the container.
@param[in] destructor_and_free_arguments optional function to be called on each
element.
@return the result of the operation.

See container documentation for specific behavior. */
#define CCC_clear_and_free(container_pointer,                                  \
                           destructor_and_free_arguments...)                   \
    CCC_private_clear_and_free(container_pointer, destructor_and_free_arguments)

/** @brief Clears the container previously reserved and frees its underlying
buffer. Covers the case of a one-time memory reserved container that does not
otherwise have permissions over its own memory to resize or free.
@param[in] container_pointer a pointer to the container.
@param[in] destructor_and_free_arguments optional destructor function to be
called on each element and the required allocation function to free memory.
@return the result of the operation.

See container documentation for specific behavior. */
#define CCC_clear_and_free_reserve(container_pointer,                          \
                                   destructor_and_free_arguments...)           \
    CCC_private_clear_and_free_reserve(container_pointer,                      \
                                       destructor_and_free_arguments)

/**@}*/

/** @name State Interface
Obtain the container state. */
/**@{*/

/** @brief Return the count of elements in the container.
@param[in] container_pointer a pointer to the container.
@return the size or an argument error is set if container_pointer is NULL.

See container documentation for specific behavior. */
#define CCC_count(container_pointer) CCC_private_count(container_pointer)

/** @brief Return the capacity of the container.
@param[in] container_pointer a pointer to the container.
@return the capacity or an argument error is set if container_pointer is NULL.

See container documentation for specific behavior. */
#define CCC_capacity(container_pointer) CCC_private_capacity(container_pointer)

/** @brief Return the size status of a container.
@param[in] container_pointer a pointer to the container.
@return true if empty or NULL false if not.

See container documentation for specific behavior. */
#define CCC_is_empty(container_pointer) CCC_private_is_empty(container_pointer)

/** @brief Return the invariant statuses of the container.
@param[in] container_pointer a pointer to the container.
@return true if all invariants hold, false if not.

See container documentation for specific behavior. */
#define CCC_validate(container_pointer) CCC_private_validate(container_pointer)

/**@}*/

/** Define this preprocessor directive to shorten trait names. */
#ifdef TRAITS_USING_NAMESPACE_CCC
/* NOLINTBEGIN(readability-identifier-naming) */
#    define swap_entry(arguments...) CCC_swap_entry(arguments)
#    define swap_entry_wrap(arguments...) CCC_swap_entry_wrap(arguments)
#    define swap_handle(arguments...) CCC_swap_handle(arguments)
#    define swap_handle_wrap(arguments...) CCC_swap_handle_wrap(arguments)
#    define try_insert(arguments...) CCC_try_insert(arguments)
#    define insert_or_assign(arguments...) CCC_insert_or_assign(arguments)
#    define insert_or_assign_wrap(arguments...)                                \
        CCC_insert_or_assign_wrap(arguments)
#    define try_insert_wrap(arguments...) CCC_try_insert_wrap(arguments)
#    define remove_key_value(arguments...) CCC_remove_key_value(arguments)
#    define remove_key_value_wrap(arguments...)                                \
        CCC_remove_key_value_wrap(arguments)
#    define remove_entry(arguments...) CCC_remove_entry(arguments)
#    define remove_entry_wrap(arguments...) CCC_remove_entry_wrap(arguments)
#    define remove_handle(arguments...) CCC_remove_handle(arguments)
#    define remove_handle_wrap(arguments...) CCC_remove_handle_wrap(arguments)
#    define entry(arguments...) CCC_entry(arguments)
#    define entry_wrap(arguments...) CCC_entry_wrap(arguments)
#    define handle(arguments...) CCC_handle(arguments)
#    define handle_wrap(arguments...) CCC_handle_wrap(arguments)
#    define or_insert(arguments...) CCC_or_insert(arguments)
#    define insert_entry(arguments...) CCC_insert_entry(arguments)
#    define insert_handle(arguments...) CCC_insert_handle(arguments)
#    define and_modify(arguments...) CCC_and_modify(arguments)
#    define and_context_modify(arguments...) CCC_and_context_modify(arguments)
#    define occupied(arguments...) CCC_occupied(arguments)
#    define insert_error(arguments...) CCC_insert_error(arguments)
#    define unwrap(arguments...) CCC_unwrap(arguments)

#    define push(arguments...) CCC_push(arguments)
#    define push_back(arguments...) CCC_push_back(arguments)
#    define push_front(arguments...) CCC_push_front(arguments)
#    define pop(arguments...) CCC_pop(arguments)
#    define pop_front(arguments...) CCC_pop_front(arguments)
#    define pop_back(arguments...) CCC_pop_back(arguments)
#    define front(arguments...) CCC_front(arguments)
#    define back(arguments...) CCC_back(arguments)
#    define update(arguments...) CCC_update(arguments)
#    define increase(arguments...) CCC_increase(arguments)
#    define decrease(arguments...) CCC_decrease(arguments)
#    define erase(arguments...) CCC_erase(arguments)
#    define extract(arguments...) CCC_extract(arguments)
#    define extract_range(arguments...) CCC_extract_range(arguments)

#    define get_key_value(arguments...) CCC_get_key_value(arguments)
#    define get_mut(arguments...) CCC_get_key_value_mut(arguments)
#    define contains(arguments...) CCC_contains(arguments)

#    define begin(arguments...) CCC_begin(arguments)
#    define reverse_begin(arguments...) CCC_reverse_begin(arguments)
#    define next(arguments...) CCC_next(arguments)
#    define reverse_next(arguments...) CCC_reverse_next(arguments)
#    define end(arguments...) CCC_end(arguments)
#    define reverse_end(arguments...) CCC_reverse_end(arguments)

#    define equal_range(arguments...) CCC_equal_range(arguments)
#    define equal_range_reverse(arguments...) CCC_equal_range_reverse(arguments)
#    define equal_range_wrap(arguments...) CCC_equal_range_wrap(arguments)
#    define equal_range_reverse_wrap(arguments...)                             \
        CCC_equal_range_reverse_wrap(arguments)
#    define range_begin(arguments...) CCC_range_begin(arguments)
#    define range_end(arguments...) CCC_range_end(arguments)
#    define range_reverse_begin(arguments...) CCC_range_reverse_begin(arguments)
#    define range_reverse_end(arguments...) CCC_range_reverse_end(arguments)
#    define splice(arguments...) CCC_splice(arguments)
#    define splice_range(arguments...) CCC_splice_range(arguments)

#    define copy(arguments...) CCC_copy(arguments)
#    define reserve(arguments...) CCC_reserve(arguments)
#    define clear(arguments...) CCC_clear(arguments)
#    define clear_and_free(arguments...) CCC_clear_and_free(arguments)
#    define clear_and_free_reserve(arguments...)                               \
        CCC_clear_and_free_reserve(arguments)

#    define count(arguments...) CCC_count(arguments)
#    define capacity(arguments...) CCC_capacity(arguments)
#    define is_empty(arguments...) CCC_is_empty(arguments)
#    define validate(arguments...) CCC_validate(arguments)
/* NOLINTEND(readability-identifier-naming) */
#endif /* TRAITS_USING_NAMESPACE_CCC */

#endif /* CCC_TRAITS_H */
