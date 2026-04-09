# Contributing

Not sure if this library will be used but might as well add this just in case. People can also ask questions in issues or the Discussions page.

## Organization

Here is a description of the current repository organization.

- `ccc` - The user-facing headers for each container. Documentation generation comes from these headers.
    - `ccc/private` - The type definitions for all containers. The macros used for initialization and more efficient insertion/construction can be found here. They are separated here to make the headers in `ccc` cleaner and more readable. Intended for developer use. Documented internally and users may read the generated documentation if they wish.
- `cmake` - Helper CMake files for installing the `ccc` library.
- `docs` - Folder for generating Doxygen Documentation locally and hosting online on the `gh-pages` branch.
- `tools` - The tools folder to make running `clang-format` and `clang-tidy` more convenient.
- `samples` - The sample programs used to demonstrate library capabilities. Samples are not included in releases.
- `source` - The implementations for the containers listed in the `ccc/` headers.
- `tests` - Testing code. The `run_tests.c` test runner. Also the `checkers.h` testing framework is well-documented for writing test cases. Tests are not included in releases.
    - `tests/[container]` - The folder for the container specific tests.
- `utility` - General helpers for the `tests` and `samples`. A `str_view` helper library is here to make `argc` and `argv` handling easier in the test runner and samples. Utilities for containers should not go here. If containers need to share utilities, they should find the header in `src`. The `utility` folder is removed from releases.

## Tooling

Required tools:

- [pre-commit](https://pre-commit.com/) - Run `pre-commit install` once the repo is forked/cloned. This will ensure pointless formatting changes don't enter the commit history.
- clangd - This is helpful for your editor if you have a `LSP` that can auto-format on save. This repo has both a `.clang-tidy` and `.clang-format` file that help `LSP`'s and other tools with warnings while you write.
- gcc-15.2+ - GCC 14 onward added excellent `-fanalyzer` capabilities. There are GCC presets in `CMakePresets.json` to run `-fanalyzer` and numerous sanitizers. These work best with newer GCC versions. Also, the defer keyword is used throughout samples and tests which is much newer C23 feature.
- clang-22+ - Clang added support for the `defer` keyword which is used throughout samples and tests. This is such a great feature that the code base uses it whenever possible in tests and samples. However, `defer` should not be used in core container code that is shipped with releases until it leaves technical specification and enters the next C2Y standard.
- .editorconfig - Settles cross platform issues like line endings and editor concerns like tabs vs spaces. Ensure your editor supports .editorconfig.
- .clang-format - This is needed less so now that `pre-commit` helps with formatting.
- .clang-tidy - Clang tidy should be run often on any changes to the code to catch obvious errors.

### Code Coverage

A code coverage report is hosted along with our documentation on GitHub Pages. It is updated to reflect the code coverage of the source file implementations whenever the main branch is updated. However, it would be very slow to wait for the deployment to update on every push to a pull request so run the tool locally if working on improving coverage.

A preset for gcc and clang are provided. I would recommend making a custom user preset, such as `cmake --preset=my-llvm-cov`, to obtain the newest tools possible for coverage reports. I have found that even in gcc gcov compatibility mode, llvm-cov generates the more accurate reports.

```zsh
make clean && cmake --preset=llvm-cov && make test && make coverage-developer
```

The developer version of the coverage report includes the tests to confirm all paths in written tests are executed. The `coverage-publish` target is the one that is hosted on GitHub pages along with the docs. It does not include tests to avoid appearing to pad percentages of coverage.

The code coverage report is now in `docs/coverage` and double clicking `docs/coverage/index.html` will open a navigable version in your local browser. Writing the command this way ensures that the process stops if any step fails.

When writing tests to increase coverage, rerun the above command to regenerate a clean report and reload the web page you are viewing in your local browser to see if the changes hit the targeted lines. Start by trying to increase function coverage to ensure container functionality is working correctly. Then start to focus on specific lines and paths that need to be exercised.

View the current [coverage report here](https://skeletoss.github.io/ccc/coverage). This report stays updated to the current state of the main branch. It does not track pull request branches.

### Presets

Add a `CMakeUserPresets.json` file so that you can run the sanitizer presets found in `CMakePresets.json`. Here is my setup as a sample.

```json
{
    "version": 3,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 23,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "my-gcc-debug",
            "inherits": ["gcc-debug"],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-15.2"
            }
        },
        {
            "name": "my-gcc-release",
            "inherits": ["gcc-release"],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-15.2"
            }
        },
        {
            "name": "my-clang-debug",
            "inherits": ["clang-debug"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_FLAGS": "-ggdb3 $env{CCC_WARNING_C_FLAGS}"
            }
        },
        {
            "name": "my-clang-release",
            "inherits": ["clang-release"],
            "generator": "Ninja"
        },
        {
            "name": "my-sanitize-debug",
            "inherits": ["gcc-sanitize-debug"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-15.2"
            }
        },
        {
            "name": "my-sanitize-release",
            "inherits": ["gcc-sanitize-release"],
            "generator": "Ninja",
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-15.2"
            }
        },
        {
            "name": "my-memory-sanitize-debug",
            "inherits": ["clang-memory-sanitize-debug"],
            "generator": "Ninja"
        },
        {
            "name": "my-memory-sanitize-release",
            "inherits": ["clang-memory-sanitize-release"],
            "generator": "Ninja"
        },
        {
            "name": "my-gcc-gcov",
            "inherits": ["gcc-gcov"],
            "cacheVariables": {
                "CMAKE_C_COMPILER": "gcc-15.2",
                "CCC_COVERAGE_TOOL": "gcov-15.2"
            }
        },
        {
            "name": "my-llvm-cov",
            "inherits": ["llvm-cov"],
            "generator": "Ninja"
        }
    ]
}
```

Now, I am able to run the GCC undefined behavior and address sanitizer builds as well as Clang's memory sanitizer builds. GCC 14+ provides the most robust `-fanalyzer` and UB/Adress sanitizer diagnostics. GCC has been improving these diagnostic tools rapidly since version 14.

## Workflow

Now that tooling is set up, the workflow is roughly as follows.

- Checkout a branch and start working on changes.
- When ready or almost ready, open a draft pr so CI can start running checks.
- Before completion run the following tools. Most run remotely on the PR as well but feedback is faster locally.
    - Run `make tidy` on debug and release builds and fix any issues from `clang-tidy`. This is not run on the remote repository.
    - Run `make clean && cmake --preset=my-sanitize-debug && cmake --build build -j8 --target ccc tests samples`. Replace the `-j8` flag with the number of cores on your system. This runs GCC's `-fanalyzer` and supplementary sanitizer flags. GCC's `-fanalyzer` will flag issues at compile time. Sanitizers requires you run actual programs so it can observe undefined behavior, buffer overflow, out of bounds memory access, etc. Run the tests `make test` and any samples your changes affect. Tests will run the PR remotely in case you forget, but feedback is faster locally.
    - Run `make clean && cmake --preset=my-release-debug && cmake --build build -j8 --target ccc tests samples`. Replace the `-j8` flag with the number of cores on your system. This is the same as the previous step just in release mode. Sometimes the compiler can optimize in such a way to create different issues the sanitizer can catch. Run `make test`. Tests will run the PR remotely in case you forget, but feedback is faster locally.
- Mark the pr as ready for review when all CI checks pass and tools show no errors locally.

## Targets

- `ccc` - The core C Container Collection library.
    - `cmake --preset=[PRESET HERE] && cmake --build build -j[NUM THREADS] --target ccc`
- `tests` - The tests.
    - `cmake --preset=[PRESET HERE] && make tests`
    - `make test` to run all the tests.
- `samples` - The samples.
    - `cmake --preset=[PRESET HERE] && make samples`
    - `./build/debug/bin/[SAMPLE_NAME]` to run a sample in debug mode.
    - `./build/bin/[SAMPLE_NAME]` to run a sample in release mode.

## Style

Formatting should be taken care of by the tools. Clang tidy will settle some small style matters. Defer to clang tidy suggestions when it makes them. Here are more general guidelines.

### No NULL Arguments

> [!IMPORTANT]
> `NULL` is never an acceptable argument at any C Container Collection function call site. If `NULL` appears as an argument in user code, a programmer error has occurred.

This is a critical rule that users should internalize immediately. The collection API usually accepts pointers to structs to alleviate register pressure and remain consistent across the code base. Also, with C23 semantics, passing anonymous compound literal references is an excellent way to write explicit code that communicates with the C type system rather than obtuse NULL arguments. Consider three uses of a function in the flat priority queue API.

```c
[[nodiscard]] void *CCC_flat_priority_queue_push(
    CCC_Flat_priority_queue *priority_queue,
    void const *type,
    void *temp,
    CCC_Allocator const *allocator
);
CCC_flat_priority_queue_push(&pq, &(int){4}, &(int){}, &allocator);
CCC_flat_priority_queue_push(&pq, &(int){4}, &(int){}, &(CCC_Allocator){});
CCC_flat_priority_queue_push(&pq, &(int){4}, &(int){}, /*  WRONG! */ NULL);
```

The first function call site tells us the following: in this queue, push the value 4, here is a swap slot, and here is an allocator to resize the underlying storage if needed. The second signature instead says in this queue, push the value 4, here is a swap slot, and this function may not allocate if resizing is required. The third example is wrong. An empty compound literal is much more readable at the call site. The reader can understand the general intent of the operation without needing to consult API documentation. The function would reject any NULL arguments and return either an error specifying a bad argument or NULL depending on the API contract.

### Use the C Type System at Interface Boundaries

Building upon the last point regarding how to avoid `NULL` at interface boundaries, it is important to communicate with C23's built in type system whenever possible. The last section showed extensive use of inline compound literal references, rather than `NULL`. This is an example of communicating with the C type system rather than arbitrarily chosen API argument conventions. Here is another example, from the README.md, showing fixed size data structure initialization.

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

When practical, these initializers accept arguments as compound literals rather than arguments in an arbitrary order. A type name and designated initializer fields within a compound literal offer much more readable initialization, to someone unfamiliar with the CCC API, than arguments in arbitrary, comma-separated order. Only use compound literal argument bundles when all omitted fields have well-defined, safe defaults. If a field is required for correct operation, such as the key field for maps, it should be provided as a direct argument instead. In this example, no `CCC_Hasher`, `CCC_Key_comparator`, or `CCC_Comparator` needs context provided to the `.context` field, and that is OK.

The use of compound literal arrays also helps communicate the nature of these data structures without having to supply more obtuse type, size, and capacity arguments. For most containers the provided compound literal is used directly as the backing storage. For containers such as the `Array_adaptive_map`, `Array_tree_map`, `Flat_hash_map`, and `Flat_bitset`, the provided compound literal array is used as a guideline for structuring efficient backing storage. The maps use it to construct the first array in a struct-of-arrays (SOA) layout. The bit set uses the request for a certain number of bits as a guide for determining how many blocks of a platform specified integer width to use for efficient operation.

In any case, the user benefits from expressing what they want with the C type system rather than our arbitrary interface conventions. This will make it easier for users to return to their code and understand its general ideas long after they have forgotten our API conventions.

### Naming

> [!IMPORTANT]
> Prioritize code reading over writing. Code for the user assumes little, simplifies usage, and minimizes implementation details. Code for the developer is documented, specifies design rationale, and maximizes available implementation details.

Given these constraints we use the following naming conventions.

#### No Abbreviations or Jargon

> [!IMPORTANT]
> Abbreviations in user facing headers included directly or transitively are prohibited.

While line length was a valid concern in the early days of C, this is no longer the case with modern tooling. Any time the user spends decoding our abbreviations is time they are not spending using the collection to solve their own problems. We also do not assume the user is familiar with common programming or C jargon such as `deq`, `iter`, `ptr`, `i`, `ctx`, or `rbegin` to name a few. Therefore, the previous jargon becomes `double_ended_queue`, `iterator`, `pointer`, `index`, `context`, and `reverse_begin` in our headers.

This minimizes user processing time while reading code and ensures maximum understanding across a wide range of developer experience levels.

Abbreviations may be used in the `source/` implementations at the developer's discretion, but explicit and clear naming is encouraged.

> [!WARNING]
> This is not enforceable with tooling.

#### Namespace

> [!IMPORTANT]
> Prefix everything user facing with `CCC_`.

All types, functions, and constants in headers that the user includes directly or transitively contain the `CCC_` prefix. This stands for *C Container Collection*. Many other libraries do something similar, such as SDL using the `SDL_` prefix. At this time, I am confident this is a unique prefix among the C library landscape.

Users may omit this prefix with name shortening on a per module basis. Every header allows the user to turn off the prefixing for that specific header's types, constants, and functions with the following definition, `#define [INTERFACE]_USING_NAMESPACE_CCC`. For example, here the user wants all fundamental types and the flat hash map interface to omit the `CCC_` prefix.

```c
#define TYPES_USING_NAMESPACE_CCC
#define FLAT_HASH_MAP_USING_NAMESPACE_CCC
#include "ccc/flat_hash_map.h"
#include "ccc/types.h"
```

This toggle affects only the specified headers, not the global namespace.

> [!NOTE]
> This is enforced by clang tidy.

#### Types

> [!IMPORTANT]
> Types use `Leading_upper_snake_case`. Member fields use `snake_case`. We `typedef` types in user facing headers. Use the following template.
> `[prefix]_[descriptor(s)]_[object]`.

Types, such as `struct`, `union`, and `enum`, that are defined by this library use `Leading_upper_snake_case`. While there are a few libraries in the C ecosystem that use `PascalCase` for types, see raylib or SDL, the pervasive style in C systems programming is `snake_case`. This library is intended for use in contexts such as kernels, compilers, or embedded environments. It should fit in to the best of its ability.

However, C does not allow types and functions to have the same name. Consider the function to obtain an entry, following Rust's API design: `CCC_flat_hash_map_entry()`. The entry type cannot also be named `CCC_flat_hash_map_entry`. So, types simply capitalize the first letter to distinguish themselves: `CCC_Flat_hash_map_entry`. This also improves code readability, grep-ability, and renaming via search and replace.

The types in the user facing headers are `typedef` to encourage the user to use them through the provided interface. Whether they are passing a `union`, `struct`, or `enum` to an interface function should not be important to the user. Types in the `private/` headers and `source/` files are never `typedef` to encourage maximum information and readability for developers.

Naming a type uses the prefix, any additional descriptors needed, and then the object being declared. For example the `CCC_Flat_hash_map` has the `CCC_` prefix, describes the map as `Flat` because it is in an array, and that it is a `hash_map`.

> [!NOTE]
> Capitalization is enforced by clang tidy. Logical naming is a convention.

#### Functions and Function-like Macros

> [!IMPORTANT]
> Functions and function-like macros use `snake_case` and the following template.
> `[prefix][container name][action or query]`.

This is the least visually invasive choice among a wide variety of code bases. It prioritizes readability. Function-like macros follow function style because they signal to the user that they must provide arguments. Also, any modern IDE or LSP configuration will trivially show the user that the macro is not a function.

Consider the flat hash map function to unwrap or obtain status regarding an entry. We have `CCC_flat_hash_map_occupied()` and `CCC_flat_hash_map_unwrap()`. Notice that the former omits the filler word `is` because there is no ambiguity when using this adjective to describe the current state. The `is` addition is helpful when the function would otherwise be ambiguous, such as `container_is_empty()` being more clear than `container_empty()`. The latter may suggest the container elements are being freed from memory.

Use `get_`, `is_`, or `has_` only when the function would be ambiguous without them or the function implements a common API from another language. For example, Rust's `get_key_value` function for associative containers.

> [!WARNING]
> This is enforced by clang tidy for functions but cannot be enforced for function-like macros. Logical naming is a convention.

#### Constants

> [!IMPORTANT]
> Constants from `#define` declarations or `enum` members are `UPPER_CASE`. Static translation unit constant variables are prefixed `static_`, only appearing in `source/` files. Prefer `enum` whenever possible.

An `enum` is preferred because in `C23` they can be type specified compile-time constants. They appear in debuggers, unlike `#define` constants. They also do not require space in the data segment, unlike static constants.

The `static_` prefix makes it clear that we are working with a variable that occupies space in memory, unlike an `enum` or `#define`.

> [!NOTE]
> This is enforced by clang tidy.

### Container Naming Schemes

#### Intrusive Containers

> [!IMPORTANT]
> Intrusive containers are the default and do not qualify their names with their memory layout. They offer pointer stability of the elements they allocate, or intrude upon if allocation is prohibited.

For example, the `CCC_Doubly_linked_list`, `CCC_Priority_queue`, and `CCC_Tree_map` all use the minimal words needed to qualify their data structure.

#### Flat Containers

> [!IMPORTANT]
> Flat containers qualify that they are stored contiguously in memory. References to elements in a flat container are invalidated upon insertion, resizing, and in some cases removal.

For example, the `CCC_Flat_hash_map`, `CCC_Flat_priority_queue`, and `CCC_Flat_double_ended_queue` all qualify their memory layout and use the `Flat_` prefix to distinguish their guarantees from the [`Array_`](#array-containers) containers.

#### Array Containers

> [!IMPORTANT]
> Array containers qualify that they are stored contiguously in memory **and offer handle stability**. Handles remain valid until removed, regardless of insertion of other elements, removal of other elements, and resizing operations.

For example, the `CCC_Array_tree_map` and `CCC_Array_adaptive_map` qualify their contiguous memory layout and their use of the `CCC_Handle` interface. The guarantees of handle stability from `Array_` container are much stronger than what `Flat_` containers offer. The `Array_` containers offer the same stability as intrusive containers but use a stable index rather than pointer. For this reason it is important that the interface document that a `CCC_Handle` and `CCC_Handle_index` should be saved rather than a pointer.

A benefit of handle stability is `O(1)` access to a user element referenced by a handle regardless of how the underlying storage moves in memory.

### Validate Parameters

Because this is a 3rd party library, we are responsible for avoiding undefined behavior and mysterious crashes from the user's perspective. This means checking any inputs to the user facing interface functions as non-NULL and valid for the operations we must complete.

Every function has some way to return early. In the worst case we must return `NULL` early. In functions that let us return a result or status, we can return the type of error we encountered to the user and there are mechanisms in `types.h` for viewing or logging more detailed error messages. This library does not use `errno` style reporting and it does not crash the user program if compiled in release mode with any assert style mechanism. We also do not use an optional error reporting parameter to all functions. While this is a valid approach, it would detract from one of the design goals of this library: detailed initialization leads to cleaner container operations. Cluttering every container function call site with NULL parameters or allowing a container to provide a function that ignores errors is currently not a design priority.

Using `assert` is only acceptable in this library code where internal invariants of the data structure are being checked and documentation for all functions must be written as if those asserts do not exist (they do not exist in releases). Asserts help in the development and testing stage rather than the user-facing releases.

If an error can be checked and reported to the user in a meaningful way, there should be a direct if branch and return in the code rather than an assert. See the Entry Interface implementations in any of the associative containers for examples of this reporting style. There can still be improvements made to the current code in this regard.

### Prohibited Dynamic Allocation

Variable length arrays are strictly prohibited. All containers must be designed to accommodate a non-allocating mode. This means the user can initialize the container to have no allocation permissions. However, for some containers this poses a challenge.

For example, a flat priority queue is a binary heap that operates by swapping elements. To swap elements we need a temporary space the size of one of the elements in the heap. We are not a header only template-style library so we must find space across an interface boundary. To ensure the user can store exactly as many elements as they wish in the flat array all signatures for functions that require swapping internally ask for a swap slot. For example here is the pop operation signature.

```c
CCC_Result CCC_flat_priority_queue_pop(CCC_Flat_priority_queue *q, void *tmp);
```

Other containers may do the same or be able to avoid pushing this space requirement to the user. Here is the ordered map swap entry operation that requires a swap slot.

```c
[[nodiscard]] CCC_Entry
CCC_adaptive_map_swap_entry(CCC_Adaptive_map *m,
                            CCC_Adaptive_map_node *key_val_output,
                            CCC_Adaptive_map_node *tmp);
```

For the equivalent handle version of this container the space requirement is handled internally.

```c
[[nodiscard]] CCC_Handle
CCC_array_adaptive_swap_handle(CCC_Array_adaptive_map *m, void *key_val_output);
```

The handle version of the container is required to preserve the 0th slot in the array as the nil node so it is able to swap when needed with this extra slot.

Variable length arrays are prohibited because they could cause hard to find bugs if the array caused a stack overflow in our library code for the user.

### Avoid Compilation Bloat

This library will never be as optimized as a C template-like library that generates type safe containers and in-lines all functions to interact with them. However, the benefit of this is that we will never bloat the user code with macro expansion every time they generate a new type container.

We can still try to offer the user some performance benefits on key code paths via our container specialized macros. All macros that accept compound literal user types write the user specified data directly to memory via casting instead of calling a generic copy function. The pseudo code for such an operation is roughly as follows.

```c
#define map_insert_or_assign_macro(container_pointer, key, allocator,          \
                                   compound_literal...)                        \
    (__extension__({                                                           \
        struct Entry entry = container_find_slot(container_pointer, key);      \
        if (entry.status & OCCUPIED) {                                         \
            *((typeof(compound_literal) *)container_slot_at(&entry))           \
                = compound_literal;                                            \
        } else {                                                               \
            *((typeof(compound_literal) *)container_new_slot(                  \
                &entry, &(allocator)                                           \
            )) = compound_literal;                                             \
        }                                                                      \
        entry;                                                                 \
    }))
```

Notice that there is minimal scaffolding around calls to the container interface functions. These calls generate the needed memory to write the user data directly. This allows the compiler to optimize the write freely while we do not generate any container machinery code at the call-site. This container specific code can be quite complex, depending on the container, so ensuring we do not duplicate that code is critical. The user is able to leverage an optimizing compiler without exploding their binary size or defining specialized containers throughout their code base.

Therefore, when adding these macros, ensure core container machinery is contained within interface functions.

### No Defer in Container Code

The `defer` keyword is finally being added to major compilers. It is a technical specification for now but will likely enter the next `C2Y` standard. Samples and tests can use `defer` but refrain from using it in any `ccc/*` or `source/*` files that ship with release packages until the feature makes it into the language officially.

## To Do

We are very close to `v1.0`. Now that the first pass of code coverage and documentation is complete I am just working on naming everything correctly and ensuring the interfaces are as usable as possible. Suggestions are welcome during this phase.

