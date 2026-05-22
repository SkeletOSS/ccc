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
#ifndef CCC_PRIVATE_FLAT_BUFFER_H
#define CCC_PRIVATE_FLAT_BUFFER_H

/** @cond */
#include <stddef.h>
#include <string.h>
/** @endcond */

#include "../types.h" /* IWYU pragma: keep */

/* NOLINTBEGIN(readability-identifier-naming) */

/** @internal A Flat_buffer is a contiguous array of a uniform type. The user
can specify any type. The Flat_buffer can be fixed size if no allocation
permission is given or dynamic if allocation permission is granted. The
Flat_buffer can also be manually resized via the interface. */
struct CCC_Flat_buffer {
    /** @internal The contiguous memory of uniform type. */
    void *data;
    /** @internal The current count of active Flat_buffer slots. */
    size_t count;
    /** @internal The total Flat_buffer slots possible for this array. */
    size_t capacity;
    /** @internal The size of the type the user stores in the buffer. */
    size_t sizeof_type;
    /** @internal The alignment of the type the user stores in the buffer. */
    size_t alignof_type;
};

/* @internal */
#define CCC_private_flat_buffer_default(private_type_name)                     \
    (struct CCC_Flat_buffer) {                                                 \
        .sizeof_type = sizeof(private_type_name),                              \
        .alignof_type = alignof(private_type_name),                            \
    }

/** @internal Initializes the Flat_buffer with a default size of 0. However the
user can specify that the Flat_buffer has some count of elements from index
`[0, capacity - 1)` at initialization time. The Flat_buffer assumes these
elements are contiguous. */
#define CCC_private_flat_buffer_for(                                           \
    private_type_name, private_capacity, private_count, private_data...        \
)                                                                              \
    (struct CCC_Flat_buffer) {                                                 \
        .data = (private_data), .sizeof_type = sizeof(private_type_name),      \
        .alignof_type = alignof(private_type_name), .count = (private_count),  \
        .capacity = (private_capacity),                                        \
    }

/** @internal For dynamic containers to perform the allocation and
initialization in one convenient step for user. */
#define CCC_private_flat_buffer_from(                                          \
    private_allocator,                                                         \
    private_optional_capacity,                                                 \
    private_compound_literal_array...                                          \
)                                                                              \
    (struct { struct CCC_Flat_buffer private; }){(__extension__({              \
        typeof(*private_compound_literal_array)                                \
            *private_flat_buffer_initializer_list                              \
            = private_compound_literal_array;                                  \
        struct CCC_Flat_buffer private_buf = CCC_private_flat_buffer_default(  \
            typeof(*private_flat_buffer_initializer_list)                      \
        );                                                                     \
        size_t const private_n                                                 \
            = sizeof(private_compound_literal_array)                           \
            / sizeof(*private_flat_buffer_initializer_list);                   \
        size_t const private_cap = private_optional_capacity;                  \
        if (CCC_flat_buffer_reserve(                                           \
                &private_buf,                                                  \
                (private_n > private_cap ? private_n : private_cap),           \
                &(private_allocator)                                           \
            )                                                                  \
            == CCC_RESULT_OK) {                                                \
            (void)memcpy(                                                      \
                private_buf.data,                                              \
                private_flat_buffer_initializer_list,                          \
                private_n * sizeof(*private_flat_buffer_initializer_list)      \
            );                                                                 \
            private_buf.count = private_n;                                     \
        }                                                                      \
        private_buf;                                                           \
    }))}.private

/** @internal For dynamic containers to perform initialization and reservation
of memory in one step. */
#define CCC_private_flat_buffer_with_capacity(                                 \
    private_type_name, private_allocator, private_capacity                     \
)                                                                              \
    (struct { struct CCC_Flat_buffer private; }){(__extension__({              \
        struct CCC_Flat_buffer private_buf                                     \
            = CCC_private_flat_buffer_default(private_type_name);              \
        (void)CCC_flat_buffer_reserve(                                         \
            &private_buf, (private_capacity), &(private_allocator)             \
        );                                                                     \
        private_buf;                                                           \
    }))}.private

/** @internal Clang is more forgiving with what qualifies as a constant
expression for both constructing compound literals and using static asserts.
The static asserts are so helpful that it is worth every effort to give them
to the user. GCC is not so forgiving. */
#if defined(__clang__) || defined(__llvm__)
/** @internal Initializes a fixed size buffer with no allocation or context. */
#    define CCC_private_flat_buffer_with_storage(                              \
        private_count, private_compound_literal_array...                       \
    )                                                                          \
        (struct {                                                              \
            static_assert(                                                     \
                sizeof(private_compound_literal_array),                        \
                "provide non-zero capacity compound literal array"             \
            );                                                                 \
            static_assert(                                                     \
                private_count                                                  \
                    <= (sizeof(private_compound_literal_array)                 \
                        / sizeof(*private_compound_literal_array)),            \
                "provide count less than or equal to capacity of "             \
                "compound literal array"                                       \
            );                                                                 \
            CCC_Flat_buffer private;                                           \
        }){{                                                                   \
               .data = (private_compound_literal_array),                       \
               .sizeof_type = sizeof(*private_compound_literal_array),         \
               .alignof_type = alignof(*private_compound_literal_array),       \
               .count = private_count,                                         \
               .capacity = sizeof(private_compound_literal_array)              \
                         / sizeof(*private_compound_literal_array),            \
           }}                                                                  \
            .private
#else
/** @internal Initializes a fixed size buffer with no allocation or context. */
#    define CCC_private_flat_buffer_with_storage(                              \
        private_count, private_compound_literal_array...                       \
    )                                                                          \
        (struct CCC_Flat_buffer) {                                             \
            .data = (private_compound_literal_array),                          \
            .sizeof_type = sizeof(*private_compound_literal_array),            \
            .alignof_type = alignof(*private_compound_literal_array),          \
            .count = private_count,                                            \
            .capacity = sizeof(private_compound_literal_array)                 \
                      / sizeof(*private_compound_literal_array),               \
        }
#endif

/** @internal */
#define CCC_private_flat_buffer_emplace(                                       \
    private_flat_buffer_pointer, index, private_type_compound_literal...       \
)                                                                              \
    (__extension__({                                                           \
        __auto_type private_i = (index);                                       \
        typeof(private_type_compound_literal) *private_flat_buffer_res         \
            = CCC_flat_buffer_at((private_flat_buffer_pointer), private_i);    \
        if (private_flat_buffer_res) {                                         \
            *private_flat_buffer_res = private_type_compound_literal;          \
        }                                                                      \
        private_flat_buffer_res;                                               \
    }))

/** @internal */
#define CCC_private_flat_buffer_emplace_back(                                  \
    private_flat_buffer_pointer,                                               \
    private_allocator_pointer,                                                 \
    private_type_compound_literal...                                           \
)                                                                              \
    (__extension__({                                                           \
        typeof(private_type_compound_literal) *private_flat_buffer_res         \
            = CCC_flat_buffer_allocate_back(                                   \
                (private_flat_buffer_pointer), (private_allocator_pointer)     \
            );                                                                 \
        if (private_flat_buffer_res) {                                         \
            *private_flat_buffer_res = private_type_compound_literal;          \
        }                                                                      \
        private_flat_buffer_res;                                               \
    }))

/* NOLINTEND(readability-identifier-naming) */

#endif /* CCC_PRIVATE_FLAT_BUFFER_H */
