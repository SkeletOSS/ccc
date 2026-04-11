/** Copyright 2025 Alexander G. Lopez

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */
/** C23 provided headers. */
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/** CCC provided headers. */
#include "ccc/configuration.h"
#include "ccc/flat_buffer.h"
#include "ccc/flat_priority_queue.h"
#include "ccc/private/private_flat_priority_queue.h"
#include "ccc/sort.h"
#include "ccc/types.h"

enum : size_t {
    START_CAP = 8,
};

/*=====================      Prototypes      ================================*/

static size_t index_of(struct CCC_Flat_priority_queue const *, void const *);
static CCC_Tribool
wins(void const *, void const *, CCC_Order, CCC_Comparator const *);
static size_t
bubble_up(CCC_Flat_buffer *, size_t, void *, CCC_Order, CCC_Comparator const *);
static size_t bubble_down(
    CCC_Flat_buffer *, size_t, void *, CCC_Order, CCC_Comparator const *
);
static size_t update_fixup(struct CCC_Flat_priority_queue *, void *, void *);
static void
heapify(CCC_Flat_buffer *, void *, CCC_Order, CCC_Comparator const *);
static void
heapsort(CCC_Flat_buffer *, void *, CCC_Order, CCC_Comparator const *);
static void
destroy_each(struct CCC_Flat_priority_queue *, CCC_Destructor const *);
static void swap(CCC_Flat_buffer *, void *, void *, void *);
static void *at(CCC_Flat_buffer const *buffer, size_t i);

/*=====================       Interface      ================================*/

CCC_Result
CCC_flat_priority_queue_copy_heapify(
    CCC_Flat_priority_queue *const priority_queue,
    CCC_Flat_buffer const *const buffer,
    void *const temp,
    CCC_Allocator const *const allocator
) {
    if (!priority_queue || !temp || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    CCC_Result const copy_result
        = CCC_flat_buffer_copy(&priority_queue->buffer, buffer, allocator);
    if (copy_result != CCC_RESULT_OK) {
        return copy_result;
    }
    heapify(
        &priority_queue->buffer,
        temp,
        priority_queue->order,
        &priority_queue->comparator
    );
    return CCC_RESULT_OK;
}

CCC_Flat_priority_queue
CCC_flat_priority_queue_in_place_heapify(
    CCC_Flat_buffer *const buffer,
    void *const temp,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    if (!buffer || !temp || !comparator || !comparator->compare
        || (order != CCC_ORDER_GREATER && order != CCC_ORDER_LESSER)) {
        return (CCC_Flat_priority_queue){
            .order = CCC_ORDER_ERROR,
        };
    }
    CCC_Flat_priority_queue priority_queue = {
        .buffer = *buffer,
        .order = order,
        .comparator = *comparator,
    };
    heapify(
        &priority_queue.buffer,
        temp,
        priority_queue.order,
        &priority_queue.comparator
    );
    *buffer = (CCC_Flat_buffer){};
    return priority_queue;
}

void *
CCC_flat_priority_queue_push(
    CCC_Flat_priority_queue *const priority_queue,
    void const *const type,
    void *const temp,
    CCC_Allocator const *const allocator
) {
    if (!priority_queue || !type || !temp || !allocator) {
        return NULL;
    }
    void *const new
        = CCC_flat_buffer_allocate_back(&priority_queue->buffer, allocator);
    if (!new) {
        return NULL;
    }
    if (new != type) {
        (void)memcpy(new, type, priority_queue->buffer.sizeof_type);
    }
    assert(temp);
    size_t const i = bubble_up(
        &priority_queue->buffer,
        priority_queue->buffer.count - 1,
        temp,
        priority_queue->order,
        &priority_queue->comparator
    );
    assert(i < priority_queue->buffer.count);
    return at(&priority_queue->buffer, i);
}

CCC_Result
CCC_flat_priority_queue_pop(
    CCC_Flat_priority_queue *const priority_queue, void *const temp
) {
    if (!priority_queue || !temp || !priority_queue->buffer.count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    --priority_queue->buffer.count;
    if (!priority_queue->buffer.count) {
        return CCC_RESULT_OK;
    }
    swap(
        &priority_queue->buffer,
        temp,
        at(&priority_queue->buffer, 0),
        at(&priority_queue->buffer, priority_queue->buffer.count)
    );
    (void)bubble_down(
        &priority_queue->buffer,
        0,
        temp,
        priority_queue->order,
        &priority_queue->comparator
    );
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_priority_queue_erase(
    CCC_Flat_priority_queue *const priority_queue,
    void *const type,
    void *const temp
) {
    if (!priority_queue || !type || !temp || !priority_queue->buffer.count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    size_t const i = index_of(priority_queue, type);
    --priority_queue->buffer.count;
    if (i == priority_queue->buffer.count) {
        return CCC_RESULT_OK;
    }
    swap(
        &priority_queue->buffer,
        temp,
        at(&priority_queue->buffer, i),
        at(&priority_queue->buffer, priority_queue->buffer.count)
    );
    CCC_Order const order_res
        = priority_queue->comparator.compare((CCC_Comparator_arguments){
            .type_left = at(&priority_queue->buffer, i),
            .type_right
            = at(&priority_queue->buffer, priority_queue->buffer.count),
            .context = priority_queue->comparator.context,
        });
    if (order_res == priority_queue->order) {
        (void)bubble_up(
            &priority_queue->buffer,
            i,
            temp,
            priority_queue->order,
            &priority_queue->comparator
        );
    } else if (order_res != CCC_ORDER_EQUAL) {
        (void)bubble_down(
            &priority_queue->buffer,
            i,
            temp,
            priority_queue->order,
            &priority_queue->comparator
        );
    }
    return CCC_RESULT_OK;
}

void *
CCC_flat_priority_queue_update(
    CCC_Flat_priority_queue *const priority_queue,
    void *const type,
    void *const temp,
    CCC_Modifier const *const modifier
) {
    if (!priority_queue || !type || !temp || !modifier || !modifier->modify
        || !priority_queue->buffer.count) {
        return NULL;
    }
    modifier->modify((CCC_Arguments){
        .type = type,
        .context = modifier->context,
    });
    return at(
        &priority_queue->buffer, update_fixup(priority_queue, type, temp)
    );
}

/* There are no efficiency benefits in knowing an increase will occur. */
void *
CCC_flat_priority_queue_increase(
    CCC_Flat_priority_queue *const priority_queue,
    void *const type,
    void *const temp,
    CCC_Modifier const *const modifier
) {
    return CCC_flat_priority_queue_update(priority_queue, type, temp, modifier);
}

/* There are no efficiency benefits in knowing an decrease will occur. */
void *
CCC_flat_priority_queue_decrease(
    CCC_Flat_priority_queue *const priority_queue,
    void *const type,
    void *const temp,
    CCC_Modifier const *const modifier
) {
    return CCC_flat_priority_queue_update(priority_queue, type, temp, modifier);
}

void *
CCC_flat_priority_queue_front(
    CCC_Flat_priority_queue const *const priority_queue
) {
    if (!priority_queue || !priority_queue->buffer.count) {
        return NULL;
    }
    return at(&priority_queue->buffer, 0);
}

CCC_Tribool
CCC_flat_priority_queue_is_empty(
    CCC_Flat_priority_queue const *const priority_queue
) {
    if (!priority_queue) {
        return CCC_TRIBOOL_ERROR;
    }
    return CCC_flat_buffer_is_empty(&priority_queue->buffer);
}

CCC_Count
CCC_flat_priority_queue_count(
    CCC_Flat_priority_queue const *const priority_queue
) {
    if (!priority_queue) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return CCC_flat_buffer_count(&priority_queue->buffer);
}

CCC_Count
CCC_flat_priority_queue_capacity(
    CCC_Flat_priority_queue const *const priority_queue
) {
    if (!priority_queue) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return CCC_flat_buffer_capacity(&priority_queue->buffer);
}

void *
CCC_flat_priority_queue_data(
    CCC_Flat_priority_queue const *const priority_queue
) {
    return priority_queue ? CCC_flat_buffer_begin(&priority_queue->buffer)
                          : NULL;
}

CCC_Order
CCC_flat_priority_queue_order(
    CCC_Flat_priority_queue const *const priority_queue
) {
    return priority_queue ? priority_queue->order : CCC_ORDER_ERROR;
}

CCC_Result
CCC_flat_priority_queue_reserve(
    CCC_Flat_priority_queue *const priority_queue,
    size_t const to_add,
    CCC_Allocator const *const allocator
) {
    if (!priority_queue || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    return CCC_flat_buffer_reserve(&priority_queue->buffer, to_add, allocator);
}

CCC_Result
CCC_flat_priority_queue_copy(
    CCC_Flat_priority_queue *const destination,
    CCC_Flat_priority_queue const *const source,
    CCC_Allocator const *const allocator
) {
    if (!destination || !source || source == destination || !allocator
        || (destination->buffer.capacity < source->buffer.capacity
            && !allocator->allocate)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    destination->order = source->order;
    destination->comparator = source->comparator;
    if (destination->buffer.capacity < source->buffer.capacity) {
        CCC_Result const r = CCC_flat_buffer_allocate(
            &destination->buffer, source->buffer.capacity, allocator
        );
        if (r != CCC_RESULT_OK) {
            return r;
        }
        destination->buffer.capacity = source->buffer.capacity;
    }
    destination->buffer.count = source->buffer.count;
    /* It is ok to only copy count elements because we know that all elements
       in a binary heap are contiguous from [0, C), where C is count. */
    if (source->buffer.count) {
        if (!source->buffer.data || !destination->buffer.data) {
            return CCC_RESULT_ARGUMENT_ERROR;
        }
        (void)memcpy(
            destination->buffer.data,
            source->buffer.data,
            source->buffer.count * source->buffer.sizeof_type
        );
    }
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_priority_queue_clear(
    CCC_Flat_priority_queue *const priority_queue,
    CCC_Destructor const *const destructor
) {
    if (!priority_queue || !destructor) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (destructor->destroy) {
        destroy_each(priority_queue, destructor);
    }
    return CCC_flat_buffer_count_set(&priority_queue->buffer, 0);
}

CCC_Result
CCC_flat_priority_queue_clear_and_free(
    CCC_Flat_priority_queue *const priority_queue,
    CCC_Destructor const *const destructor,
    CCC_Allocator const *const allocator
) {
    if (!priority_queue || !destructor || !allocator) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (destructor->destroy) {
        destroy_each(priority_queue, destructor);
    }
    return CCC_flat_buffer_allocate(&priority_queue->buffer, 0, allocator);
}

CCC_Tribool
CCC_flat_priority_queue_validate(
    CCC_Flat_priority_queue const *const priority_queue
) {
    if (!priority_queue) {
        return CCC_TRIBOOL_ERROR;
    }
    size_t const count = priority_queue->buffer.count;
    if (count <= 1) {
        return CCC_TRUE;
    }
    for (size_t i = 0,
                left = (i * 2) + 1,
                right = (i * 2) + 2,
                end = (count - 2) / 2;
         i <= end;
         ++i, left = (i * 2) + 1, right = (i * 2) + 2) {
        void const *const this_pointer = at(&priority_queue->buffer, i);
        /* Putting the child in the comparison function first evaluates
           the child's three way comparison in relation to the parent. If
           the child beats the parent in total ordering (min/max) something
           has gone wrong. */
        if (left < count
            && wins(
                at(&priority_queue->buffer, left),
                this_pointer,
                priority_queue->order,
                &priority_queue->comparator
            )) {
            return CCC_FALSE;
        }
        if (right < count
            && wins(
                at(&priority_queue->buffer, right),
                this_pointer,
                priority_queue->order,
                &priority_queue->comparator
            )) {
            return CCC_FALSE;
        }
    }
    return CCC_TRUE;
}

/*===================     Interface in sort.h   =============================*/

CCC_Result
CCC_sort_heapsort(
    CCC_Flat_buffer *const buffer,
    void *const temp,
    CCC_Order order,
    CCC_Comparator const *const comparator
) {
    if (!buffer || !temp || !comparator || !comparator->compare
        || (order != CCC_ORDER_GREATER && order != CCC_ORDER_LESSER)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    /* For sorting the user expects the buffer to be in the order they specify.
       Just like they would expect their input order to the priority queue to
       place the least or greatest element closest to the root. However,
       heap sort fills a buffer from back to front, so flip it. */
    order == CCC_ORDER_GREATER ? (order = CCC_ORDER_LESSER)
                               : (order = CCC_ORDER_GREATER);
    heapify(buffer, temp, order, comparator);
    heapsort(buffer, temp, order, comparator);
    return CCC_RESULT_OK;
}

/*===================     Private Interface     =============================*/

size_t
CCC_private_flat_priority_queue_bubble_up(
    struct CCC_Flat_priority_queue *const priority_queue,
    void *const temp,
    size_t index
) {
    return bubble_up(
        &priority_queue->buffer,
        index,
        temp,
        priority_queue->order,
        &priority_queue->comparator
    );
}

void *
CCC_private_flat_priority_queue_update_fixup(
    struct CCC_Flat_priority_queue *const priority_queue,
    void *const type,
    void *const temp
) {
    return at(
        &priority_queue->buffer, update_fixup(priority_queue, type, temp)
    );
}

void
CCC_private_flat_priority_queue_heap_order(
    struct CCC_Flat_priority_queue *const priority_queue, void *const temp
) {
    heapify(
        &priority_queue->buffer,
        temp,
        priority_queue->order,
        &priority_queue->comparator
    );
}

/*====================     Static Helpers     ===============================*/

/* Orders the heap in O(N) time. Assumes n > 0 and n <= capacity. */
static inline void
heapify(
    CCC_Flat_buffer *const buffer,
    void *temp,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    size_t i = ((buffer->count - 1) / 2) + 1;
    while (i--) {
        (void)bubble_down(buffer, i, temp, order, comparator);
    }
}

static inline void
heapsort(
    CCC_Flat_buffer *const buffer,
    void *const temp,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    if (buffer->count > 1) {
        size_t const start = buffer->count;
        void *const root = at(buffer, 0);
        while (--buffer->count) {
            swap(buffer, temp, root, at(buffer, buffer->count));
            (void)bubble_down(buffer, 0, temp, order, comparator);
        }
        buffer->count = start;
    }
}

/* Fixes the position of element e after its key value has been changed. */
static size_t
update_fixup(
    struct CCC_Flat_priority_queue *const priority_queue,
    void *const type,
    void *const temp
) {
    size_t const index = index_of(priority_queue, type);
    if (!index) {
        return bubble_down(
            &priority_queue->buffer,
            0,
            temp,
            priority_queue->order,
            &priority_queue->comparator
        );
    }
    CCC_Order const parent_order
        = priority_queue->comparator.compare((CCC_Comparator_arguments){
            .type_left = at(&priority_queue->buffer, index),
            .type_right = at(&priority_queue->buffer, (index - 1) / 2),
            .context = priority_queue->comparator.context,
        });
    if (parent_order == priority_queue->order) {
        return bubble_up(
            &priority_queue->buffer,
            index,
            temp,
            priority_queue->order,
            &priority_queue->comparator
        );
    }
    if (parent_order != CCC_ORDER_EQUAL) {
        return bubble_down(
            &priority_queue->buffer,
            index,
            temp,
            priority_queue->order,
            &priority_queue->comparator
        );
    }
    return index;
}

/* Returns the sorted position of the element starting at position i. */
static inline size_t
bubble_up(
    CCC_Flat_buffer *const buffer,
    size_t index,
    void *const temp,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    for (size_t parent = (index - 1) / 2; index;
         index = parent, parent = (parent - 1) / 2) {
        void *const parent_pointer = at(buffer, parent);
        void *const this_pointer = at(buffer, index);
        /* Not winning here means we are in correct order or equal. */
        if (!wins(this_pointer, parent_pointer, order, comparator)) {
            return index;
        }
        swap(buffer, temp, this_pointer, parent_pointer);
    }
    return 0;
}

/* Returns the sorted position of the element starting at position i. */
static inline size_t
bubble_down(
    CCC_Flat_buffer *const buffer,
    size_t index,
    void *const temp,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    for (size_t next = 0, left = (index * 2) + 1, right = left + 1;
         left < buffer->count;
         index = next, left = (index * 2) + 1, right = left + 1) {
        void const *const left_pointer = at(buffer, left);
        next = left;
        if (right < buffer->count) {
            void const *const right_pointer = at(buffer, right);
            if (wins(right_pointer, left_pointer, order, comparator)) {
                next = right;
            }
        }
        void *const next_pointer = at(buffer, next);
        void *const this_pointer = at(buffer, index);
        /* If the child beats the parent we must swap. Equal is OK to break. */
        if (!wins(next_pointer, this_pointer, order, comparator)) {
            return index;
        }
        swap(buffer, temp, this_pointer, next_pointer);
    }
    return index;
}

/* Returns true if the winner (the "left hand side") wins the comparison.
   Winning in a three-way comparison means satisfying the total order of the
   priority queue. So, there is no winner if the elements are equal and this
   function would return false. If the winner is in the wrong order, thus
   losing the total order comparison, the function also returns false. */
static inline CCC_Tribool
wins(
    void const *const winner,
    void const *const loser,
    CCC_Order const order,
    CCC_Comparator const *const comparator
) {
    return comparator->compare((CCC_Comparator_arguments){
               .type_left = winner,
               .type_right = loser,
               .context = comparator->context,
           })
        == order;
}

/* Flat priority queue code that uses indices of the underlying Flat_buffer
   should always be within the Flat_buffer range. It should never exceed the
   current size and start at or after the Flat_buffer base. Only checked in
   debug. */
static inline size_t
index_of(
    struct CCC_Flat_priority_queue const *const priority_queue,
    void const *const slot
) {
    assert(slot >= priority_queue->buffer.data);
    size_t const i
        = (size_t)((char *)slot - (char *)priority_queue->buffer.data)
        / priority_queue->buffer.sizeof_type;
    assert(i < priority_queue->buffer.count);
    return i;
}

/** Swaps data in a and b according to buffer element size. Assumes a, b, and
temp are non-null. */
static inline void
swap(
    CCC_Flat_buffer *const buffer,
    void *const temp,
    void *const a,
    void *const b
) {
    assert(temp);
    assert(a);
    assert(b);
    (void)memcpy(temp, a, buffer->sizeof_type);
    (void)memcpy(a, b, buffer->sizeof_type);
    (void)memcpy(b, temp, buffer->sizeof_type);
}

/** Provides data at index. Assumes buffer is non-null and i is within
capacity. */
static inline void *
at(CCC_Flat_buffer const *const buffer, size_t const i) {
    assert(buffer);
    assert(i < buffer->capacity);
    return (char *)buffer->data + (i * buffer->sizeof_type);
}

static inline void
destroy_each(
    struct CCC_Flat_priority_queue *const priority_queue,
    CCC_Destructor const *const destructor
) {
    size_t const count = priority_queue->buffer.count;
    for (size_t i = 0; i < count; ++i) {
        destructor->destroy((CCC_Arguments){
            .type = at(&priority_queue->buffer, i),
            .context = destructor->context,
        });
    }
}
