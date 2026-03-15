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
@brief The Flat Priority Queue Interface

A flat priority queue is a contiguous container storing elements in heap order.
This offers tightly packed data for efficient push, pop, min/max operations in
`O(lg N)` time.

A flat priority queue can use memory sources from the stack, heap, or data
segment and can be initialized at compile or runtime. The container offers
efficient initialization options such as an `O(N)` heap building initializer.
The flat priority queue also offers a destructive heap sort option if the user
desires an in-place strict `O(N * log(N))` and `O(1)` space sort that does not
use recursion.

Many functions in the interface request a temporary argument be passed as a swap
slot. This is because a flat priority queue is backed by a binary heap and
swaps elements to maintain its properties. Because the user may decide the
flat priority queue has no allocation permission, the user must provide this
swap slot. An easy way to do this in C99 and later is with anonymous compound
literal references. For example, if we have a `int` flat priority queue we can
provide a temporary slot inline to a function as follows.

```
CCC_flat_priority_queue_pop(&priority_queue, &(int){});
```

Any user defined struct can also use this technique.

```
CCC_flat_priority_queue_pop(&priority_queue, &(struct My_type){});
```

This is the preferred method because the storage remains anonymous and
inaccessible to other code in the calling scope.

To shorten names in the interface, define the following preprocessor directive
at the top of your file.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
```

All types and functions can then be written without the `CCC_` prefix. */
#ifndef CCC_FLAT_PRIORITY_QUEUE_H
#define CCC_FLAT_PRIORITY_QUEUE_H

/** @cond */
#include <stddef.h>
/** @endcond */

#include "buffer.h"
#include "private/private_flat_priority_queue.h"
#include "types.h"

/** @name Container Types
Types available in the container interface. */
/**@{*/

/** @brief A container offering direct storage and sorting of user data by heap
order.
@warning it is undefined behavior to access an uninitialized container.

A flat priority queue can be initialized on the stack, heap, or data segment at
runtime or compile time.*/
typedef struct CCC_Flat_priority_queue CCC_Flat_priority_queue;

/**@}*/

/** @name Initialization Interface
Initialize the container with memory, callbacks, and permissions. */
/**@{*/

/** @brief Initialize an empty priority queue
@param[in] type_name the name of the user type.
@return the initialized priority queue on the right hand side of an equality
operator. */
#define CCC_flat_priority_queue_default(type_name)                             \
    CCC_private_flat_priority_queue_default(type_name)

/** @brief Initialize a priority_queue as a min or max heap.
@param[in] type_name the name of the user type.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max
heap, respectively.
@param[in] compare the user defined comarison function for user types.
@param[in] capacity the capacity of contiguous elements at data_pointer.
@param[in] data_pointer a pointer to an array of user types or NULL.
@return the initialized priority queue on the right hand side of an equality
operator. */
#define CCC_flat_priority_queue_for(type_name, order, compare, capacity,       \
                                    data_pointer)                              \
    CCC_private_flat_priority_queue_for(type_name, order, compare, capacity,   \
                                        data_pointer)

/** @brief Partial order an array of elements as a min or max heap at runtime
in O(N) time and space equal to the provided data capacity.
@param[in] type_name the name of the user type.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max
heap, respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] capacity the capacity of contiguous elements at data_pointer.
@param[in] count the count <= capacity of valid elements.
@param[in] data_pointer a pointer to an array of user types or NULL.
@return the initialized priority queue on the right hand side of an equality
operator.
@warning One additional element of the provided type is allocated on the stack
for swapping purposes. */
#define CCC_flat_priority_queue_heapify(type_name, order, compare, capacity,   \
                                        count, data_pointer...)                \
    CCC_private_flat_priority_queue_heapify(type_name, order, compare,         \
                                            capacity, count, data_pointer)

/** @brief Partial order an array of elements as a min or max heap at runtime
in O(N) time and space equal to the provided data capacity. Intended for
converting storage into a fixed capacity flat priority queue with no allocation
permission and no context for comparison.
@param[in] type_name the name of the user type.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max
heap, respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] capacity the capacity of contiguous elements at data_pointer.
@param[in] count the count <= capacity of valid elements.
@param[in] data_pointer a pointer to an array of user types or NULL.
@return the initialized priority queue on the right hand side of an equality
operator.
@warning One additional element of the provided type is allocated on the stack
for swapping purposes. */
#define CCC_flat_priority_queue_heapify_storage(                               \
    type_name, order, compare, capacity, count, data_pointer...)               \
    CCC_private_flat_priority_queue_heapify_storage(                           \
        type_name, order, compare, capacity, count, data_pointer)

/** @brief Partial order an array of elements as a min or max heap at runtime
in O(N) time and space equal to the provided data capacity. Intended for
converting storage into a fixed capacity flat priority queue with no allocation
permission but context for comparison.
@param[in] type_name the name of the user type.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max
heap, respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] context any context data needed for destruction of elements.
@param[in] capacity the capacity of contiguous elements at data_pointer.
@param[in] count the count <= capacity of valid elements.
@param[in] data_pointer a pointer to an array of user types or NULL.
@return the initialized priority queue on the right hand side of an equality
operator.
@warning One additional element of the provided type is allocated on the stack
for swapping purposes. */
#define CCC_flat_priority_queue_context_heapify_storage(                       \
    type_name, order, compare, context, capacity, count, data_pointer...)      \
    CCC_private_flat_priority_queue_context_heapify_storage(                   \
        type_name, order, compare, context, capacity, count, data_pointer)

/** @brief Partial order a compound literal array of elements as a min or max
heap. O(N).
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max heap,
respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] allocate the allocation function or NULL if no allocation.
@param[in] optional_capacity the optional capacity larger than the input
compound literal array array to reserve. If capacity provided is less than the
size of the input compound literal array, the capacity is set to the size of the
input compound literal array. If not needed, simply leave as zero.
@param[in] compound_literal_array the initializer of the type stored in flat
priority queue (e.g. `(int[]){1,2,3}`).
@return the initialized priority queue on the right hand side of an equality
operator.
@warning One additional element of the provided type is allocated on the stack
for swapping purposes.

Initialize a dynamic Flat_priority_queue with capacity equal to size.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
int
main(void)
{
    Flat_priority_queue f = flat_priority_queue_from(
        CCC_ORDER_LESSER,
        compare_ints,
        std_allocate,
        0,
        (int[]){6, 99, 32, 44, 1, 0}
    );
    return 0;
}
```

Initialize a dynamic Flat_priority_queue with a large capacity.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
int
main(void)
{
    Flat_priority_queue f = flat_priority_queue_from(
        CCC_ORDER_LESSER,
        compare_ints,
        std_allocate,
        4096,
        (int[]){6, 99, 32, 44, 1, 0}
    );
    return 0;
}
```

Only dynamic priority queues may be initialized this way. For static or stack
based initialization of fixed capacity compound literals with no elements see
the CCC_flat_priority_queue_with_storage() macro. */
#define CCC_flat_priority_queue_from(                                          \
    order, compare, allocate, optional_capacity, compound_literal_array...)    \
    CCC_private_flat_priority_queue_from(                                      \
        order, compare, allocate, optional_capacity, compound_literal_array)

/** @brief Partial order a compound literal array of elements as a min or max
heap. O(N).
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max heap,
respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] allocate the allocation function or NULL if no allocation.
@param[in] context any context data needed for destruction of elements.
@param[in] optional_capacity the optional capacity larger than the input
compound literal array array to reserve. If capacity provided is less than the
size of the input compound literal array, the capacity is set to the size of the
input compound literal array. If not needed, simply leave as zero.
@param[in] compound_literal_array the initializer of the type stored in flat
priority queue (e.g. `(int[]){1,2,3}`).
@return the initialized priority queue on the right hand side of an equality
operator.
@warning One additional element of the provided type is allocated on the stack
for swapping purposes.

Initialize a dynamic Flat_priority_queue with capacity equal to size.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
int
main(void)
{
    Flat_priority_queue f = flat_priority_queue_context_from(
        CCC_ORDER_LESSER,
        compare_ints,
        arena_allocate,
        &arena,
        0,
        (int[]){6, 99, 32, 44, 1, 0}
    );
    return 0;
}
```

Initialize a dynamic Flat_priority_queue with a large capacity.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
int
main(void)
{
    Flat_priority_queue f = flat_priority_queue_context_from(
        CCC_ORDER_LESSER,
        compare_ints,
        arena_allocate,
        &arena,
        4096,
        (int[]){6, 99, 32, 44, 1, 0}
    );
    return 0;
}
```

Only dynamic priority queues may be initialized this way. For static or stack
based initialization of fixed capacity compound literals with no elements see
the CCC_flat_priority_queue_with_storage() macro. */
#define CCC_flat_priority_queue_context_from(order, compare, allocate,         \
                                             context, optional_capacity,       \
                                             compound_literal_array...)        \
    CCC_private_flat_priority_queue_context_from(order, compare, allocate,     \
                                                 context, optional_capacity,   \
                                                 compound_literal_array)

/** @brief Initialize a Flat_priority_queue with a capacity.
@param[in] type_name the name of the user type.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max
heap, respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] allocate the allocation function or NULL if no allocation.
@param[in] capacity the capacity of contiguous elements at data_pointer.
@return the initialized flat_priority_queue. Directly assign to
Flat_priority_queue on the right hand side of the equality operator.

Initialize a dynamic Flat_priority_queue.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
int
main(void)
{
    Flat_priority_queue f = flat_priority_queue_with_capacity(
        int,
        CCC_ORDER_LESSER,
        compare_ints,
        std_allocate,
        4096
    );
    return 0;
}
```

Only dynamic priority queues may be initialized this way. For static or stack
based initialization of fixed capacity compound literals with no elements see
the CCC_flat_priority_queue_with_storage() macro. */
#define CCC_flat_priority_queue_with_capacity(type_name, order, compare,       \
                                              allocate, capacity)              \
    CCC_private_flat_priority_queue_with_capacity(type_name, order, compare,   \
                                                  allocate, capacity)

/** @brief Initialize a Flat_priority_queue with a capacity.
@param[in] type_name the name of the user type.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max
heap, respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] allocate the allocation function or NULL if no allocation.
@param[in] context any context data needed for destruction of elements.
@param[in] capacity the capacity of contiguous elements at data_pointer.
@return the initialized flat_priority_queue. Directly assign to
Flat_priority_queue on the right hand side of the equality operator.

Initialize a dynamic Flat_priority_queue.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
int
main(void)
{
    Flat_priority_queue f = flat_priority_queue_context_with_capacity(
        int,
        CCC_ORDER_LESSER,
        compare_ints,
        arena_allocate,
        &arena,
        4096
    );
    return 0;
}
```

Only dynamic priority queues may be initialized this way. For static or stack
based initialization of fixed capacity compound literals with no elements see
the CCC_flat_priority_queue_with_storage() macro. */
#define CCC_flat_priority_queue_context_with_capacity(                         \
    type_name, order, compare, allocate, context, capacity)                    \
    CCC_private_flat_priority_queue_context_with_capacity(                     \
        type_name, order, compare, allocate, context, capacity)

/** @brief Initialize a priority_queue as a min or max heap with no allocation
permission, no context data, and a compound literal as backing storage.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max heap,
respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] compound_literal_array the compound literal array of fixed capacity.
@return the initialized priority queue on the right hand side of an equality
operator. Capacity of the compound literal is capacity of the priority queue.
@warning The compound literal is NOT swapped into heap order upon
initialization. This initializer is meant for compile or runtime initialization
with a fixed capacity compound literal with a count of 0. */
#define CCC_flat_priority_queue_with_storage(order, compare,                   \
                                             compound_literal_array)           \
    CCC_private_flat_priority_queue_with_storage(order, compare,               \
                                                 compound_literal_array)

/** @brief Initialize a priority_queue as a min or max heap with no allocation
permission, context data, and a compound literal as backing storage.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max heap,
respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] context any context needed by the priority queue.
@param[in] compound_literal_array the compound literal array of fixed capacity.
@return the initialized priority queue on the right hand side of an equality
operator.
@warning The compound literal is NOT swapped into heap order upon
initialization. This initializer is meant for compile or runtime initialization
with a fixed capacity compound literal with a count of 0. */
#define CCC_flat_priority_queue_context_with_storage(order, compare, context,  \
                                                     compound_literal_array)   \
    CCC_private_flat_priority_queue_context_with_storage(                      \
        order, compare, context, compound_literal_array)

/** @brief Initialize an empty dynamic queue at compile or runtime with an
allocator.
@param[in] type_name the name of the type stored in the queue.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max heap,
respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] allocate the compound literal array of fixed capacity.
@return the initialized priority queue on the right hand side of an equality
operator. */
#define CCC_flat_priority_queue_with_allocator(type_name, order, compare,      \
                                               allocate)                       \
    CCC_private_flat_priority_queue_with_allocator(type_name, order, compare,  \
                                                   allocate)

/** @brief Initialize an empty dynamic queue at compile or runtime with an
allocator with context.
@param[in] type_name the name of the type stored in the queue.
@param[in] order CCC_ORDER_LESSER or CCC_ORDER_GREATER for min or max heap,
respectively.
@param[in] compare the user defined comparison function for user types.
@param[in] allocate the compound literal array of fixed capacity.
@param[in] context the context for allocator.
@return the initialized priority queue on the right hand side of an equality
operator. */
#define CCC_flat_priority_queue_context_with_allocator(                        \
    type_name, order, compare, allocate, context)                              \
    CCC_private_flat_priority_queue_context_with_allocator(                    \
        type_name, order, compare, allocate, context)

/** @brief Copy the priority_queue from source to newly initialized
destination.
@param[in] destination the destination that will copy the source
flat_priority_queue.
@param[in] source the source of the flat_priority_queue.
@param[in] allocate the allocation function in case resizing of destination is
needed.
@return the result of the copy operation. If the destination capacity is less
than the source capacity and no allocation function is provided an input error
is returned. If resizing is required and resizing of destination fails a memory
error is returned.
@note destination must have capacity greater than or equal to source. If
destination capacity is less than source, an allocation function must be
provided with the allocate argument.

Note that there are two ways to copy data from source to destination: provide
sufficient memory and pass NULL as allocate, or allow the copy function to take
care of allocation for the copy.

Manual memory management with no allocation function provided.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
Flat_priority_queue source = flat_priority_queue_for(
    int,
    CCC_ORDER_LESSER,
    int_order,
    NULL,
    NULL,
    10,
    (int[10]){}
);
push_rand_ints(&source);
Flat_priority_queue destination = flat_priority_queue_for(
    int,
    CCC_ORDER_LESSER,
    int_order,
    NULL,
    NULL,
    11,
    (int[11]){}
);
CCC_Result res = flat_priority_queue_copy(&destination, &source, NULL);
```

The above requires destination capacity be greater than or equal to source
capacity. Here is memory management handed over to the copy function.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
Flat_priority_queue source = flat_priority_queue_for(
    int,
    CCC_ORDER_LESSER,
    int_order,
    std_allocate,
    NULL,
    0,
    NULL
);
push_rand_ints(&source);
Flat_priority_queue destination = flat_priority_queue_for(
    int,
    CCC_ORDER_LESSER,
    int_order,
    std_allocate,
    NULL,
    0,
    NULL
);
CCC_Result res = flat_priority_queue_copy(&destination, &source, std_allocate);
```

The above allows destination to have a capacity less than that of the source as
long as copy has been provided an allocation function to resize destination.
Note that this would still work if copying to a destination that the user wants
as a fixed size flat_priority_queue.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
Flat_priority_queue source = flat_priority_queue_for(
    int,
    CCC_ORDER_LESSER,
    int_order,
    std_allocate,
    NULL,
    0,
    NULL
);
push_rand_ints(&source);
Flat_priority_queue destination = flat_priority_queue_for(
    int,
    CCC_ORDER_LESSER,
    int_order,
    NULL,
    NULL,
    0,
    NULL
);
CCC_Result res = flat_priority_queue_copy(&destination, &source, std_allocate);
```

The above sets up destination with fixed size while source is a dynamic
flat_priority_queue. Because an allocation function is provided, the destination
is resized once for the copy and retains its fixed size after the copy is
complete. This would require the user to manually free the underlying Buffer at
destination eventually if this method is used. Usually it is better to allocate
the memory explicitly before the copy if copying between ring buffers.

These options allow users to stay consistent across containers with their
memory management strategies. */
CCC_Result CCC_flat_priority_queue_copy(CCC_Flat_priority_queue *destination,
                                        CCC_Flat_priority_queue const *source,
                                        CCC_Allocator *allocate);

/** @brief Reserves space for at least to_add more elements.
@param[in] priority_queue a pointer to the flat priority queue.
@param[in] to_add the number of elements to add to the current size.
@param[in] allocate the allocation function to use to reserve memory.
@return the result of the reservation. OK if successful, otherwise an error
status is returned.
@note see the CCC_flat_priority_queue_clear_and_free_reserve function if this
function is being used for a one-time dynamic reservation.

This function can be used for a dynamic priority_queue with or without
allocation permission. If the priority_queue has allocation permission, it
will reserve the required space and later resize if more space is needed.

If the priority_queue has been initialized with no allocation permission
and no memory this function can serve as a one-time reservation. This is helpful
when a fixed size is needed but that size is only known dynamically at runtime.
To free the priority_queue in such a case see the
CCC_flat_priority_queue_clear_and_free_reserve function. */
CCC_Result
CCC_flat_priority_queue_reserve(CCC_Flat_priority_queue *priority_queue,
                                size_t to_add, CCC_Allocator *allocate);

/**@}*/

/** @name Insert and Remove Interface
Insert or remove elements from the flat priority queue. */
/**@{*/

/** @brief Write a type directly to a priority queue slot. O(lgN).
@param[in] priority_queue_pointer a pointer to the priority queue.
@param[in] type_compound_literal the compound literal or direct scalar type.
@return a reference to the inserted element or NULL if allocation failed. */
#define CCC_flat_priority_queue_emplace(priority_queue_pointer,                \
                                        type_compound_literal...)              \
    CCC_private_flat_priority_queue_emplace(priority_queue_pointer,            \
                                            type_compound_literal)

/** @brief Copy input buffer into the flat priority queue, organizing into data
into heap order in O(N) time.
@param[in] priority_queue a pointer to the priority queue.
@param[in] buffer a pointer to the buffer of types to copy into the flat
priority queue and heapify.
@param[in] temp a pointer to an additional element of array type for swapping.
@return OK if ordering was successful or an input error if bad input is
provided. A permission error will occur if no allocation is allowed and the
input buffer is larger than the flat priority queue capacity. A memory
error will occur if reallocation is required to fit all elements but
reallocation fails.
@warning Assumes the input buffer has been initialized correctly via its
interface.
@warning Any elements in the original flat priority queue are overwritten or
lost when the buffer contents are copied.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(My_type){}`.

Note that this version of heapify copies elements from the input buffer. If an
in place heapify is required see any of the following functions.

```
CCC_flat_priority_queue_in_place_heapify()
CCC_flat_priority_queue_heapify()
CCC_flat_priority_queue_context_heapify_storage()
CCC_flat_priority_queue_heapify_storage()
```

This function does not modify the input buffer. */
CCC_Result
CCC_flat_priority_queue_copy_heapify(CCC_Flat_priority_queue *priority_queue,
                                     CCC_Buffer const *buffer, void *temp);

/** @brief Order count elements of the input Buffer as a flat priority queue,
destroying the input metadata Buffer struct taking ownership of its underlying
memory.
@param[in] buffer a pointer to a buffer with memory that will be sorted into
heap order, given to the flat priority queue, and its metadata struct will be
cleared.
@param[in] order the order of the heap, minimum or maximum priority queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@return a flat priority queue that now owns the underlying buffer storage and
is in correct heap order. If an error occurs all fields are set to 0 or NULL
and the order of the priority queue is set to CCC_ORDER_ERROR. The order can
be read with CCC_flat_priority_queue_order(). If an error occurs, the buffer
remains unmodified.
@warning Assumed the buffer has been correctly initialized.
@warning All fields in the input buffer are cleared, zeroed, or set to NULL.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
CCC_Flat_priority_queue
CCC_flat_priority_queue_in_place_heapify(CCC_Buffer *buffer, CCC_Order order,
                                         CCC_Comparator *compare, void *temp);

/** @brief Order count elements of the input Buffer as a flat priority queue,
destroying the input metadata Buffer struct taking ownership of its underlying
memory. Provide context for the comparison function.
@param[in] buffer a pointer to a buffer with memory that will be sorted into
heap order, given to the flat priority queue, and its metadata struct will be
cleared.
@param[in] order the order of the heap, minimum or maximum priority queue.
@param[in] context a pointer to context needed for the comparator.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@return a flat priority queue that now owns the underlying buffer storage and
is in correct heap order. If an error occurs all fields are set to 0 or NULL
and the order of the priority queue is set to CCC_ORDER_ERROR. The order can
be read with CCC_flat_priority_queue_order(). If an error occurs, the buffer
remains unmodified.
@warning Assumed the buffer has been correctly initialized.
@warning All fields in the input buffer are cleared, zeroed, or set to NULL.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
CCC_Flat_priority_queue CCC_flat_priority_queue_context_in_place_heapify(
    CCC_Buffer *buffer, CCC_Order order, CCC_Comparator *compare, void *context,
    void *temp);

/** @brief Pushes element pointed to at e into flat_priority_queue. O(lgN).
@param[in] priority_queue a pointer to the priority queue.
@param[in] type a pointer to the user element of same type as in
flat_priority_queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@return a pointer to the inserted element or NULl if NULL arguments are provided
or push required more memory and failed. Failure can occur if the
flat_priority_queue is full and allocation is not allowed or a resize failed
when allocation is allowed.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
[[nodiscard]] void *
CCC_flat_priority_queue_push(CCC_Flat_priority_queue *priority_queue,
                             void const *type, void *temp);

/** @brief Pop the front element (min or max) element in the
flat_priority_queue. O(lgN).
@param[in] priority_queue a pointer to the priority queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@return OK if the pop succeeds or an input error if priority_queue is NULL
or empty.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
CCC_Result CCC_flat_priority_queue_pop(CCC_Flat_priority_queue *priority_queue,
                                       void *temp);

/** @brief Erase element e that is a handle to the stored flat_priority_queue
element.
@param[in] priority_queue a pointer to the priority queue.
@param[in] type a pointer to the stored priority_queue element. Must be in
the flat_priority_queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@return OK if the erase is successful or an input error if NULL arguments are
provided or the priority_queue is empty.
@warning the user must ensure e is in the flat_priority_queue.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`.

Note that the reference to type is invalidated after this call. */
CCC_Result
CCC_flat_priority_queue_erase(CCC_Flat_priority_queue *priority_queue,
                              void *type, void *temp);

/** @brief Update e that is a handle to the stored priority_queue element.
O(lgN).
@param[in] priority_queue a pointer to the flat priority queue.
@param[in] type a pointer to the stored priority_queue element. Must be in
the flat_priority_queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@param[in] modify the update function to act on e.
@param[in] context any context data needed for the update function.
@return a reference to the element at its new position in the
flat_priority_queue on success, NULL if parameters are invalid or
flat_priority_queue is empty.
@warning the user must ensure e is in the flat_priority_queue.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
void *CCC_flat_priority_queue_update(CCC_Flat_priority_queue *priority_queue,
                                     void *type, void *temp,
                                     CCC_Modifier *modify, void *context);

/** @brief Update the user type stored in the priority queue directly. O(lgN).
@param[in] priority_queue_pointer a pointer to the flat priority queue.
@param[in] type_pointer a pointer to the user type being updated.
@param[in] update_closure_over_T the semicolon separated statements to execute
on the user type at T (optionally wrapping {code here} in braces may help
with formatting). This closure may safely modify the key used to track the user
element's priority in the priority queue.
@return a reference to the element at its new position in the
flat_priority_queue on success, NULL if parameters are invalid or
flat_priority_queue is empty.
@warning the user must ensure type_pointer is in the flat_priority_queue.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
Flat_priority_queue priority_queue = build_rand_int_flat_priority_queue();
(void)flat_priority_queue_update_with(&flat_priority_queue,
get_rand_flat_priority_queue_node(&flat_priority_queue), { *T = rand_key(); });
```

Note that whether the key increases or decreases does not affect runtime. */
#define CCC_flat_priority_queue_update_with(                                   \
    priority_queue_pointer, type_pointer, update_closure_over_T...)            \
    CCC_private_flat_priority_queue_update_with(                               \
        priority_queue_pointer, type_pointer, update_closure_over_T)

/** @brief Increase e that is a handle to the stored flat_priority_queue
element. O(lgN).
@param[in] priority_queue a pointer to the flat priority queue.
@param[in] type a pointer to the stored priority_queue element. Must be in
the flat_priority_queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@param[in] modify the update function to act on e.
@param[in] context any context data needed for the update function.
@return a reference to the element at its new position in the
flat_priority_queue on success, NULL if parameters are invalid or
flat_priority_queue is empty.
@warning the user must ensure e is in the flat_priority_queue.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
void *CCC_flat_priority_queue_increase(CCC_Flat_priority_queue *priority_queue,
                                       void *type, void *temp,
                                       CCC_Modifier *modify, void *context);

/** @brief Increase the user type stored in the priority queue directly. O(lgN).
@param[in] flat_priority_queue_pointer a pointer to the flat priority queue.
@param[in] type_pointer a pointer to the user type being updated.
@param[in] increase_closure_over_T the semicolon separated statements to
execute on the user type at T (optionally wrapping {code here} in
braces may help with formatting). This closure may safely modify the key used to
track the user element's priority in the priority queue.
@return a reference to the element at its new position in the
flat_priority_queue on success, NULL if parameters are invalid or
flat_priority_queue is empty.
@warning the user must ensure type_pointer is in the flat_priority_queue.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
Flat_priority_queue priority_queue = build_rand_int_flat_priority_queue();
(void)flat_priority_queue_increase_with(&flat_priority_queue,
get_rand_flat_priority_queue_node(&flat_priority_queue), { (*T)++; });
```

Note that if this priority queue is min or max, the runtime is the same. */
#define CCC_flat_priority_queue_increase_with(                                 \
    flat_priority_queue_pointer, type_pointer, increase_closure_over_T...)     \
    CCC_private_flat_priority_queue_increase_with(                             \
        flat_priority_queue_pointer, type_pointer, increase_closure_over_T)

/** @brief Decrease e that is a handle to the stored flat_priority_queue
element. O(lgN).
@param[in] priority_queue a pointer to the flat priority queue.
@param[in] type a pointer to the stored priority_queue element. Must be in
the flat_priority_queue.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@param[in] modify the update function to act on e.
@param[in] context any context data needed for the update function.
@return a reference to the element at its new position in the
flat_priority_queue on success, NULL if parameters are invalid or
flat_priority_queue is empty.
@warning the user must ensure e is in the flat_priority_queue.

A simple way to provide a temp for swapping is with an inline compound literal
reference provided directly to the function argument `&(name_of_type){}`. */
void *CCC_flat_priority_queue_decrease(CCC_Flat_priority_queue *priority_queue,
                                       void *type, void *temp,
                                       CCC_Modifier *modify, void *context);

/** @brief Increase the user type stored in the priority queue directly. O(lgN).
@param[in] flat_priority_queue_pointer a pointer to the flat priority queue.
@param[in] type_pointer a pointer to the user type being updated.
@param[in] decrease_closure_over_T the semicolon separated statements to
execute on the user type at T (optionally wrapping {code here} in
braces may help with formatting). This closure may safely modify the key used to
track the user element's priority in the priority queue.
@return a reference to the element at its new position in the
flat_priority_queue on success, NULL if parameters are invalid or
flat_priority_queue is empty.
@warning the user must ensure type_pointer is in the flat_priority_queue.

```
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
Flat_priority_queue priority_queue = build_rand_int_flat_priority_queue();
(void)flat_priority_queue_decrease_with(&flat_priority_queue,
get_rand_flat_priority_queue_node(&flat_priority_queue), { (*T)--; });
```

Note that if this priority queue is min or max, the runtime is the same. */
#define CCC_flat_priority_queue_decrease_with(                                 \
    flat_priority_queue_pointer, type_pointer, decrease_closure_over_T...)     \
    CCC_private_flat_priority_queue_decrease_with(                             \
        flat_priority_queue_pointer, type_pointer, decrease_closure_over_T)

/**@}*/

/** @name Deallocation Interface
Deallocate the container or destroy the heap invariants. */
/**@{*/

/** @brief Clears the priority_queue calling destroy on every element if
provided. O(1)-O(N).
@param[in] priority_queue a pointer to the flat priority queue.
@param[in] destroy the destructor function or NULL if not needed.
@return OK if input is valid and clear succeeds, otherwise input error.

Note that because the priority queue is flat there is no need to free
elements stored in the flat_priority_queue. However, the destructor is free to
manage cleanup in other parts of user code as needed upon destruction of each
element.

If the destructor is NULL, the function is O(1) and no attempt is made to
free capacity of the flat_priority_queue. */
CCC_Result
CCC_flat_priority_queue_clear(CCC_Flat_priority_queue *priority_queue,
                              CCC_Destructor *destroy);

/** @brief Clears the priority_queue calling destroy on every element if
provided and frees the underlying buffer. O(1)-O(N).
@param[in] priority_queue a pointer to the flat priority queue.
@param[in] destroy the destructor function or NULL if not needed.
@return OK if input is valid and clear succeeds, otherwise input error. If the
Buffer attempts to free but is not allowed a no allocate error is returned.

Note that because the priority queue is flat there is no need to free elements
stored in the flat_priority_queue. However, the destructor is free to manage
cleanup in other parts of user code as needed upon destruction of each element.

If the destructor is NULL, the function is O(1) and only relies on the runtime
of the provided allocation function free operation. */
CCC_Result
CCC_flat_priority_queue_clear_and_free(CCC_Flat_priority_queue *priority_queue,
                                       CCC_Destructor *destroy);

/** @brief Frees all slots in the priority_queue and frees the underlying
Buffer that was previously dynamically reserved with the reserve function.
@param[in] priority_queue the priority_queue to be cleared.
@param[in] destructor the destructor for each element. NULL can be passed if no
maintenance is required on the elements in the priority_queue before their
slots are dropped.
@param[in] allocate the required allocation function to provide to a
dynamically reserved flat_priority_queue. Any context data provided upon
initialization will be passed to the allocation function when called.
@return the result of free operation. OK if success, or an error status to
indicate the error.
@warning It is an error to call this function on a priority_queue that was
not reserved with the provided CCC_Allocator. The priority_queue must have
existing memory to free.

This function covers the edge case of reserving a dynamic capacity for a
flat_priority_queue at runtime but denying the priority_queue allocation
permission to resize. This can help prevent a priority_queue from growing
untree. The user in this case knows the priority_queue does not have
allocation permission and therefore no further memory will be dedicated to the
flat_priority_queue.

However, to free the priority_queue in such a case this function must be
used because the priority_queue has no ability to free itself. Just as the
allocation function is required to reserve memory so to is it required to free
memory.

This function will work normally if called on a priority_queue with
allocation permission however the normal CCC_flat_priority_queue_clear_and_free
is sufficient for that use case. */
CCC_Result CCC_flat_priority_queue_clear_and_free_reserve(
    CCC_Flat_priority_queue *priority_queue, CCC_Destructor *destructor,
    CCC_Allocator *allocate);

/**@}*/

/** @name State Interface
Obtain state from the container. */
/**@{*/

/** @brief Return a pointer to the front (min or max) element in the
flat_priority_queue. O(1).
@param[in] priority_queue a pointer to the priority queue.
@return A pointer to the front element or NULL if empty or flat_priority_queue
is NULL. */
[[nodiscard]] void *
CCC_flat_priority_queue_front(CCC_Flat_priority_queue const *priority_queue);

/** @brief Returns true if the priority_queue is empty false if not. O(1).
@param[in] priority_queue a pointer to the flat priority queue.
@return true if the size is 0, false if not empty. Error if flat_priority_queue
is NULL. */
[[nodiscard]] CCC_Tribool
CCC_flat_priority_queue_is_empty(CCC_Flat_priority_queue const *priority_queue);

/** @brief Returns the count of the priority_queue active slots.
@param[in] priority_queue a pointer to the flat priority queue.
@return the size of the priority_queue or an argument error is set if
flat_priority_queue is NULL. */
[[nodiscard]] CCC_Count
CCC_flat_priority_queue_count(CCC_Flat_priority_queue const *priority_queue);

/** @brief Returns the capacity of the priority_queue representing total
possible slots.
@param[in] priority_queue a pointer to the flat priority queue.
@return the capacity of the priority_queue or an argument error is set if
flat_priority_queue is NULL. */
[[nodiscard]] CCC_Count
CCC_flat_priority_queue_capacity(CCC_Flat_priority_queue const *priority_queue);

/** @brief Return a pointer to the base of the backing array. O(1).
@param[in] priority_queue a pointer to the priority queue.
@return A pointer to the base of the backing array or NULL if
flat_priority_queue is NULL.
@note this reference starts at index 0 of the backing array. All
flat_priority_queue elements are stored contiguously starting at the base
through size of the flat_priority_queue.
@warning it is the users responsibility to ensure that access to any data is
within the capacity of the backing buffer. */
[[nodiscard]] void *
CCC_flat_priority_queue_data(CCC_Flat_priority_queue const *priority_queue);

/** @brief Verifies the internal invariants of the priority_queue hold.
@param[in] priority_queue a pointer to the flat priority queue.
@return true if the priority_queue is valid false if invalid. Error if
flat_priority_queue is NULL. */
[[nodiscard]] CCC_Tribool
CCC_flat_priority_queue_validate(CCC_Flat_priority_queue const *priority_queue);

/** @brief Return the order used to initialize the flat_priority_queue.
@param[in] priority_queue a pointer to the flat priority queue.
@return LES or GRT ordering. Any other ordering is invalid. */
[[nodiscard]] CCC_Order
CCC_flat_priority_queue_order(CCC_Flat_priority_queue const *priority_queue);

/**@}*/

/** Define this preprocessor directive if shortened names are desired for the
flat priority queue container. Check for collisions before name shortening. */
#ifdef FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
/* NOLINTBEGIN(readability-identifier-naming) */
typedef CCC_Flat_priority_queue Flat_priority_queue;
#    define flat_priority_queue_for(arguments...)                              \
        CCC_flat_priority_queue_for(arguments)
#    define flat_priority_queue_from(arguments...)                             \
        CCC_flat_priority_queue_from(arguments)
#    define flat_priority_queue_context_from(arguments...)                     \
        CCC_flat_priority_queue_context_from(arguments)
#    define flat_priority_queue_with_capacity(arguments...)                    \
        CCC_flat_priority_queue_with_capacity(arguments)
#    define flat_priority_queue_context_with_capacity(arguments...)            \
        CCC_flat_priority_queue_context_with_capacity(arguments)
#    define flat_priority_queue_with_storage(arguments...)                     \
        CCC_flat_priority_queue_with_storage(arguments)
#    define flat_priority_queue_context_with_storage(arguments...)             \
        CCC_flat_priority_queue_context_with_storage(arguments)
#    define flat_priority_queue_with_allocator(arguments...)                   \
        CCC_flat_priority_queue_with_allocator(arguments)
#    define flat_priority_queue_context_with_allocator(arguments...)           \
        CCC_flat_priority_queue_context_with_allocator(arguments)
#    define flat_priority_queue_heapify(arguments...)                          \
        CCC_flat_priority_queue_heapify(arguments)
#    define flat_priority_queue_heapify_storage(arguments...)                  \
        CCC_flat_priority_queue_heapify_storage(arguments)
#    define flat_priority_queue_context_heapify_storage(arguments...)          \
        CCC_flat_priority_queue_context_heapify_storage(arguments)
#    define flat_priority_queue_copy(arguments...)                             \
        CCC_flat_priority_queue_copy(arguments)
#    define flat_priority_queue_reserve(arguments...)                          \
        CCC_flat_priority_queue_reserve(arguments)
#    define flat_priority_queue_copy_heapify(arguments...)                     \
        CCC_flat_priority_queue_copy_heapify(arguments)
#    define flat_priority_queue_in_place_heapify(arguments...)                 \
        CCC_flat_priority_queue_in_place_heapify(arguments)
#    define flat_priority_queue_context_in_place_heapify(arguments...)         \
        CCC_flat_priority_queue_context_in_place_heapify(arguments)
#    define flat_priority_queue_emplace(arguments...)                          \
        CCC_flat_priority_queue_emplace(arguments)
#    define flat_priority_queue_push(arguments...)                             \
        CCC_flat_priority_queue_push(arguments)
#    define flat_priority_queue_front(arguments...)                            \
        CCC_flat_priority_queue_front(arguments)
#    define flat_priority_queue_pop(arguments...)                              \
        CCC_flat_priority_queue_pop(arguments)
#    define flat_priority_queue_extract(arguments...)                          \
        CCC_flat_priority_queue_extract(arguments)
#    define flat_priority_queue_update(arguments...)                           \
        CCC_flat_priority_queue_update(arguments)
#    define flat_priority_queue_increase(arguments...)                         \
        CCC_flat_priority_queue_increase(arguments)
#    define flat_priority_queue_decrease(arguments...)                         \
        CCC_flat_priority_queue_decrease(arguments)
#    define flat_priority_queue_update_with(arguments...)                      \
        CCC_flat_priority_queue_update_with(arguments)
#    define flat_priority_queue_increase_with(arguments...)                    \
        CCC_flat_priority_queue_increase_with(arguments)
#    define flat_priority_queue_decrease_with(arguments...)                    \
        CCC_flat_priority_queue_decrease_with(arguments)
#    define flat_priority_queue_clear(arguments...)                            \
        CCC_flat_priority_queue_clear(arguments)
#    define flat_priority_queue_clear_and_free(arguments...)                   \
        CCC_flat_priority_queue_clear_and_free(arguments)
#    define flat_priority_queue_clear_and_free_reserve(arguments...)           \
        CCC_flat_priority_queue_clear_and_free_reserve(arguments)
#    define flat_priority_queue_is_empty(arguments...)                         \
        CCC_flat_priority_queue_is_empty(arguments)
#    define flat_priority_queue_count(arguments...)                            \
        CCC_flat_priority_queue_count(arguments)
#    define flat_priority_queue_capacity(arguments...)                         \
        CCC_flat_priority_queue_capacity(arguments)
#    define flat_priority_queue_data(arguments...)                             \
        CCC_flat_priority_queue_data(arguments)
#    define flat_priority_queue_validate(arguments...)                         \
        CCC_flat_priority_queue_validate(arguments)
#    define flat_priority_queue_order(arguments...)                            \
        CCC_flat_priority_queue_order(arguments)
/* NOLINTEND(readability-identifier-naming) */
#endif /* FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC */

#endif /* CCC_FLAT_PRIORITY_QUEUE_H */
