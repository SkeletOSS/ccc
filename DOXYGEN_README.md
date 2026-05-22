# C Container Collection Documentation

- [Navigation](#header-navigation)
- [CCC Cardinal Rules](#ccc-cardinal-rules)
    - [Understand allocators](#understand-allocators)
        - [Empty allocators](#empty-allocators)
    - [Never pass NULL](#never-pass-null)
        - [Beware of intrusive containers](#beware-of-intrusive-containers)
    - [Use the correct initializer](#use-the-correct-initializer)
    - [Understand pointer versus handle stability](#pointers-versus-handles)
        - [Invalidated pointers](#invalidated-pointers)
        - [Stable pointers](#stable-pointers)
        - [Stable handles](#stable-handles)
- [Installation](#installation)
- [Coverage Report](#coverage-report)
- [Back to Repository](https://github.com/SkeletOSS/ccc)

## Header Navigation {#header-navigation}

Follow the links to the C Container Collection headers.

- [ccc/adaptive_map.h](https://skeletoss.github.io/ccc/adaptive__map_8h.html)
- [ccc/array_adaptive_map.h](https://skeletoss.github.io/ccc/array__adaptive__map_8h.html)
- [ccc/array_tree_map.h](https://skeletoss.github.io/ccc/array__tree__map_8h.html)
- [ccc/configuration.h](https://skeletoss.github.io/ccc/configuration_8h.html)
- [ccc/doubly_linked_list.h](https://skeletoss.github.io/ccc/doubly__linked__list_8h.html)
- [ccc/flat_bitset.h](https://skeletoss.github.io/ccc/flat__bitset_8h.html)
- [ccc/flat_buffer.h](https://skeletoss.github.io/ccc/flat__buffer_8h.html)
- [ccc/flat_double_ended_queue.h](https://skeletoss.github.io/ccc/flat__double__ended__queue_8h.html)
- [ccc/flat_hash_map.h](https://skeletoss.github.io/ccc/flat__hash__map_8h.html)
- [ccc/flat_priority_queue.h](https://skeletoss.github.io/ccc/flat__priority__queue_8h.html)
- [ccc/priority_queue.h](https://skeletoss.github.io/ccc/priority__queue_8h.html)
- [ccc/singly_linked_list.h](https://skeletoss.github.io/ccc/singly__linked__list_8h.html)
- [ccc/sort.h](https://skeletoss.github.io/ccc/sort_8h.html)
- [ccc/traits.h](https://skeletoss.github.io/ccc/traits_8h.html)
- [ccc/tree_map.h](https://skeletoss.github.io/ccc/tree__map_8h.html)
- [ccc/types.h](https://skeletoss.github.io/ccc/types_8h.html)

## CCC Cardinal Rules {#ccc-cardinal-rules}

1. **[Understand allocators](#understand-allocators)**.
2. **[Never pass NULL to any C Container Collection function or macro](#never-pass-null)**.
3. **[Use the correct initializer](#use-the-correct-initializer)**.
4. **[Understand pointer versus handle stability](#pointers-versus-handles)**.

### Understand Allocators {#understand-allocators}

Traditionally, dynamic memory is obtained, resized, and freed through an OS or language provided interface such as `malloc`, `realloc`, and `free`; these three functions manage a global allocator. However, these three core functionalities of an allocator can be united into one function. This is the approach that the C Container Collection takes. This is the implementation of the standard library allocator through the `CCC_Allocator_interface` type function.

```c
/** @brief A bundle of arguments to pass to the user-implemented
Allocator_interface function interface. This ensures clarity in inputs and
expected outputs to an allocator function the user wishes to use for managing
containers. Additional context can be provided for more complex allocation
schemes.

The pointers are const to bind the input closely with its context, if context is
provided. Because container code is providing the user a reference to a user
type it currently stores, or data it manages, it is critical the user does not
accidentally move or misuse the pointer to jeopardize container invariants. */
typedef struct {
    /** The input to the allocation function. NULL or previously allocated. */
    void *const input;
    /** The bytes being requested from the allocator. 0 is a free request. */
    size_t bytes;
    /** The alignment for the base address of the returned bytes. */
    size_t alignment;
    /** Additional state to pass to the allocator to help manage memory. */
    void *const context;
} CCC_Allocator_arguments;

/** @brief The function interface for allocating, resizing, and freeing memory.

The C Container Collection relies on the following behaviors when calling the
user provided allocator function internally.

- If input is NULL and bytes 0, NULL is returned.
- If input is NULL with non-zero bytes, new memory is allocated and returned
  with the base address aligned to the alignment argument. If alignment is zero,
  the default alignment of the allocator is used (usually `max_align_t`).
- If input is non-NULL it has been previously allocated by the allocator.
- If input is non-NULL with non-zero size, input is resized to at least bytes
  size. The pointer returned is NULL if resizing fails. Upon success, the
  pointer returned might not be equal to the pointer provided and is aligned to
  the provided alignment argument. If alignment is zero, the default alignment
  of the allocator is used (usually `max_align_t`).
- If input is non-NULL and size is 0, input is freed and NULL is returned.

@warning Wrapping `malloc`, `realloc`, and `free` is sufficient for hosted
environments where allocation alignment requirements do not exceed
`max_align_t`. For fully portable C Container Collection code, regardless of
user type alignment requirements, users are encouraged to use allocators that
respect alignment for allocating and resizing. */
typedef void *CCC_Allocator_interface(CCC_Allocator_arguments);

void *
std_allocate(CCC_Allocator_arguments const arguments) {
    if (!arguments.input && !arguments.bytes) {
        return NULL;
    }
    if (!arguments.input) {
        return malloc(arguments.bytes);
    }
    if (!arguments.bytes) {
        free(arguments.input);
        return NULL;
    }
    return realloc(arguments.input, arguments.bytes);
}
```

Then, containers that allocate accept a reference to a `CCC_Allocator` which is a simple bundle for the `CCC_Allocator_interface` and context.

```c
typedef struct {
    /** The allocator function to be passed to an allocating operation. */
    CCC_Allocator_interface *allocate;
    /** Additional state to pass to the allocator to help manage memory. */
    void *context;
} CCC_Allocator;

(void)CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    &(CCC_Allocator){.allocate = std_allocate}
);
```

The ability to pass context along with a `CCC_Allocator`, and have that same context provided as an argument to the allocator function call, is significant. This opens up the possibility for non-global allocators, or allocators with context that is not available until runtime. Consider the stack allocator that the CCC test suite uses to test container code paths that use an allocator, but avoid the overhead and cleanup that comes with the standard library allocator.

```c
struct Stack_allocator {
    /** The stack array of a compile time known capacity of user types. Always
        the first field of the struct. Do not move. */
    void *blocks;
    /** The size in bytes of the user type stored in this stack array. */
    size_t sizeof_type;
    /** The byte capacity of the compile time known array of user types. */
    size_t bytes_capacity;
    /** The bytes occupied within this array. Multiple of type size. */
    size_t bytes_occupied;
};

void *
stack_allocator_allocate(CCC_Allocator_arguments arguments) {
    if (!arguments.bytes || arguments.input || !arguments.context) {
        return NULL;
    }
    struct Stack_allocator *const allocator = arguments.context;
    size_t const remainder = arguments.bytes % allocator->sizeof_type;
    if (remainder) {
        arguments.bytes = arguments.bytes + allocator->sizeof_type - remainder;
    }
    if (allocator->bytes_occupied + arguments.bytes
        > allocator->bytes_capacity) {
        return NULL;
    }
    void *const block = (char *)allocator->blocks + allocator->bytes_occupied;
    allocator->bytes_occupied += arguments.bytes;
    return block;
}
```

It is basically a bump allocator that never frees. It requires context because it only exists within the stack frame of a function.

```c
struct Val {
    int val;
    CCC_Priority_queue_node elem;
};

int
main(void) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context = &stack_allocator_for((struct Val[1]){}),
    };
    CCC_Priority_queue priority_queue = CCC_priority_queue_default(
        struct Val,
        elem,
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = val_order}
    );
    CCC_priority_queue_push(
        &priority_queue,
        &(struct Val){.val = i}.elem,
        &allocator
    );
    return 0;
}
```

Full knowledge of the stack allocator is not required to understand the gist of this code block. There is an allocation function and a structure that manages the requested block of types the user plans to use. Context is a powerful concept for allocators that can provide fine-grained control and flexibility.

Passing allocators at call sites also ensures that the reader knows where in the code memory allocation may occur. Different types of allocators with various levels of debugging or safety features can also be swapped in at runtime or compile time easily. Finally, no boiler plate is required if the user plans on using only fixed size containers or managing their own memory.

#### Empty Allocators {#empty-allocators}

An empty `&(CCC_Allocator){}` can be passed to any function and means the following.

- No allocation, reallocation, or deallocation will occur.
- Any operation that requires allocation will fail and return the reason or change behavior accordingly.
- Flat and array based containers will copy the user provided element into their internal storage if sufficient capacity already exists.
- Intrusive containers will operate solely on the intrusive element within the user provided type, in-place. The user must guarantee the lifetime of the element and is responsible for any required deallocation.

### Never Pass NULL {#never-pass-null}

When reading user written C Container Collection code, the presence of `NULL` at a function or macro call site indicates a programmer error. Consider the following function call inserting an element into a flat hash map.

```c
CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    &std_allocator
);
```

The call site of this function helps the reader understand the operation of this code without any special knowledge of the CCC API: insert or assign this key value pair into the map and if the table needs to re-size use the provided allocator. What if the user has a fixed size map, or has otherwise reserved all the space they need, and wants to ensure no further memory is used? The user might be tempted to write the following.

**WARNING! The following example shows incorrect usage of the CCC API.**

```c
CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    NULL /* <-ERROR HERE! */
);
```

The user is trying to express that they don't want this function to allocate if the table is full. They want to keep the return value so they can check if this insertion failure has taken place and proceed with their program. However, using `NULL` is a programmer error and will return an error status because it obscures intent and would force a reader unfamiliar with the collection to consult documentation. Instead use this construction.

```c
CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    &(CCC_Allocator){}
);
```

This tells the reader that a key value pair will be inserted or assigned to an existing slot in the table and that resizing of the table is forbidden because a default initialized, empty `CCC_Allocator` is passed to the function. This is also a common pattern to use when clearing a container of its allocations and no advanced destructor functionality is required to act on each element before they are all freed.

```c
CCC_flat_hash_map_clear_and_free(
    &map,
    &(CCC_Destructor){},
    &std_allocator
);
```

No destructor has been implemented, but we follow the no passing `NULL` rule. An empty compound literal reference is the correct way to express that no user provided implementation of that argument exists. The API will respond accordingly. See documentation for details.

#### Beware of Intrusive Containers {#beware-of-intrusive-containers}

When passing an empty allocator some details must be remembered. One difference between `Flat_` and `Array_` based containers and the standard intrusive containers is that the former copy user provided types into a table-like structure. When an empty allocator is passed to these flat and array based containers resizing of the underlying table is forbidden. For intrusive containers the contract is different.

Consider this insertion into an intrusive priority queue.

```c
struct Priority {
    CCC_Priority_queue_node pq_elem;
    int priority;
};
CCC_priority_queue_push(
    &pq,
    &(struct Priority){.priority = 99}.pq_elem,
    &std_allocator
);
```

Here, the user type is passed to the function as an inline compound literal reference. This is OK because an allocator is provided. Internally, the container will try to allocate a new node for the priority queue with the provided allocator and copy in the provided values in the user provided type argument. This would not work if allocation is forbidden.

**WARNING! The following example shows possible incorrect usage of the CCC API.**

```c
CCC_priority_queue_push(
    &pq,
    &(struct Priority){.priority = 99}.pq_elem, /* <-ERROR HERE! */
    &(CCC_Allocator){}
);
```

Because allocation has been forbidden, the container does not assume anything about the scope and lifetime of the provided user element. It simply operates on the intrusive element the user is providing to maintain internal data structure invariants. Because an inline compound literal reference has been provided the scope and lifetime of that element is the enclosing block, and that is almost always a programmer error. The user must guarantee the lifetime of the element.

```c
struct Priority *elem = malloc(sizeof(struct Priority));
*elem = (struct Priority){.priority = 99};
CCC_priority_queue_push(
    &pq,
    &elem->pq_elem,
    &(CCC_Allocator){}
);
```

This required attention to scope and lifetime applies identically to all intrusive containers.

### Use the Correct Initializer {#use-the-correct-initializer}

Building upon the last rule, that `NULL` should never appear at a function or macro call site, the user should apply this rule to initializers. Every container offers a variety of initializers. If the container will be initialized as empty use the `_default()` initializer. Compare the following wrong choice with a subsequent correct choice.

**WARNING! The following example shows incorrect usage of the CCC API.**

```c
CCC_Flat_bitset b = CCC_flat_bitset_for(
    0,
    0,
    NULL /* <-ERROR HERE! */
);
```

This misuse breaks rule 2, and it is difficult to understand what the two zeros and `NULL` mean just from reading the code. Use the correct initializer.

```c
CCC_Flat_bitset b = CCC_flat_bitset_default();
```

For the `Flat_` and `Array_` based containers, consider using the `_with_storage()` initializers for expressive compile time initialization of fixed size containers.

```c
struct Key_val {
    int key;
    int val;
};
static CCC_Array_adaptive_map adaptive_map
    = CCC_array_adaptive_map_with_storage(
        key,
        (CCC_Key_comparator){.compare = compare_int_key},
        (struct Key_val[1024]){}
    );
static CCC_Array_tree_map tree_map
    = CCC_array_tree_map_with_storage(
        key,
        (CCC_Key_comparator){.compare = compare_int_key},
        (struct Key_val[1024]){}
    );
static CCC_Flat_hash_map hash_map
    = CCC_flat_hash_map_with_storage(
        key,
        ((CCC_Hasher){.hash = hash_int, .compare = compare_int_key}),
        (struct Key_val[1024]){}
    );
static CCC_Flat_priority_queue priority_queue
    = CCC_flat_priority_queue_with_storage(
        CCC_ORDER_LESSER,
        (CCC_Comparator){.compare = compare_int},
        (int[1024]){}
    );
static CCC_Flat_double_ended_queue ring_buffer
    = CCC_flat_double_ended_queue_with_storage(
        0,
        (int[1024]){}
    );
static CCC_Flat_buffer buffer
    = CCC_flat_buffer_with_storage(
        0,
        (int[1024]){}
    );
static CCC_Flat_bitset bitset
    = CCC_flat_bitset_with_storage(
        1024,
        (CCC_Bit[1024]){}
    );
```

The `_default()` initializers can be used at compile or runtime as well. The correct initializer will always present the more readable and expressive code. More complex initializers such as the `_for()` family of initializers are in place to support well-documented but rare corner cases of user memory management strategies. Always read the container initialization section of the container documentation to pick the right initializer for the job.

### Pointers Versus Handles {#pointers-versus-handles}

There are three important concepts about references the user must understand in the C Container Collection: pointers that may be invalidated, pointers that are stable, and handles that are stable. Here is a table summarizes the container types the user will encounter in this collection.


|Container|Memory Layout|Pointer Stable|Handle Stable|
|----|-------|-----------------|----------------|
|Intrusive|Non-Contiguous|Yes|N/A|
|Flat|Contiguous|No|N/A|
|Array|Contiguous|No|Yes|


#### Invalidated Pointers {#invalidated-pointers}

Pointer invalidation occurs only when a container is permitted to allocate. When a flat container is passed an allocator for an insert or push type operation, assume that all references are invalidated after that operation. For a simple dynamic buffer this may be obvious.

```c
int *front = CCC_flat_buffer_front(&buffer);
(void)CCC_flat_buffer_push_back(&buffer, &(int){7}, &allocator);
/* front may be invalid */
```

After the push operation, the reference `*front` may be invalid if the buffer has resized to fit the new element. This is also true for more complex containers such as the flat hash map.

```c
struct Key {
    int key;
    int value;
};
struct Key *a = CCC_flat_hash_map_or_insert(
    CCC_flat_hash_map_entry_wrap(&map, &(int){7}, &allocator),
    &(struct Key){.key = 7, .value = 7},
);
struct Key *b = CCC_flat_hash_map_or_insert(
    CCC_flat_hash_map_entry_wrap(&map, &(int){8}, &allocator),
    &(struct Key){.key = 8, .value = 8},
);
/* a may be invalid */
```

After the insert or assign operation, the reference `*a` may have been invalidated.

#### Stable Pointers {#stable-pointers}

Intrusive containers assume that the user defined types upon which they intrude are pointer stable. That means that when other elements are inserted or removed, the location in memory of an existing element in the container never changes. Consider an intrusive map.

```c
struct Key {
    CCC_Tree_map_node map_node;
    int key;
    int value;
};
struct Key *a = CCC_tree_map_or_insert(
    CCC_tree_map_entry_wrap(&map, &(int){7})
    &(struct Key){.key = 7, .value = 7}.map_node,
    &allocator
);
struct Key *b = CCC_tree_map_or_insert(
    CCC_tree_map_entry_wrap(&map, &(int){8})
    &(struct Key){.key = 8, .value = 8}.map_node,
    &allocator
);
/* a is still valid */
```

The map assumes the pointer `*a` is still valid after the insertion of `*b`. The container assumption is the same whether an allocator has been provided or the user is managing memory and has passed an empty `&(CCC_Allocator){}`.

#### Stable Handles {#stable-handles}

The special `Array_` based containers promise handle stability. This is similar to pointer stability, but provides slightly greater flexibility to the container implementation. A `CCC_Handle` and its unwrapped `CCC_Handle_index` remain stable for the lifetime of the element in the container which they represent.

```c
struct Key {
    int key;
    int value;
};
CCC_Handle_index a = CCC_array_tree_map_or_insert(
    CCC_array_tree_map_handle_wrap(&map, &(int){7}),
    &(struct Key){.key = 7, .value = 7},
    &allocator
);
CCC_Handle_index b = CCC_array_tree_map_or_insert(
    CCC_array_tree_map_handle_wrap(&map, &(int){8}),
    &(struct Key){.key = 8, .value = 8},
    &allocator
);
```

After the second element is inserted, `a` remains valid and can be provided to the container API to obtain a pointer to the user element. The underlying storage may have been resized or moved, but the location in the array remains stable. Therefore, it is important to remember that only the handle index remains stable, never the pointer obtained with that index.

## Installation {#installation}

- [INSTALL.md](INSTALL.md) - See the installation instructions for how to incorporate the C Container Collection into your project.

## Coverage Report {#coverage-report}

If you are looking to contribute, tests that increase coverage are a great start. View the [coverage report here](https://skeletoss.github.io/ccc/coverage).

The report is generating by running the test suite. See the tests folder for more.

## Back to Repository

[Return to repository](https://github.com/SkeletOSS/ccc).
