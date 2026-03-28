# CCC DOC

- [Usage](#usage)
- [CCC Cardinal Rules](#ccc-cardinal-rules)
- [Installation](#installation)
- [Coverage Report](#coverage-report)

## Usage

Welcome to the doc! Follow the links below to become familiar with the collection.

- [ccc](https://skeletoss.github.io/ccc/files.html) - The collection of containers. Click the header for the desired container to view its interface.
    - Each `*.h` file listed contains the entire interface for that container.
    - All types and functions are listed at the top of each page.
    - All types and functions have links to the extended documentation below.
    - Because Doxygen takes directly from the source files the same documentation can be read directly in the header files distributed with the library.
- [types.h](https://skeletoss.github.io/ccc/types_8h.html) - The fundamental types that all containers use.
    - Become familiar with the memory allocation interface and memory permissions.
    - Understand the Entry and Handle interfaces for efficient associative container operations.
- [traits.h](https://skeletoss.github.io/ccc/traits_8h.html) - Generic versions of shared functionality across containers.
    - Write more expressive less distracting code with shorter naming.
    - Combine shared functionality across containers with one united interface.

## CCC Cardinal Rules

1. [Understand allocators](#understand-allocators).
2. [Never pass `NULL` to any C Container Collection function or macro](#never-pass-null).
3. [Use the correct initializer](#use-the-correct-initializer).
4. [Understand pointer versus handle stability](#pointers-versus-handles).

### Understand Allocators

### Never Pass NULL

When reading user written C Container Collection code, the presence of `NULL` at a function or macro call site indicates a programmer error. Consider the following function call inserting an element into a flat hash map.

```c
(void)CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    &std_allocator
);
```

The call site of this function helps the reader understand the operation of this code without any special knowledge of the CCC API: insert or assign this key value pair into the map and if the table needs to re-size use the provided allocator; discard the return value of this function. What if the user has a fixed size map, or has otherwise reserved all the space they need, and wants to ensure no further memory is used? The user might be tempted to write the following.

> [!WARNING]
> The following example shows incorrect usage of the CCC API.

```c
CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    NULL /* <-ERROR HERE! */
);
```

The user is trying to express that they don't want this function to allocate if the table is full. They want to keep the return value so they can check if this insertion failure has taken place and proceed with their program. However, using `NULL` is a programmer error and will return an error status because it forces the reader to consult documentation to understand the function call. Instead use this construction.

```c
CCC_Entry const e = CCC_flat_hash_map_insert_or_assign(
    &map,
    &(struct Val){.key = 37, .val = 1},
    &(CCC_Allocator){}
);
```

This tells the reader that a key value pair will be inserted or assigned to an existing slot in the table and that resizing of the table is forbidden because a default initialized, empty `CCC_Allocator` is passed to the function. An empty compound literal reference is the correct way to express that no user provided implementation of that argument exists. The API will respond accordingly. See documentation for details.

#### Beware of Intrusive Containers

When passing an empty allocator some details must be remembered. One difference between `Flat_` and `Array_` based containers and the standard intrusive containers is that the former copy user provided types into a table-like structure. When an empty allocator is passed to these flat and array based containers only resizing of the underlying table is forbidden. For intrusive containers the contract is different.

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

```c
struct Priority *elem = malloc(sizeof(struct Priority));
CCC_priority_queue_push(
    &pq,
    &elem->pq_elem,
    &(CCC_Allocator){}
);
```

Because allocation has been forbidden, the container does not assume anything about the scope and lifetime of the provided user element. It simply operates on the intrusive element the user is providing to maintain internal data structure invariants. If an inline compound literal reference had been provided the scope and lifetime of that element would be the enclosing block, and that is almost always a programmer error.

This required attention to scope and lifetime applies identically to all intrusive containers.

### Use the Correct Initializer

### Pointers Versus Handles

## Installation

- [INSTALL.md](INSTALL.md) - See the installation instructions for how to incorporate the C Container Collection into your project.

## Coverage Report

If you are looking to contribute, tests that increase coverage are a great start. View the [coverage report here](https://skeletoss.github.io/ccc/coverage).

The report is generating by running the test suite. See the tests folder for more.
