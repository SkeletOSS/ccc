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
@brief The Array Tree Map Interface

An array tree map offers insertion, removal, and searching with a strict bound
of `O(log(N))` time. This map is suitable for realtime applications if resizing
can be well controlled. Insert operations may cause resizing if allocation is
allowed changing the insertion runtime to amortized `O(log(N))`. Searching is a
thread-safe read-only operation. Balancing modifications only occur upon
insertion or removal.

This array variant of the tree map promises contiguous storage and random
access if needed with handles. Handles are stable and the user can use them to
refer to an element until that element is removed from the map. Handles remain
valid even if resizing of the table, insertions of other elements, or removals
of other elements occur. Active user elements may not be contiguous from index
`[0, N)` where `N` is the size of map; there may be gaps between active elements
in the array and it is only guaranteed that `N` elements are stored between
index `[0, Capacity)`.

All elements in the map track their relationships via indices in the buffer.
Therefore, this data structure can be relocated, copied, serialized, or written
to disk and all internal data structure references will remain valid. Insertion
may invoke an `O(N)` operation if resizing occurs. Finally, if allocation is
prohibited upon initialization, and the user provides a capacity of `C` upon
initialization, one slot will be used for a sentinel node. The user available
capacity is `C - 1`.

All interface functions accept `void *` references to either the key or the full
type the user is storing in the map. Therefore, it is important for the user to
be aware if they are passing a reference to the key or the full type depending
on the function requirements.

To use the map as a set, when only keys are needed, wrap a type in a struct or
union. For example a set of `int` could be represented by creating a type
`struct My_int {int key;};`. All interface functions can then be used normally.

To shorten names in the interface, define the following preprocessor directive
at the top of your file.

```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
```

All types and functions can then be written without the `CCC_` prefix. */
#ifndef CCC_ARRAY_TREE_MAP_H
#define CCC_ARRAY_TREE_MAP_H

/** @cond */
#include <stddef.h>
/** @endcond */

#include "private/private_array_tree_map.h"
#include "types.h"

/** @name Container Types
Types available in the container interface. */
/**@{*/

/** @brief An array tree map offers O(lg N) search and erase, and
amortized O(lg N) insert.
@warning it is undefined behavior to access an uninitialized container.

An array tree map can be initialized on the stack, heap, or data
segment at runtime or compile time.*/
typedef struct CCC_Array_tree_map CCC_Array_tree_map;

/** @brief A container specific handle used to implement the Handle Interface.
@warning it is undefined behavior to access an uninitialized container.

The Handle Interface offers efficient search and subsequent insertion, deletion,
or value update based on the needs of the user. Handles obtained via the Handle
Interface are stable until the user removes the element at the provided handle.
Insertions and deletions of other elements do not affect handle stability.
Resizing of the table does not affect handle stability. */
typedef struct CCC_Array_tree_map_handle CCC_Array_tree_map_handle;

/**@}*/

/** @name Initialization Interface
Initialize the container with memory, callbacks, and permissions. */
/**@{*/

/** @brief Create the underlying fixed size map storage from a user declared
compound literal array of the type the user intends to store.
@param[in] user_type_compound_literal_array a compound literal array of the type
around which the map will be built.
@param[in] optional_storage_specifier a storage specifier for the backing struct
of array storage may be added on newer compilers such as static.
@warning This should rarely be used. If a fixed size map is desired simply use
the CCC_array_tree_map_with_storage() initializer. For dynamic maps,
there are also many other options.

This macro is required to support the edge case for the user allocating a fixed
size map dynamically from an allocator at runtime. In this case, the user needs
access to the compound literal struct of arrays map to know how many bytes to
allocate. See the below example.

```
struct Val
{
    int key;
    int val;
};

int
main(void)
{
    void *const map = malloc(
        sizeof(CCC_array_tree_map_storage_for((struct Val[4096]){}))
    );
    defer free(map);
    CCC_Array_tree_map map = CCC_array_tree_map_for(
        struct Val,
        key,
        order_vals,
        NULL,
        NULL,
        4096,
        map
    );
}
```

Usually, using a dynamic map and the reserve interface would be sufficient.
However, the reserve interface only guarantees that at least the needed bytes
are allocated. When the user must know the exact size of the backing object due
to strict memory requirements, this is helpful. Such a use case may be rare, but
must be supported by this container. */
#define CCC_array_tree_map_storage_for(                                        \
    user_type_compound_literal_array, optional_storage_specifier...            \
)                                                                              \
    CCC_private_array_tree_map_storage_for(                                    \
        user_type_compound_literal_array, optional_storage_specifier           \
    )

/** @brief Initializes a default empty map at runtime or compile time.
@param[in] type_name the name of the user type stored in the map.
@param[in] type_key_field the name of the field in user type used as key.
@param[in] comparator the CCC_Key_comparator for key comparison.
@return the map for assignment on the right hand side of equality operator. */
#define CCC_array_tree_map_default(type_name, type_key_field, comparator...)   \
    CCC_private_array_tree_map_default(type_name, type_key_field, comparator)

/** @brief Initializes the map at runtime or compile time.
@param[in] type_name the name of the user type stored in the map.
@param[in] type_key_field the name of the field in user type used as key.
@param[in] comparator the CCC_Key_comparator for key comparison.
@param[in] capacity the capacity at data or 0.
@param[in] memory_pointer a pointer to the contiguous user types or NULL.
@return the struct initialized tree map for direct assignment
(i.e. CCC_Array_tree_map m = CCC_array_tree_map_for(...);). */
#define CCC_array_tree_map_for(                                                \
    type_name, type_key_field, comparator, capacity, memory_pointer            \
)                                                                              \
    CCC_private_array_tree_map_for(                                            \
        type_name, type_key_field, comparator, capacity, memory_pointer        \
    )

/** @brief Initialize a dynamic map at runtime from an initializer list.
@param[in] type_key_field the field of the struct used for key storage.
@param[in] comparator the CCC_Key_comparator for key comparison.
@param[in] allocator the required CCC_Allocator for reservation.
@param[in] optional_capacity optionally specify the capacity of the map if
different from the size of the compound literal array initializer. If the
capacity is greater than the size of the compound literal array initializer, it
is respected and the capacity is reserved. If the capacity is less than the size
of the compound array initializer, the compound literal array initializer size
is set as the capacity. Therefore, 0 is valid if one is not concerned with the
size of the underlying reservation.
@param[in] type_compound_literal_array a list of key value pairs of the type
intended to be stored in the map, using array compound literal initialization
syntax (e.g `(struct my_type[]){{.k = 0, .v 0}, {.k = 1, .v = 1}}`).
@return the  map directly initialized on the right hand side of the equality
operator (i.e. CCC_Array_tree_map map = CCC_array_tree_map_from(...);)
@warning An allocator is required to use this initializer.
@warning When duplicate keys appear in the initializer list, the last occurrence
replaces earlier ones by value (all fields are overwritten).
@warning If initialization fails all subsequent queries, insertions, or
removals will indicate the error: either memory related or lack of an
allocation function provided.

Initialize a dynamic map at run time.
```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
struct Val
{
    int key;
    int val;
};
int
main(void)
{
    Array_tree_map static_map = array_tree_map_from(
        key,
        val_key_comparator,
        std_allocator,
        0,
        (struct Val[]) {
            {.key = 1, .val = 1},
            {.key = 2, .val = 2},
            {.key = 3, .val = 3},
        },
    );
    return 0;
}
```

Only dynamic maps may be initialized this way due the inability of the map
map to protect its invariants from user error at compile time. */
#define CCC_array_tree_map_from(                                               \
    type_key_field,                                                            \
    comparator,                                                                \
    allocator,                                                                 \
    optional_capacity,                                                         \
    type_compound_literal_array...                                             \
)                                                                              \
    CCC_private_array_tree_map_from(                                           \
        type_key_field,                                                        \
        comparator,                                                            \
        allocator,                                                             \
        optional_capacity,                                                     \
        type_compound_literal_array                                            \
    )

/** @brief Initialize a dynamic map at runtime with at least the specified
capacity.
@param[in] type_name the name of the type being stored in the map.
@param[in] type_key_field the field of the struct used for key storage.
@param[in] comparator the CCC_Key_comparator for key comparison.
@param[in] allocator the required CCC_Allocator for reservation.
@param[in] capacity the desired capacity for the map. A capacity of 0 results
in an argument error and is a no-op after the map is initialized empty.
@return the map directly initialized on the right hand side of the equality
operator (i.e. CCC_Array_tree_map map =
CCC_array_tree_map_with_capacity(...);)
@warning An allocation function is required. This initializer is only available
for dynamic maps.
@warning If initialization fails all subsequent queries, insertions, or
removals will indicate the error: either memory related or lack of an
allocation function provided.

Initialize a dynamic map at run time. This example requires no context
data for initialization.

```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
struct Val
{
    int key;
    int val;
};
int
main(void)
{
    Array_tree_map map = array_tree_map_with_capacity(
        struct Val,
        key,
        val_key_comparator,
        std_allocator,
        4096
    );
    return 0;
}
```

Only dynamic maps may be initialized this way as it simply combines the steps
of initialization and reservation. */
#define CCC_array_tree_map_with_capacity(                                      \
    type_name, type_key_field, comparator, allocator, capacity                 \
)                                                                              \
    CCC_private_array_tree_map_with_capacity(                                  \
        type_name, type_key_field, comparator, allocator, capacity             \
    )

/** @brief Initialize a fixed map at compile or runtime from any user chosen
type with no allocation permission or context.
@param[in] type_key_field the field of the struct used for key storage.
@param[in] comparator the CCC_Key_comparator for key comparison.
@param[in] compound_literal the compound literal array of a type provided by the
user around which the struct of array backing storage for the map will be built.
@param[in] optional_storage_specifier lifetime specifier of the backing struct
of array storage, such as static, for the fixed size map in the scope at which
it is allocated or declared.
@return the map directly initialized on the right hand side of the equality
operator.

Initialize a fixed map.

```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
struct Val
{
    int key;
    int val;
};
static Array_tree_map map = array_tree_map_with_storage(
    key,
    val_key_comparator,
    (struct Val[4096]){}
);
```

This can help eliminate boilerplate in initializers. */
#define CCC_array_tree_map_with_storage(                                       \
    type_key_field,                                                            \
    comparator,                                                                \
    compound_literal,                                                          \
    optional_storage_specifier...                                              \
)                                                                              \
    CCC_private_array_tree_map_with_storage(                                   \
        type_key_field,                                                        \
        comparator,                                                            \
        compound_literal,                                                      \
        optional_storage_specifier                                             \
    )

/** @brief Copy the map at source to destination.
@param[in] destination the initialized destination for the copy of the source
map.
@param[in] source the initialized source of the map.
@param[in] allocator CCC_Allocator for resizing or `&(CCC_Allocator){}`.
@return the result of the copy operation. If the destination capacity is less
than the source capacity and no allocation function is provided an input error
is returned. If resizing is required and resizing of destination fails a memory
error is returned.
@note destination must have capacity greater than or equal to source. If
destination capacity is less than source, an allocator must be provided with the
allocate argument.

Note that there are two ways to copy data from source to destination: provide
sufficient memory and pass `&(CCC_Allocator){}` as allocator, or pass an
allocator.

Manual memory management with no allocator provided.

```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
struct Val
{
    int key;
    int val;
};
static map source = array_tree_map_with_storage(
    struct Val,
    key,
    val_key_comparator,
    &(struct Val[64]){}
);
insert_rand_vals(&source, &(CCC_Allocator){});
static map destination = array_tree_map_with_storage(
    struct Val,
    key,
    val_key_comparator,
    &(struct Val[64]){}
);
CCC_Result res
    = array_tree_map_copy(&destination, &source, &(CCC_Allocator){});
```

The above requires destination capacity be greater than or equal to source
capacity. Here is memory management handed over to the copy function.

```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
struct Val
{
    int key;
    int val;
};
static Array_adaptive_map source = array_tree_map_default(
    struct Val,
    key,
    val_key_comparator
);
insert_rand_vals(&source, &std_allocator);
static Array_adaptive_map destination = array_tree_map_default(
    struct Val,
    key,
    val_key_comparator
);
CCC_Result res = array_tree_map_copy(&destination, &source, &std_allocator);
```

These options allow users to stay consistent across containers with their
memory management strategies. */
CCC_Result CCC_array_tree_map_copy(
    CCC_Array_tree_map *destination,
    CCC_Array_tree_map const *source,
    CCC_Allocator const *allocator
);

/** @brief Reserves space for at least to_add more elements.
@param[in] map a pointer to the array tree map.
@param[in] to_add the number of elements to add to the current size.
@param[in] allocator CCC_Allocator for reserving memory.
@return the result of the reservation. OK if successful, otherwise an error
status is returned. */
CCC_Result CCC_array_tree_map_reserve(
    CCC_Array_tree_map *map, size_t to_add, CCC_Allocator const *allocator
);

/**@}*/

/**@name Membership Interface
Test membership or obtain references to stored user types directly. */
/**@{*/

/** @brief Returns a reference to the user data at the provided handle.
@param[in] handle a pointer to the map.
@param[in] index the stable handle obtained by the user.
@return a pointer to the user type stored at the specified handle or NULL if
an out of range handle or handle representing no data is provided.
@warning this function can only check if the handle value is in range. If a
handle represents a slot that has been taken by a new element because the old
one has been removed that new element data will be returned.
@warning do not try to access data in the table manually with a handle. Always
use this provided interface function when a reference to data is needed. */
[[nodiscard]] void *
CCC_array_tree_map_at(CCC_Array_tree_map const *handle, CCC_Handle_index index);

/** @brief Returns a reference to the user type in the table at the handle.
@param[in] array_tree_map_pointer a pointer to the map.
@param[in] type_name name of the user type stored in each slot of the map.
@param[in] array_index the index handle obtained from previous map operations.
@return a reference to the handle at handle in the map as the type the user has
stored in the map. */
#define CCC_array_tree_map_as(                                                 \
    array_tree_map_pointer, type_name, array_index...                          \
)                                                                              \
    CCC_private_array_tree_map_as(                                             \
        array_tree_map_pointer, type_name, array_index                         \
    )

/** @brief Searches the map for the presence of key.
@param[in] map the map to be searched.
@param[in] key pointer to the key matching the key type of the user struct.
@return true if the struct containing key is stored, false if not. Error if
array_tree_map or key is NULL. */
[[nodiscard]] CCC_Tribool
CCC_array_tree_map_contains(CCC_Array_tree_map const *map, void const *key);

/** @brief Returns a reference into the map at handle key.
@param[in] map the tree map to search.
@param[in] key the key to search matching stored key type.
@return a view of the map handle if it is present, else NULL. */
[[nodiscard]] CCC_Handle_index CCC_array_tree_map_get_key_value(
    CCC_Array_tree_map const *map, void const *key
);

/**@}*/

/** @name Handle Interface
Obtain and operate on container entries for efficient queries when non-trivial
control flow is needed. */
/**@{*/

/** @brief Invariantly inserts the key value in type_output.
@param[in] map the pointer to the tree map.
@param[out] type_output the type user type map elem.
@param[in] allocator CCC_Allocator for reserving memory if needed.
@return a type element in the table. If Vacant, no prior element with
key existed and the type key value type remains unchanged. If Occupied the
old value is written to the type key value type. If more space is needed
but allocation fails or has been forbidden, an insert error is set.

Note that this function may write to the provided user type struct. */
[[nodiscard]] CCC_Handle CCC_array_tree_map_swap_handle(
    CCC_Array_tree_map *map, void *type_output, CCC_Allocator const *allocator
);

/** @brief Invariantly inserts the key value in type_output_pointer.
@param[in] array_tree_map_pointer the pointer to the tree map.
@param[out] type_output_pointer type user type map elem.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@return a compound literal reference to a type element in the table. If
Vacant, no prior element with key existed and the type key value type
remains unchanged. If Occupied the old value is written to the type wrapping
key value type. If more space is needed but allocation fails or has been
forbidden, an insert error is set.

Note that this function may write to the provided user type struct. */
#define CCC_array_tree_map_swap_handle_wrap(                                   \
    array_tree_map_pointer, type_output_pointer, allocator_pointer...          \
)                                                                              \
    &(struct { CCC_Handle private; }){                                         \
        CCC_array_tree_map_swap_handle(                                        \
            (array_tree_map_pointer), (type_output_pointer), allocator_pointer \
        )}                                                                     \
         .private

/** @brief Attempts to insert the key value in type.
@param[in] map the pointer to the map.
@param[in] type the type user type map elem.
@param[in] allocator CCC_Allocator for reserving memory if needed.
@return a handle. If Occupied, the handle contains a reference to the key value
user type in the map and may be unwrapped. If Vacant the handle contains a
reference to the newly inserted handle in the map. If more space is needed but
allocation fails, an insert error is set. */
[[nodiscard]] CCC_Handle CCC_array_tree_map_try_insert(
    CCC_Array_tree_map *map, void const *type, CCC_Allocator const *allocator
);

/** @brief Attempts to insert the key value type_pointer.
@param[in] array_tree_map_pointer the pointer to the map.
@param[in] type_pointer the type user type map elem.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@return a compound literal reference to a handle. If Occupied, the handle
contains a reference to the key value user type in the map and may be unwrapped.
If Vacant the handle contains a reference to the newly inserted handle in the
map. If more space is needed but allocation fails an insert error is set. */
#define CCC_array_tree_map_try_insert_wrap(                                    \
    array_tree_map_pointer, type_pointer, allocator_pointer...                 \
)                                                                              \
    &(struct { CCC_Handle private; }){                                         \
        CCC_array_tree_map_try_insert(                                         \
            (array_tree_map_pointer), (type_pointer), allocator_pointer        \
        )}                                                                     \
         .private

/** @brief lazily insert type_compound_literal into the map at key if key is
absent.
@param[in] array_tree_map_pointer a pointer to the map.
@param[in] key the direct key r-value.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@param[in] type_compound_literal the compound literal specifying the value.
@return a compound literal reference to the handle of the existing or newly
inserted value. Occupied indicates the key existed, Vacant indicates the key
was absent. Behavior in any case provides the current value unless an error
occurs that prevents insertion. An insertion error will flag such a case.

Note that for brevity and convenience the user need not write the key to the
lazy value compound literal as well. This function ensures the key in the
compound literal matches the searched key. */
#define CCC_array_tree_map_try_insert_with(                                    \
    array_tree_map_pointer, key, allocator_pointer, type_compound_literal...   \
)                                                                              \
    &(struct { CCC_Handle private; }){                                         \
        CCC_private_array_tree_map_try_insert_with(                            \
            array_tree_map_pointer,                                            \
            key,                                                               \
            allocator_pointer,                                                 \
            type_compound_literal                                              \
        )}                                                                     \
         .private

/** @brief Invariantly inserts or overwrites a user struct into the map.
@param[in] map a pointer to the handle map.
@param[in] type the type user struct key value.
@param[in] allocator CCC_Allocator for reserving memory if needed.
@return a handle. If Occupied a handle was overwritten by the new key value.
If Vacant no prior map handle existed.

Note that this function can be used when the old user type is not needed but
the information regarding its presence is helpful. */
[[nodiscard]] CCC_Handle CCC_array_tree_map_insert_or_assign(
    CCC_Array_tree_map *map, void const *type, CCC_Allocator const *allocator
);

/** @brief Invariantly inserts or overwrites a user struct into the map.
@param[in] map_pointer a pointer to the handle map.
@param[in] type_pointer a pointer to the user struct key value type.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@return a compound literal reference to a handle. If Occupied a handle was
overwritten by the new key value. If Vacant no prior map handle existed.

Note that this function can be used when the old user type is not needed but
the information regarding its presence is helpful. */
#define CCC_array_tree_map_insert_or_assign_wrap(                              \
    map_pointer, type_pointer, allocator_pointer...                            \
)                                                                              \
    &(struct { CCC_Handle private; }){                                         \
        CCC_array_tree_map_insert_or_assign(                                   \
            map_pointer, type_pointer, allocator_pointer                       \
        )}                                                                     \
         .private

/** @brief Inserts a new key value pair or overwrites the existing handle.
@param[in] array_tree_map_pointer the pointer to the handle map.
@param[in] key the key to be searched in the map.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@param[in] type_compound_literal the compound literal to insert or use for
overwrite.
@return a compound literal reference to the handle of the existing or newly
inserted value. Occupied indicates the key existed, Vacant indicates the key
was absent. Any case provides the current value unless an error occurs that
prevents insertion. An insertion error will flag such a case.
@note For brevity and convenience the user need not write the key to the
lazy value compound literal as well. This function ensures the key in the
compound literal matches the searched key. */
#define CCC_array_tree_map_insert_or_assign_with(                              \
    array_tree_map_pointer, key, allocator_pointer, type_compound_literal...   \
)                                                                              \
    &(struct { CCC_Handle private; }){                                         \
        CCC_private_array_tree_map_insert_or_assign_with(                      \
            array_tree_map_pointer,                                            \
            key,                                                               \
            allocator_pointer,                                                 \
            type_compound_literal                                              \
        )}                                                                     \
         .private

/** @brief Removes the key value in the map storing the old value, if present,
in the type_output provided by the user.
@param[in] map the pointer to the tree map.
@param[out] type_output the type user type map elem.
@return the removed handle. If Occupied key value type holds the old key value
pair. If Vacant the key value pair was not stored in the map. If bad input is
provided an input error is set.

Note that this function may write to the user type struct. */
[[nodiscard]] CCC_Handle
CCC_array_tree_map_remove_key_value(CCC_Array_tree_map *map, void *type_output);

/** @brief Removes the key value in the map storing the old value, if present,
in the key value type provided by the user.
@param[in] array_tree_map_pointer the pointer to the tree map.
@param[out] type_output_pointer type user type map elem.
@return a compound literal reference to the removed handle. If Occupied
type_output_pointer holds the old key value pair.. If Vacant the key
value pair was not stored in the map. If bad input is provided an input error is
set.

Note that this function may write to the user type struct. */
#define CCC_array_tree_map_remove_key_value_wrap(                              \
    array_tree_map_pointer, type_output_pointer                                \
)                                                                              \
    &(struct { CCC_Handle private; }){                                         \
        CCC_array_tree_map_remove_key_value(                                   \
            (array_tree_map_pointer), (type_output_pointer)                    \
        )}                                                                     \
         .private

/** @brief Obtains a handle for the provided key in the map for future use.
@param[in] map the map to be searched.
@param[in] key the key used to search the map matching the stored key type.
@return a specialized handle for use with other functions in the Handle
Interface.
@warning the contents of a handle should not be examined or modified. Use the
provided functions, only.

A handle is a search result that provides either an Occupied or Vacant handle
in the map. An occupied handle signifies that the search was successful. A
Vacant handle means the search was not successful but a handle is gained to
where in the map such an element should be inserted.

A handle is rarely useful on its own. It should be passed in a functional style
to subsequent calls in the Handle Interface. */
[[nodiscard]] CCC_Array_tree_map_handle
CCC_array_tree_map_handle(CCC_Array_tree_map const *map, void const *key);

/** @brief Obtains a handle for the provided key in the map for future use.
@param[in] array_tree_map_pointer the map to be searched.
@param[in] key_pointer the key used to search the map matching the stored key
type.
@return a compound literal reference to a specialized handle for use with other
functions in the Handle Interface.
@warning the contents of a handle should not be examined or modified. Use the
provided functions, only.

A handle is a search result that provides either an Occupied or Vacant handle
in the map. An occupied handle signifies that the search was successful. A
Vacant handle means the search was not successful but a handle is gained to
where in the map such an element should be inserted.

A handle is rarely useful on its own. It should be passed in a functional style
to subsequent calls in the Handle Interface. */
#define CCC_array_tree_map_handle_wrap(array_tree_map_pointer, key_pointer)    \
    &(struct { CCC_Array_tree_map_handle private; }){                          \
        CCC_array_tree_map_handle((array_tree_map_pointer), (key_pointer))}    \
         .private

/** @brief Modifies the provided handle if it is Occupied.
@param[in] handle the handle obtained from a handle function or macro.
@param[in] modifier a CCC_Modifer that operates on an occupied element.
@return the updated handle if Occupied or the unmodified vacant handle. */
[[nodiscard]] CCC_Array_tree_map_handle *CCC_array_tree_map_and_modify(
    CCC_Array_tree_map_handle *handle, CCC_Modifier const *modifier
);

/** @brief Modify an Occupied handle with a closure over user type T.
@param[in] map_array_pointer a pointer to the obtained handle.
@param[in] typed_pointer_to_T the pointer type `My_type *` or `My_type const *`
with which to interpret an occupied entry named T.
@param[in] closure_over_T the code to be run on the reference to user type T,
if Occupied. This may be a semicolon separated list of statements to execute on
T or a section of code wrapped in braces {code here} which may be preferred
for formatting.
@return a compound literal reference to the modified handle if it was occupied
or a vacant handle if it was vacant.
@note T is a reference to the user type specified by the provided pointer
argument stored in the entry guaranteed to be non-NULL if the closure executes.

```
#define ARRAY_TREE_MAP_USING_NAMESPACE_CCC
// Increment the count if found otherwise do nothing.
Array_tree_map_handle *h =
    array_tree_map_and_modify_with(
        handle_wrap(&array_tree_map, &k),
        Word *,
        T->cnt++;
    );
// Increment the count if found otherwise insert a default value.
Handle_index w =
    array_tree_map_or_insert_with(
        array_tree_map_and_modify_with(
            handle_wrap(&array_tree_map, &k),
            Word *,
            { T->cnt++; }
        ),
        (Word){.key = k, .cnt = 1}
    );
```

Note that any code written is only evaluated if the handle is Occupied and the
container can deliver the user type T. This means any function calls are lazily
evaluated in the closure scope. */
#define CCC_array_tree_map_and_modify_with(                                    \
    map_array_pointer, typed_pointer_to_T, closure_over_T...                   \
)                                                                              \
    &(struct { CCC_Array_tree_map_handle private; }){                          \
        CCC_private_array_tree_map_and_modify_with(                            \
            map_array_pointer, typed_pointer_to_T, closure_over_T              \
        )}                                                                     \
         .private

/** @brief Inserts the provided user type if the handle is Vacant.
@param[in] handle the handle obtained via function or macro call.
@param[in] type the type struct to be inserted to a Vacant handle.
@param[in] allocator CCC_Allocator for reserving memory if needed.
@return a pointer to handle in the map invariantly. NULL on error.

Because this functions takes a handle and inserts if it is Vacant, the only
reason NULL shall be returned is when an insertion error occurs, usually due to
a user struct allocation failure.

If no allocation is permitted, this function assumes the user struct wrapping
elem has been allocated with the appropriate lifetime and scope by the user. */
[[nodiscard]] CCC_Handle_index CCC_array_tree_map_or_insert(
    CCC_Array_tree_map_handle const *handle,
    void const *type,
    CCC_Allocator const *allocator
);

/** @brief Lazily insert the desired key value into the handle if it is Vacant.
@param[in] map_array_pointer a pointer to the obtained handle.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@param[in] type_compound_literal the compound literal to construct in place if
the handle is Vacant.
@return a reference to the unwrapped user type in the handle, either the
unmodified reference if the handle was Occupied or the newly inserted element
if the handle was Vacant. NULL is returned if resizing is required but fails or
is not allowed.

Note that if the compound literal uses any function calls to generate values
or other data, such functions will not be called if the handle is Occupied. */
#define CCC_array_tree_map_or_insert_with(                                     \
    map_array_pointer, allocator_pointer, type_compound_literal...             \
)                                                                              \
    CCC_private_array_tree_map_or_insert_with(                                 \
        map_array_pointer, allocator_pointer, type_compound_literal            \
    )

/** @brief Inserts the provided user type invariantly.
@param[in] handle the handle returned from a call obtaining a handle.
@param[in] type a type struct the user intends to insert.
@param[in] allocator CCC_Allocator for reserving memory if needed.
@return a pointer to the inserted element or NULL upon allocation failure.

This method can be used when the old value in the map does not need to
be preserved. See the regular insert method if the old value is of interest. */
[[nodiscard]] CCC_Handle_index CCC_array_tree_map_insert_handle(
    CCC_Array_tree_map_handle const *handle,
    void const *type,
    CCC_Allocator const *allocator
);

/** @brief Write the contents of the compound literal type_compound_literal to a
node.
@param[in] map_array_pointer a pointer to the obtained handle.
@param[in] allocator_pointer CCC_Allocator for reserving memory if needed.
@param[in] type_compound_literal the compound literal to write to a new slot.
@return a reference to the newly inserted or overwritten user type. NULL is
returned if allocation failed or is not allowed when required. */
#define CCC_array_tree_map_insert_handle_with(                                 \
    map_array_pointer, allocator_pointer, type_compound_literal...             \
)                                                                              \
    CCC_private_array_tree_map_insert_handle_with(                             \
        map_array_pointer, allocator_pointer, type_compound_literal            \
    )

/** @brief Remove the handle from the map if Occupied.
@param[in] handle a pointer to the map handle.
@return a handle containing NULL or a reference to the old handle. If Occupied
a handle in the map existed and was removed. If Vacant, no prior handle existed
to be removed.
@warning the reference to the removed handle is invalidated upon any further
insertions. */
CCC_Handle
CCC_array_tree_map_remove_handle(CCC_Array_tree_map_handle const *handle);

/** @brief Remove the handle from the map if Occupied.
@param[in] map_array_pointer a pointer to the map handle.
@return a compound literal reference to a handle containing NULL or a reference
to the old handle. If Occupied a handle in the map existed and was removed. If
Vacant, no prior handle existed to be removed.

Note that the reference to the removed handle is invalidated upon any further
insertions. */
#define CCC_array_tree_map_remove_handle_wrap(map_array_pointer)               \
    &(struct { CCC_Handle private; }){                                         \
        CCC_array_tree_map_remove_handle((map_array_pointer))}                 \
         .private

/** @brief Unwraps the provided handle to obtain a view into the map element.
@param[in] handle the handle from a query to the map via function or macro.
@return a view into the table handle if one is present, or NULL. */
[[nodiscard]] CCC_Handle_index
CCC_array_tree_map_unwrap(CCC_Array_tree_map_handle const *handle);

/** @brief Returns the Vacant or Occupied status of the handle.
@param[in] handle the handle from a query to the map via function or macro.
@return true if the handle is occupied, false if not. Error if handle is NULL.
*/
[[nodiscard]] CCC_Tribool
CCC_array_tree_map_occupied(CCC_Array_tree_map_handle const *handle);

/** @brief Provides the status of the handle should an insertion follow.
@param[in] handle the handle from a query to the table via function or macro.
@return true if a handle obtained from an insertion attempt failed to insert
due to an allocation failure when allocation success was expected. Error if
handle is NULL. */
[[nodiscard]] CCC_Tribool
CCC_array_tree_map_insert_error(CCC_Array_tree_map_handle const *handle);

/** @brief Obtain the handle status from a container handle.
@param[in] handle a pointer to the handle.
@return the status stored in the handle after the required action on the
container completes. If handle is NULL a handle input error is returned so
ensure e is non-NULL to avoid an inaccurate status returned.

Note that this function can be useful for debugging or if more detailed
messages are needed for logging purposes. See CCC_handle_status_message() in
ccc/types.h for more information on detailed handle statuses. */
[[nodiscard]] CCC_Handle_status
CCC_array_tree_map_handle_status(CCC_Array_tree_map_handle const *handle);

/**@}*/

/** @name Deallocation Interface
Deallocate the container. */
/**@{*/

/** @brief Frees all slots in the map for use without affecting capacity.
@param[in] map the map to be cleared.
@param[in] destructor the optional CCC_Destructor or `&(CCC_Destructor){}` if
no maintenance is required on the elements in the map before their slots are
forfeit.

If the empty destructor is passed as the destructor function time is O(1), else
O(size). */
CCC_Result CCC_array_tree_map_clear(
    CCC_Array_tree_map *map, CCC_Destructor const *destructor
);

/** @brief Frees all slots in the map and frees the underlying buffer.
@param[in] map the map to be cleared.
@param[in] destructor the optional CCC_Destructor or `&(CCC_Destructor){}` if
no maintenance is required on the elements in the map before their slots are
forfeit.
@param[in] allocator CCC_Allocator for reserving memory if needed.
@return the result of free operation. If no allocator is provided it is
an error to attempt to free storage and a memory error is returned. Otherwise,
an OK result is returned.

If an empty destructor is passed as the destructor function time is O(1), else
O(size). */
CCC_Result CCC_array_tree_map_clear_and_free(
    CCC_Array_tree_map *map,
    CCC_Destructor const *destructor,
    CCC_Allocator const *allocator
);

/**@}*/

/** @name Iterator Interface
Obtain and manage iterators over the container. */
/**@{*/

/** @brief Return an iterable range of values from [begin_key, end_key). O(lgN).
@param[in] map a pointer to the map.
@param[in] begin_key a pointer to the key intended as the start of the range.
@param[in] end_key a pointer to the key intended as the end of the range.
@return a range containing the first element NOT LESS than the begin_key and
the first element GREATER than end_key.

Note that due to the variety of values that can be returned in the range, using
the provided range iteration functions from types.h is recommended for example:

```
for (CCC_Handle_index index = range_begin(&range);
     index != range_end(&range);
     index = next(&map, index))
{}
```

This avoids any possible errors in handling an end range element that is in the
map versus the end map sentinel. */
[[nodiscard]] CCC_Handle_range CCC_array_tree_map_equal_range(
    CCC_Array_tree_map const *map, void const *begin_key, void const *end_key
);

/** @brief Returns a compound literal reference to the desired range. O(lg N).
@param[in] array_tree_map_pointer a pointer to the map.
@param[in] begin_and_end_key_pointers two pointers, the first to the start of
the range the second to the end of the range.
@return a compound literal reference to the produced range associated with the
enclosing scope. This reference is always non-NULL. */
#define CCC_array_tree_map_equal_range_wrap(                                   \
    array_tree_map_pointer, begin_and_end_key_pointers...                      \
)                                                                              \
    &(struct { CCC_Handle_range private; }){                                   \
        CCC_array_tree_map_equal_range(                                        \
            (array_tree_map_pointer), begin_and_end_key_pointers               \
        )}                                                                     \
         .private

/** @brief Return an iterable range_reverse of values from [begin_key, end_key).
O(lg N).
@param[in] map a pointer to the map.
@param[in] reverse_begin_key a pointer to the key intended as the start of the
range_reverse.
@param[in] reverse_end_key a pointer to the key intended as the end of the
range_reverse.
@return a range_reverse containing the first element NOT GREATER than the
begin_key and the first element LESS than reverse_end_key.

Note that due to the variety of values that can be returned in the
range_reverse, using the provided range_reverse iteration functions from types.h
is recommended for example:

```
for (CCC_Handle_index index = range_reverse_begin(&range);
     index != range_reverse_end(&range);
     index = next(&map, index))
{}
```

This avoids any possible errors in handling an reverse_end range_reverse element
that is in the map versus the end map sentinel. */
[[nodiscard]] CCC_Handle_range_reverse CCC_array_tree_map_equal_range_reverse(
    CCC_Array_tree_map const *map,
    void const *reverse_begin_key,
    void const *reverse_end_key
);

/** @brief Returns a compound literal reference to the desired range_reverse.
O(lg N).
@param[in] array_tree_map_pointer a pointer to the map.
@param[in] reverse_begin_and_reverse_end_key_pointers two pointers, the first
to the start of the range_reverse the second to the end of the range_reverse.
@return a compound literal reference to the produced range_reverse associated
with the enclosing scope. This reference is always non-NULL. */
#define CCC_array_tree_map_equal_range_reverse_wrap(                           \
    array_tree_map_pointer, reverse_begin_and_reverse_end_key_pointers...      \
)                                                                              \
    &(                                                                         \
         struct { CCC_Handle_range_reverse private; }                          \
    ){CCC_array_tree_map_equal_range_reverse(                                  \
          (array_tree_map_pointer), reverse_begin_and_reverse_end_key_pointers \
      )}                                                                       \
         .private

/** @brief Return the start of an inorder traversal of the map. O(lg N).
@param[in] map a pointer to the map.
@return a handle for the minimum element of the map. */
[[nodiscard]] CCC_Handle_index
CCC_array_tree_map_begin(CCC_Array_tree_map const *map);

/** @brief Return the start of a reverse inorder traversal of the map. O(lg N).
@param[in] map a pointer to the map.
@return a handle for the maximum element of the map. */
[[nodiscard]] CCC_Handle_index
CCC_array_tree_map_reverse_begin(CCC_Array_tree_map const *map);

/** @brief Return the next element in an inorder traversal of the map. O(1).
@param[in] map a pointer to the map.
@param[in] iterator a pointer to the intrusive map element of the current
iterator.
@return a handle for the next user type stored in the map in an inorder
traversal. */
[[nodiscard]] CCC_Handle_index CCC_array_tree_map_next(
    CCC_Array_tree_map const *map, CCC_Handle_index iterator
);

/** @brief Return the reverse_next element in a reverse inorder traversal of the
map. O(1).
@param[in] map a pointer to the map.
@param[in] iterator a pointer to the intrusive map element of the
current iterator.
@return a handle for the reverse_next user type stored in the map in a reverse
inorder traversal. */
[[nodiscard]] CCC_Handle_index CCC_array_tree_map_reverse_next(
    CCC_Array_tree_map const *map, CCC_Handle_index iterator
);

/** @brief Return the end of an inorder traversal of the map. O(1).
@param[in] map a pointer to the map.
@return a handle for the maximum element of the map. */
[[nodiscard]] CCC_Handle_index
CCC_array_tree_map_end(CCC_Array_tree_map const *map);

/** @brief Return the reverse_end of a reverse inorder traversal of the map.
O(1).
@param[in] map a pointer to the map.
@return a handle for the minimum element of the map. */
[[nodiscard]] CCC_Handle_index
CCC_array_tree_map_reverse_end(CCC_Array_tree_map const *map);

/**@}*/

/** @name State Interface
Obtain the container state. */
/**@{*/

/** @brief Returns the size status of the map.
@param[in] map the map.
@return true if empty else false. Error if map is NULL.
*/
[[nodiscard]] CCC_Tribool
CCC_array_tree_map_is_empty(CCC_Array_tree_map const *map);

/** @brief Returns the count of map occupied slots.
@param[in] map the map.
@return the size of the map or an argument error is set if
array_tree_map is NULL. */
[[nodiscard]] CCC_Count CCC_array_tree_map_count(CCC_Array_tree_map const *map);

/** @brief Returns the capacity of the map representing total available slots.
@param[in] map the map.
@return the capacity or an argument error is set if array_tree_map
is NULL. */
[[nodiscard]] CCC_Count
CCC_array_tree_map_capacity(CCC_Array_tree_map const *map);

/** @brief Validation of invariants for the map.
@param[in] map the map to validate.
@return true if all invariants hold, false if corruption occurs. Error if
array_tree_map is NULL. */
[[nodiscard]] CCC_Tribool
CCC_array_tree_map_validate(CCC_Array_tree_map const *map);

/**@}*/

/** Define this preprocessor directive if shorter names are helpful. Ensure
 no namespace clashes occur before shortening. */
#ifdef ARRAY_TREE_MAP_USING_NAMESPACE_CCC
/* NOLINTBEGIN(readability-identifier-naming) */
typedef CCC_Array_tree_map Array_tree_map;
typedef CCC_Array_tree_map_handle Array_tree_map_handle;
#    define array_tree_map_storage_for(arguments...)                           \
        CCC_array_tree_map_storage_for(arguments)
#    define array_tree_map_default(arguments...)                               \
        CCC_array_tree_map_default(arguments)
#    define array_tree_map_for(arguments...) CCC_array_tree_map_for(arguments)
#    define array_tree_map_from(arguments...) CCC_array_tree_map_from(arguments)
#    define array_tree_map_with_capacity(arguments...)                         \
        CCC_array_tree_map_with_capacity(arguments)
#    define array_tree_map_with_storage(arguments...)                          \
        CCC_array_tree_map_with_storage(arguments)
#    define array_tree_map_copy(arguments...) CCC_array_tree_map_copy(arguments)
#    define array_tree_map_reserve(arguments...)                               \
        CCC_array_tree_map_reserve(arguments)
#    define array_tree_map_at(arguments...) CCC_array_tree_map_at(arguments)
#    define array_tree_map_as(arguments...) CCC_array_tree_map_as(arguments)
#    define array_tree_map_and_modify_with(arguments...)                       \
        CCC_array_tree_map_and_modify_with(arguments)
#    define array_tree_map_or_insert_with(arguments...)                        \
        CCC_array_tree_map_or_insert_with(arguments)
#    define array_tree_map_insert_handle_with(arguments...)                    \
        CCC_array_tree_map_insert_handle_with(arguments)
#    define array_tree_map_try_insert(arguments...)                            \
        CCC_array_tree_map_try_insert(arguments)
#    define array_tree_map_try_insert_wrap(arguments...)                       \
        CCC_array_tree_map_try_insert_wrap(arguments)
#    define array_tree_map_try_insert_with(arguments...)                       \
        CCC_array_tree_map_try_insert_with(arguments)
#    define array_tree_map_insert_or_assign(arguments...)                      \
        CCC_array_tree_map_insert_or_assign(arguments)
#    define array_tree_map_insert_or_assign_wrap(arguments...)                 \
        CCC_array_tree_map_insert_or_assign_wrap(arguments)
#    define array_tree_map_insert_or_assign_with(arguments...)                 \
        CCC_array_tree_map_insert_or_assign_with(arguments)
#    define array_tree_map_contains(arguments...)                              \
        CCC_array_tree_map_contains(arguments)
#    define array_tree_map_get_key_value(arguments...)                         \
        CCC_array_tree_map_get_key_value(arguments)
#    define array_tree_map_handle(arguments...)                                \
        CCC_array_tree_map_handle(arguments)
#    define array_tree_map_handle_wrap(arguments...)                           \
        CCC_array_tree_map_handle_wrap(arguments)
#    define array_tree_map_remove_handle(arguments...)                         \
        CCC_array_tree_map_remove_handle(arguments)
#    define array_tree_map_remove_handle_wrap(arguments...)                    \
        CCC_array_tree_map_remove_handle_wrap(arguments)
#    define array_tree_map_remove_key_value(arguments...)                      \
        CCC_array_tree_map_remove_key_value(arguments)
#    define array_tree_map_remove_key_value_wrap(arguments...)                 \
        CCC_array_tree_map_remove_key_value_wrap(arguments)
#    define array_tree_map_swap_handle(arguments...)                           \
        CCC_array_tree_map_swap_handle(arguments)
#    define array_tree_map_swap_handle_wrap(arguments...)                      \
        CCC_array_tree_map_swap_handle_wrap(arguments)
#    define array_tree_map_begin(arguments...)                                 \
        CCC_array_tree_map_begin(arguments)
#    define array_tree_map_reverse_begin(arguments...)                         \
        CCC_array_tree_map_reverse_begin(arguments)
#    define array_tree_map_next(arguments...) CCC_array_tree_map_next(arguments)
#    define array_tree_map_reverse_next(arguments...)                          \
        CCC_array_tree_map_reverse_next(arguments)
#    define array_tree_map_end(arguments...) CCC_array_tree_map_end(arguments)
#    define array_tree_map_reverse_end(arguments...)                           \
        CCC_array_tree_map_reverse_end(arguments)
#    define array_tree_map_equal_range(arguments...)                           \
        CCC_array_tree_map_equal_range(arguments)
#    define array_tree_map_equal_range_wrap(arguments...)                      \
        CCC_array_tree_map_equal_range_wrap(arguments)
#    define array_tree_map_equal_range_reverse(arguments...)                   \
        CCC_array_tree_map_equal_range_reverse(arguments)
#    define array_tree_map_equal_range_reverse_wrap(arguments...)              \
        CCC_array_tree_map_equal_range_reverse_wrap(arguments)
#    define array_tree_map_is_empty(arguments...)                              \
        CCC_array_tree_map_is_empty(arguments)
#    define array_tree_map_count(arguments...)                                 \
        CCC_array_tree_map_count(arguments)
#    define array_tree_map_capacity(arguments...)                              \
        CCC_array_tree_map_capacity(arguments)
#    define array_tree_map_clear(arguments...)                                 \
        CCC_array_tree_map_clear(arguments)
#    define array_tree_map_clear_and_free(arguments...)                        \
        CCC_array_tree_map_clear_and_free(arguments)
#    define array_tree_map_validate(arguments...)                              \
        CCC_array_tree_map_validate(arguments)
/* NOLINTEND(readability-identifier-naming) */
#endif /* ARRAY_TREE_MAP_USING_NAMESPACE_CCC */

#endif /* CCC_ARRAY_TREE_MAP_H */
