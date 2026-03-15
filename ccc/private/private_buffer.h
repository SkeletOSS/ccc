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
#ifndef CCC_PRIVATE_BUFFER_H
#define CCC_PRIVATE_BUFFER_H

/** @cond */
#include <assert.h>
#include <stddef.h>
#include <string.h>
/** @endcond */

#include "../types.h" /* IWYU pragma: keep */

/* NOLINTBEGIN(readability-identifier-naming) */

/** @internal A Buffer is a contiguous array of a uniform type. The user can
specify any type. The Buffer can be fixed size if no allocation permission is
given or dynamic if allocation permission is granted. The Buffer can also be
manually resized via the interface. */
struct CCC_Buffer {
    /** @internal The contiguous memory of uniform type. */
    void *data;
    /** @internal The current count of active Buffer slots. */
    size_t count;
    /** @internal The total Buffer slots possible for this array. */
    size_t capacity;
    /** @internal The size of the type the user stores in the buffer. */
    size_t sizeof_type;
};

/* @internal */
#define CCC_private_buffer_default(private_type_name)                          \
    {                                                                          \
        .sizeof_type = sizeof(private_type_name),                              \
    }

/** @internal Initializes the Buffer with a default size of 0. However the user
can specify that the Buffer has some count of elements from index
`[0, capacity - 1)` at initialization time. The Buffer assumes these elements
are contiguous. */
#define CCC_private_buffer_for(private_type_name, private_capacity,            \
                               private_count, private_data...)                 \
    {                                                                          \
        .data = (private_data),                                                \
        .sizeof_type = sizeof(private_type_name),                              \
        .count = (private_count),                                              \
        .capacity = (private_capacity),                                        \
    }

/** @internal For dynamic containers to perform the allocation and
initialization in one convenient step for user. */
#define CCC_private_buffer_from(private_allocator_context_pointer,             \
                                private_optional_capacity,                     \
                                private_compound_literal_array...)             \
    (__extension__({                                                           \
        typeof(*private_compound_literal_array)                                \
            *private_buffer_initializer_list = private_compound_literal_array; \
        struct CCC_Buffer private_buf = CCC_private_buffer_default(            \
            typeof(*private_buffer_initializer_list));                         \
        size_t const private_n = sizeof(private_compound_literal_array)        \
                               / sizeof(*private_buffer_initializer_list);     \
        size_t const private_cap = private_optional_capacity;                  \
        if (CCC_buffer_reserve(                                                \
                &private_buf,                                                  \
                (private_n > private_cap ? private_n : private_cap),           \
                (private_allocator_context_pointer))                           \
            == CCC_RESULT_OK) {                                                \
            (void)memcpy(private_buf.data, private_buffer_initializer_list,    \
                         private_n                                             \
                             * sizeof(*private_buffer_initializer_list));      \
            private_buf.count = private_n;                                     \
        }                                                                      \
        private_buf;                                                           \
    }))

/** @internal For dynamic containers to perform initialization and reservation
of memory in one step. */
#define CCC_private_buffer_with_capacity(                                      \
    private_type_name, private_allocator_context_pointer, private_capacity)    \
    (__extension__({                                                           \
        struct CCC_Buffer private_buf                                          \
            = CCC_private_buffer_default(private_type_name);                   \
        (void)CCC_buffer_reserve(&private_buf, (private_capacity),             \
                                 (private_allocator_context_pointer));         \
        private_buf;                                                           \
    }))

/** @internal Initializes a fixed size buffer with no allocation or context. */
#define CCC_private_buffer_with_storage(private_count,                         \
                                        private_compound_literal_array...)     \
    (struct {                                                                  \
        static_assert(sizeof(private_compound_literal_array) > 0,              \
                      "provide non-zero capacity compound literal array");     \
        static_assert(private_count                                            \
                          <= (sizeof(private_compound_literal_array)           \
                              / sizeof(*private_compound_literal_array)),      \
                      "provide count less than or equal to capacity of "       \
                      "compound literal array");                               \
        CCC_Buffer private;                                                    \
    }){{                                                                       \
           .data = (private_compound_literal_array),                           \
           .sizeof_type = sizeof(*private_compound_literal_array),             \
           .count = private_count,                                             \
           .capacity = sizeof(private_compound_literal_array)                  \
                     / sizeof(*private_compound_literal_array),                \
       }}                                                                      \
        .private

/** @internal */
#define CCC_private_buffer_emplace(private_buffer_pointer, index,              \
                                   private_type_compound_literal...)           \
    (__extension__({                                                           \
        typeof(private_type_compound_literal) *private_buffer_res = NULL;      \
        __auto_type private_i = (index);                                       \
        private_buffer_res                                                     \
            = CCC_buffer_at((private_buffer_pointer), private_i);              \
        if (private_buffer_res) {                                              \
            *private_buffer_res = private_type_compound_literal;               \
        }                                                                      \
        private_buffer_res;                                                    \
    }))

/** @internal */
#define CCC_private_buffer_emplace_back(private_buffer_pointer,                \
                                        private_allocator_pointer,             \
                                        private_type_compound_literal...)      \
    (__extension__({                                                           \
        typeof(private_type_compound_literal) *private_buffer_res = NULL;      \
        private_buffer_res = CCC_buffer_allocate_back(                         \
            (private_buffer_pointer), (private_allocator_pointer));            \
        if (private_buffer_res) {                                              \
            *private_buffer_res = private_type_compound_literal;               \
        }                                                                      \
        private_buffer_res;                                                    \
    }))

/* NOLINTEND(readability-identifier-naming) */

#endif /* CCC_PRIVATE_BUFFER_H */
