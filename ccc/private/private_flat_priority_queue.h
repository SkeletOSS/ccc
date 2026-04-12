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
#ifndef CCC_PRIVATE_FLAT_PRIORITY_QUEUE_H
#define CCC_PRIVATE_FLAT_PRIORITY_QUEUE_H

/** @cond */
#include <stddef.h>
/** @endcond */

#include "../flat_buffer.h"
#include "../types.h"

/* NOLINTBEGIN(readability-identifier-naming) */

/** @internal A flat priority queue is a binary heap in a contiguous buffer
storing an implicit complete binary tree; elements are stored contiguously from
`[0, N)`. Starting at any node in the tree at index i the parent is at `(i - 1)
/ 2`, the left child is at `(i * 2) + 1`, and the right child is at `(i * 2) +
2`. The heap can be initialized as a min or max heap due to the use of the three
way comparison function. */
struct CCC_Flat_priority_queue {
    /** @internal The underlying Flat_buffer owned by the flat_priority_queue.
     */
    CCC_Flat_buffer buffer;
    /** @internal The order `CCC_ORDER_LESSER` (min) or `CCC_ORDER_GREATER`
     * (max) of the flat_priority_queue. */
    CCC_Order order;
    /** @internal The user defined three way comparison function. */
    CCC_Comparator comparator;
};

/*========================    Private Interface     =========================*/

/** @internal */
size_t CCC_private_flat_priority_queue_bubble_up(
    struct CCC_Flat_priority_queue const *, void *, size_t
);
/** @internal */
void CCC_private_flat_priority_queue_heap_order(
    struct CCC_Flat_priority_queue const *, void *
);
/** @internal */
void *CCC_private_flat_priority_queue_update_fixup(
    struct CCC_Flat_priority_queue const *, void *, void *
);

/*======================    Macro Implementations    ========================*/

/** @internal */
#define CCC_private_flat_priority_queue_default(                               \
    private_type_name, private_order, private_comparator...                    \
)                                                                              \
    (struct CCC_Flat_priority_queue) {                                         \
        .buffer = CCC_flat_buffer_default(private_type_name),                  \
        .order = (private_order), .comparator = private_comparator,            \
    }

/** @internal */
#define CCC_private_flat_priority_queue_for(                                   \
    private_type_name,                                                         \
    private_order,                                                             \
    private_comparator,                                                        \
    private_capacity,                                                          \
    private_data_pointer                                                       \
)                                                                              \
    (struct CCC_Flat_priority_queue) {                                         \
        .buffer = CCC_flat_buffer_for(                                         \
            private_type_name, private_capacity, 0, private_data_pointer       \
        ),                                                                     \
        .order = (private_order), .comparator = (private_comparator),          \
    }

/** @internal */
#define CCC_private_flat_priority_queue_heapify(                               \
    private_type_name,                                                         \
    private_order,                                                             \
    private_comparator,                                                        \
    private_capacity,                                                          \
    private_size,                                                              \
    private_data_pointer...                                                    \
)                                                                              \
    (struct { struct CCC_Flat_priority_queue private; }){(__extension__({      \
        typeof(*(                                                              \
            private_data_pointer                                               \
        )) *private_flat_priority_queue_heapify_data = private_data_pointer;   \
        struct CCC_Flat_priority_queue private_flat_priority_queue_heapify_res \
            = CCC_private_flat_priority_queue_for(                             \
                private_type_name,                                             \
                private_order,                                                 \
                private_comparator,                                            \
                private_capacity,                                              \
                private_flat_priority_queue_heapify_data                       \
            );                                                                 \
        private_flat_priority_queue_heapify_res.buffer.count = (private_size); \
        CCC_private_flat_priority_queue_heap_order(                            \
            &private_flat_priority_queue_heapify_res, &(private_type_name){}   \
        );                                                                     \
        private_flat_priority_queue_heapify_res;                               \
    }))}.private

/** @internal */
#define CCC_private_flat_priority_queue_from(                                  \
    private_order,                                                             \
    private_comparator,                                                        \
    private_allocator,                                                         \
    private_optional_capacity,                                                 \
    private_compound_literal_array...                                          \
)                                                                              \
    (struct { struct CCC_Flat_priority_queue private; }){(__extension__({      \
        struct CCC_Flat_priority_queue private_flat_priority_queue = {         \
            .buffer = CCC_flat_buffer_from(                                    \
                private_allocator,                                             \
                private_optional_capacity,                                     \
                private_compound_literal_array                                 \
            ),                                                                 \
            .order = (private_order),                                          \
            .comparator = (private_comparator),                                \
        };                                                                     \
        if (private_flat_priority_queue.buffer.count) {                        \
            CCC_private_flat_priority_queue_heap_order(                        \
                &private_flat_priority_queue,                                  \
                &(typeof(*private_compound_literal_array)){}                   \
            );                                                                 \
        }                                                                      \
        private_flat_priority_queue;                                           \
    }))}.private

/** @internal */
#define CCC_private_flat_priority_queue_with_capacity(                         \
    private_type_name,                                                         \
    private_order,                                                             \
    private_comparator,                                                        \
    private_allocator,                                                         \
    private_capacity                                                           \
)                                                                              \
    (struct { struct CCC_Flat_priority_queue private; }){(__extension__({      \
        struct CCC_Flat_priority_queue private_flat_priority_queue = {         \
            .buffer = CCC_flat_buffer_with_capacity(                           \
                private_type_name, private_allocator, private_capacity         \
            ),                                                                 \
            .order = (private_order),                                          \
            .comparator = (private_comparator),                                \
        };                                                                     \
        private_flat_priority_queue;                                           \
    }))}.private

/** @internal Clang is more forgiving with what qualifies as a constant
expression for both constructing compound literals and using static asserts.
The static asserts are so helpful that it is worth every effort to give them
to the user. GCC is not so forgiving. */
#if defined(__clang__) || defined(__llvm__)
#    define CCC_private_flat_priority_queue_with_storage(                      \
        private_order, private_comparator, private_compound_literal            \
    )                                                                          \
        (struct {                                                              \
            static_assert(                                                     \
                (private_order) == CCC_ORDER_LESSER                            \
                    || (private_order) == CCC_ORDER_GREATER,                   \
                "flat priority queue must be a min or max priority queue"      \
            );                                                                 \
            struct CCC_Flat_priority_queue private;                            \
        }){{                                                                   \
               .buffer                                                         \
               = CCC_flat_buffer_with_storage(0, private_compound_literal),    \
               .order = (private_order),                                       \
               .comparator = (private_comparator),                             \
           }}                                                                  \
            .private
#else
/** @internal */
#    define CCC_private_flat_priority_queue_with_storage(                      \
        private_order, private_comparator, private_compound_literal            \
    )                                                                          \
        (struct CCC_Flat_priority_queue) {                                     \
            .buffer                                                            \
                = CCC_flat_buffer_with_storage(0, private_compound_literal),   \
                .order = (private_order), .comparator = (private_comparator),  \
        }
#endif

/** @internal This macro "returns" a value thanks to clang and gcc statement
   expressions. See documentation in the flat priority_queueueue header for
   usage. The ugly details of the macro are hidden here in the impl header. */
#define CCC_private_flat_priority_queue_emplace(                               \
    flat_priority_queue, private_allocator_pointer, type_compound_literal...   \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Flat_priority_queue *private_flat_priority_queue            \
            = (flat_priority_queue);                                           \
        typeof(type_compound_literal) *private_flat_priority_queue_res         \
            = CCC_flat_buffer_allocate_back(                                   \
                &private_flat_priority_queue->buffer,                          \
                private_allocator_pointer                                      \
            );                                                                 \
        if (private_flat_priority_queue_res) {                                 \
            *private_flat_priority_queue_res = type_compound_literal;          \
            if (private_flat_priority_queue->buffer.count > 1) {               \
                private_flat_priority_queue_res = CCC_flat_buffer_at(          \
                    &private_flat_priority_queue->buffer,                      \
                    CCC_private_flat_priority_queue_bubble_up(                 \
                        private_flat_priority_queue,                           \
                        &(typeof(type_compound_literal)){},                    \
                        private_flat_priority_queue->buffer.count - 1          \
                    )                                                          \
                );                                                             \
            } else {                                                           \
                private_flat_priority_queue_res = CCC_flat_buffer_at(          \
                    &private_flat_priority_queue->buffer, 0                    \
                );                                                             \
            }                                                                  \
        }                                                                      \
        private_flat_priority_queue_res;                                       \
    }))

/** @internal Only one update fn is needed because there is no advantage to
   updates if it is known they are min/max increase/decrease etc. */
#define CCC_private_flat_priority_queue_update_with(                           \
    flat_priority_queue_pointer,                                               \
    closure_parameter,                                                         \
    update_closure_over_closure_parameter...                                   \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Flat_priority_queue *const private_flat_priority_queue      \
            = (flat_priority_queue_pointer);                                   \
        typeof(*closure_parameter) *                                           \
            private_flat_priority_queue_updated_element = (closure_parameter); \
        if (private_flat_priority_queue                                        \
            && !CCC_flat_buffer_is_empty(                                      \
                &private_flat_priority_queue->buffer                           \
            )) {                                                               \
            {update_closure_over_closure_parameter};                           \
            private_flat_priority_queue_updated_element                        \
                = CCC_private_flat_priority_queue_update_fixup(                \
                    private_flat_priority_queue,                               \
                    private_flat_priority_queue_updated_element,               \
                    &(typeof(*closure_parameter)){}                            \
                );                                                             \
        }                                                                      \
        private_flat_priority_queue_updated_element;                           \
    }))

/** @internal */
#define CCC_private_flat_priority_queue_increase_with(                         \
    flat_priority_queue_pointer,                                               \
    closure_parameter,                                                         \
    increase_closure_over_closure_parameter...                                 \
)                                                                              \
    CCC_private_flat_priority_queue_update_with(                               \
        flat_priority_queue_pointer,                                           \
        closure_parameter,                                                     \
        increase_closure_over_closure_parameter                                \
    )

/** @internal */
#define CCC_private_flat_priority_queue_decrease_with(                         \
    flat_priority_queue_pointer,                                               \
    closure_parameter,                                                         \
    decrease_closure_over_closure_parameter...                                 \
)                                                                              \
    CCC_private_flat_priority_queue_update_with(                               \
        flat_priority_queue_pointer,                                           \
        closure_parameter,                                                     \
        decrease_closure_over_closure_parameter                                \
    )

/* NOLINTEND(readability-identifier-naming) */

#endif /* CCC_PRIVATE_FLAT_PRIORITY_QUEUE_H */
