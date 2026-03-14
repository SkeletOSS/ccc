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
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "buffer.h"
#include "flat_priority_queue.h"
#include "private/private_flat_priority_queue.h"
#include "sort.h"
#include "types.h"

enum : size_t {
    START_CAP = 8,
};

/*=====================      Prototypes      ================================*/

static size_t index_of(struct CCC_Flat_priority_queue const *priority_queue,
                       void const *slot);
static CCC_Tribool wins(CCC_Order order, CCC_Type_comparator *compare,
                        void *context, void const *winner, void const *loser);
static size_t bubble_up(CCC_Buffer *buffer, CCC_Order order,
                        CCC_Type_comparator *compare, void *context,
                        size_t index, void *temp);
static size_t bubble_down(CCC_Buffer *buffer, CCC_Order order,
                          CCC_Type_comparator *compare, void *context,
                          size_t index, void *temp);
static size_t update_fixup(struct CCC_Flat_priority_queue *, void *, void *);
static void heapify(CCC_Buffer *buffer, CCC_Order order,
                    CCC_Type_comparator *compare, void *context, void *temp);
static void heapsort(CCC_Buffer *buffer, CCC_Order order,
                     CCC_Type_comparator *compare, void *context, void *temp);
static void destroy_each(struct CCC_Flat_priority_queue *,
                         CCC_Type_destructor *);

/*=====================       Interface      ================================*/

CCC_Result
CCC_flat_priority_queue_copy_heapify(
    CCC_Flat_priority_queue *const priority_queue,
    CCC_Buffer const *const buffer, void *const temp) {
    if (!priority_queue || !temp) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    CCC_Result const copy_result = CCC_buffer_copy(
        &priority_queue->buffer, buffer, priority_queue->buffer.allocate);
    if (copy_result != CCC_RESULT_OK) {
        return copy_result;
    }
    heapify(&priority_queue->buffer, priority_queue->order,
            priority_queue->compare, priority_queue->buffer.context, temp);
    return CCC_RESULT_OK;
}

CCC_Flat_priority_queue
CCC_flat_priority_queue_in_place_heapify(CCC_Buffer *const buffer,
                                         CCC_Order const order,
                                         CCC_Type_comparator *const compare,
                                         void *const temp) {
    if (!buffer || !temp || !compare
        || (order != CCC_ORDER_GREATER && order != CCC_ORDER_LESSER)) {
        return (CCC_Flat_priority_queue){
            .order = CCC_ORDER_ERROR,
        };
    }
    CCC_Flat_priority_queue priority_queue = {
        .buffer = *buffer,
        .compare = compare,
        .order = order,
    };
    heapify(&priority_queue.buffer, priority_queue.order,
            priority_queue.compare, NULL, temp);
    *buffer = (CCC_Buffer){};
    return priority_queue;
}

CCC_Flat_priority_queue
CCC_flat_priority_queue_context_in_place_heapify(
    CCC_Buffer *const buffer, CCC_Order const order,
    CCC_Type_comparator *const compare, void *const context, void *const temp) {
    if (!buffer || !temp || !compare
        || (order != CCC_ORDER_GREATER && order != CCC_ORDER_LESSER)) {
        return (CCC_Flat_priority_queue){
            .order = CCC_ORDER_ERROR,
        };
    }
    CCC_Flat_priority_queue priority_queue = {
        .buffer = *buffer,
        .compare = compare,
        .order = order,
    };
    heapify(&priority_queue.buffer, priority_queue.order,
            priority_queue.compare, context, temp);
    *buffer = (CCC_Buffer){};
    return priority_queue;
}

void *
CCC_flat_priority_queue_push(CCC_Flat_priority_queue *const priority_queue,
                             void const *const type, void *const temp) {
    if (!priority_queue || !type || !temp) {
        return NULL;
    }
    void *const new = CCC_buffer_allocate_back(&priority_queue->buffer);
    if (!new) {
        return NULL;
    }
    if (new != type) {
        (void)memcpy(new, type, priority_queue->buffer.sizeof_type);
    }
    assert(temp);
    size_t const i = bubble_up(
        &priority_queue->buffer, priority_queue->order, priority_queue->compare,
        priority_queue->buffer.context, priority_queue->buffer.count - 1, temp);
    assert(i < priority_queue->buffer.count);
    return CCC_buffer_at(&priority_queue->buffer, i);
}

CCC_Result
CCC_flat_priority_queue_pop(CCC_Flat_priority_queue *const priority_queue,
                            void *const temp) {
    if (!priority_queue || !temp || !priority_queue->buffer.count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    --priority_queue->buffer.count;
    if (!priority_queue->buffer.count) {
        return CCC_RESULT_OK;
    }
    CCC_buffer_swap(&priority_queue->buffer, temp, 0,
                    priority_queue->buffer.count);
    (void)bubble_down(&priority_queue->buffer, priority_queue->order,
                      priority_queue->compare, priority_queue->buffer.context,
                      0, temp);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_priority_queue_erase(CCC_Flat_priority_queue *const priority_queue,
                              void *const type, void *const temp) {
    if (!priority_queue || !type || !temp || !priority_queue->buffer.count) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    size_t const i = index_of(priority_queue, type);
    --priority_queue->buffer.count;
    if (i == priority_queue->buffer.count) {
        return CCC_RESULT_OK;
    }
    (void)CCC_buffer_swap(&priority_queue->buffer, temp, i,
                          priority_queue->buffer.count);
    CCC_Order const order_res
        = priority_queue->compare((CCC_Type_comparator_context){
            .type_left = CCC_buffer_at(&priority_queue->buffer, i),
            .type_right = CCC_buffer_at(&priority_queue->buffer,
                                        priority_queue->buffer.count),
            .context = priority_queue->buffer.context,
        });
    if (order_res == priority_queue->order) {
        (void)bubble_up(&priority_queue->buffer, priority_queue->order,
                        priority_queue->compare, priority_queue->buffer.context,
                        i, temp);
    } else if (order_res != CCC_ORDER_EQUAL) {
        (void)bubble_down(&priority_queue->buffer, priority_queue->order,
                          priority_queue->compare,
                          priority_queue->buffer.context, i, temp);
    }
    /* If the comparison is equal do nothing. Element is in right spot. */
    return CCC_RESULT_OK;
}

void *
CCC_flat_priority_queue_update(CCC_Flat_priority_queue *const priority_queue,
                               void *const type, void *const temp,
                               CCC_Type_modifier *const modify,
                               void *const context) {
    if (!priority_queue || !type || !temp || !modify
        || !priority_queue->buffer.count) {
        return NULL;
    }
    modify((CCC_Type_context){
        .type = type,
        .context = context,
    });
    return CCC_buffer_at(&priority_queue->buffer,
                         update_fixup(priority_queue, type, temp));
}

/* There are no efficiency benefits in knowing an increase will occur. */
void *
CCC_flat_priority_queue_increase(CCC_Flat_priority_queue *const priority_queue,
                                 void *const type, void *const temp,
                                 CCC_Type_modifier *const modify,
                                 void *const context) {
    return CCC_flat_priority_queue_update(priority_queue, type, temp, modify,
                                          context);
}

/* There are no efficiency benefits in knowing an decrease will occur. */
void *
CCC_flat_priority_queue_decrease(CCC_Flat_priority_queue *const priority_queue,
                                 void *const type, void *const temp,
                                 CCC_Type_modifier *const modify,
                                 void *const context) {
    return CCC_flat_priority_queue_update(priority_queue, type, temp, modify,
                                          context);
}

void *
CCC_flat_priority_queue_front(
    CCC_Flat_priority_queue const *const priority_queue) {
    if (!priority_queue || !priority_queue->buffer.count) {
        return NULL;
    }
    return CCC_buffer_at(&priority_queue->buffer, 0);
}

CCC_Tribool
CCC_flat_priority_queue_is_empty(
    CCC_Flat_priority_queue const *const priority_queue) {
    if (!priority_queue) {
        return CCC_TRIBOOL_ERROR;
    }
    return CCC_buffer_is_empty(&priority_queue->buffer);
}

CCC_Count
CCC_flat_priority_queue_count(
    CCC_Flat_priority_queue const *const priority_queue) {
    if (!priority_queue) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return CCC_buffer_count(&priority_queue->buffer);
}

CCC_Count
CCC_flat_priority_queue_capacity(
    CCC_Flat_priority_queue const *const priority_queue) {
    if (!priority_queue) {
        return (CCC_Count){.error = CCC_RESULT_ARGUMENT_ERROR};
    }
    return CCC_buffer_capacity(&priority_queue->buffer);
}

void *
CCC_flat_priority_queue_data(
    CCC_Flat_priority_queue const *const priority_queue) {
    return priority_queue ? CCC_buffer_begin(&priority_queue->buffer) : NULL;
}

CCC_Order
CCC_flat_priority_queue_order(
    CCC_Flat_priority_queue const *const priority_queue) {
    return priority_queue ? priority_queue->order : CCC_ORDER_ERROR;
}

CCC_Result
CCC_flat_priority_queue_reserve(CCC_Flat_priority_queue *const priority_queue,
                                size_t const to_add,
                                CCC_Allocator *const allocate) {
    if (!priority_queue) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    return CCC_buffer_reserve(&priority_queue->buffer, to_add, allocate);
}

CCC_Result
CCC_flat_priority_queue_copy(CCC_Flat_priority_queue *const destination,
                             CCC_Flat_priority_queue const *const source,
                             CCC_Allocator *const allocate) {
    if (!destination || !source || source == destination
        || (destination->buffer.capacity < source->buffer.capacity
            && !allocate)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (!source->buffer.count) {
        return CCC_RESULT_OK;
    }
    if (destination->buffer.capacity < source->buffer.capacity) {
        CCC_Result const r = CCC_buffer_allocate(
            &destination->buffer, source->buffer.capacity, allocate);
        if (r != CCC_RESULT_OK) {
            return r;
        }
        destination->buffer.capacity = source->buffer.capacity;
    }
    if (!source->buffer.data || !destination->buffer.data) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    destination->buffer.count = source->buffer.count;
    /* It is ok to only copy count elements because we know that all elements
       in a binary heap are contiguous from [0, C), where C is count. */
    (void)memcpy(destination->buffer.data, source->buffer.data,
                 source->buffer.count * source->buffer.sizeof_type);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_flat_priority_queue_clear(CCC_Flat_priority_queue *const priority_queue,
                              CCC_Type_destructor *const destroy) {
    if (!priority_queue) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (destroy) {
        destroy_each(priority_queue, destroy);
    }
    return CCC_buffer_size_set(&priority_queue->buffer, 0);
}

CCC_Result
CCC_flat_priority_queue_clear_and_free(
    CCC_Flat_priority_queue *const priority_queue,
    CCC_Type_destructor *const destroy) {
    if (!priority_queue) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (destroy) {
        destroy_each(priority_queue, destroy);
    }
    return CCC_buffer_allocate(&priority_queue->buffer, 0,
                               priority_queue->buffer.allocate);
}

CCC_Result
CCC_flat_priority_queue_clear_and_free_reserve(
    CCC_Flat_priority_queue *const priority_queue,
    CCC_Type_destructor *const destructor, CCC_Allocator *const allocate) {
    if (!priority_queue) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    if (destructor) {
        destroy_each(priority_queue, destructor);
    }
    return CCC_buffer_allocate(&priority_queue->buffer, 0, allocate);
}

CCC_Tribool
CCC_flat_priority_queue_validate(
    CCC_Flat_priority_queue const *const priority_queue) {
    if (!priority_queue) {
        return CCC_TRIBOOL_ERROR;
    }
    size_t const count = priority_queue->buffer.count;
    if (count <= 1) {
        return CCC_TRUE;
    }
    for (size_t i = 0, left = (i * 2) + 1, right = (i * 2) + 2,
                end = (count - 2) / 2;
         i <= end; ++i, left = (i * 2) + 1, right = (i * 2) + 2) {
        void const *const this_pointer
            = CCC_buffer_at(&priority_queue->buffer, i);
        /* Putting the child in the comparison function first evaluates
           the child's three way comparison in relation to the parent. If
           the child beats the parent in total ordering (min/max) something
           has gone wrong. */
        if (left < count
            && wins(priority_queue->order, priority_queue->compare,
                    priority_queue->buffer.context,
                    CCC_buffer_at(&priority_queue->buffer, left),
                    this_pointer)) {
            return CCC_FALSE;
        }
        if (right < count
            && wins(priority_queue->order, priority_queue->compare,
                    priority_queue->buffer.context,
                    CCC_buffer_at(&priority_queue->buffer, right),
                    this_pointer)) {
            return CCC_FALSE;
        }
    }
    return CCC_TRUE;
}

/*===================     Interface in sort.h   =============================*/

CCC_Result
CCC_sort_heapsort(CCC_Buffer *const buffer, CCC_Order order,
                  CCC_Type_comparator *const compare, void *const temp) {
    if (!buffer || !temp || !compare
        || (order != CCC_ORDER_GREATER && order != CCC_ORDER_LESSER)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    /* For sorting the user expects the buffer to be in the order they specify.
       Just like they would expect their input order to the priority queue to
       place the least or greatest element closest to the root. However,
       heap sort fills a buffer from back to front, so flip it. */
    order == CCC_ORDER_GREATER ? (order = CCC_ORDER_LESSER)
                               : (order = CCC_ORDER_GREATER);
    heapify(buffer, order, compare, NULL, temp);
    heapsort(buffer, order, compare, NULL, temp);
    return CCC_RESULT_OK;
}

CCC_Result
CCC_sort_context_heapsort(CCC_Buffer *const buffer, CCC_Order order,
                          CCC_Type_comparator *const compare,
                          void *const context, void *const temp) {
    if (!buffer || !temp || !compare
        || (order != CCC_ORDER_GREATER && order != CCC_ORDER_LESSER)) {
        return CCC_RESULT_ARGUMENT_ERROR;
    }
    /* For sorting the user expects the buffer to be in the order they specify.
       Just like they would expect their input order to the priority queue to
       place the least or greatest element closest to the root. However,
       heap sort fills a buffer from back to front, so flip it. */
    order == CCC_ORDER_GREATER ? (order = CCC_ORDER_LESSER)
                               : (order = CCC_ORDER_GREATER);
    heapify(buffer, order, compare, context, temp);
    heapsort(buffer, order, compare, context, temp);
    return CCC_RESULT_OK;
}

/*===================     Private Interface     =============================*/

size_t
CCC_private_flat_priority_queue_bubble_up(
    struct CCC_Flat_priority_queue *const priority_queue, void *const temp,
    size_t index) {
    return bubble_up(&priority_queue->buffer, priority_queue->order,
                     priority_queue->compare, priority_queue->buffer.context,
                     index, temp);
}

void *
CCC_private_flat_priority_queue_update_fixup(
    struct CCC_Flat_priority_queue *const priority_queue, void *const type,
    void *const temp) {
    return CCC_buffer_at(&priority_queue->buffer,
                         update_fixup(priority_queue, type, temp));
}

void
CCC_private_flat_priority_queue_heap_order(
    struct CCC_Flat_priority_queue *const priority_queue, void *const temp) {
    if (!priority_queue) {
        return;
    }
    heapify(&priority_queue->buffer, priority_queue->order,
            priority_queue->compare, priority_queue->buffer.context, temp);
}

/*====================     Static Helpers     ===============================*/

/* Orders the heap in O(N) time. Assumes n > 0 and n <= capacity. */
static inline void
heapify(CCC_Buffer *const buffer, CCC_Order const order,
        CCC_Type_comparator *const compare, void *const context,
        void *const temp) {
    size_t i = ((buffer->count - 1) / 2) + 1;
    while (i--) {
        (void)bubble_down(buffer, order, compare, context, i, temp);
    }
}

static inline void
heapsort(CCC_Buffer *const buffer, CCC_Order const order,
         CCC_Type_comparator *const compare, void *const context,
         void *const temp) {
    if (buffer->count > 1) {
        size_t const start = buffer->count;
        while (--buffer->count) {
            CCC_buffer_swap(buffer, temp, 0, buffer->count);
            (void)bubble_down(buffer, order, compare, context, 0, temp);
        }
        buffer->count = start;
    }
}

/* Fixes the position of element e after its key value has been changed. */
static size_t
update_fixup(struct CCC_Flat_priority_queue *const priority_queue,
             void *const type, void *const temp) {
    size_t const index = index_of(priority_queue, type);
    if (!index) {
        return bubble_down(&priority_queue->buffer, priority_queue->order,
                           priority_queue->compare,
                           priority_queue->buffer.context, 0, temp);
    }
    CCC_Order const parent_order = priority_queue->compare((
        CCC_Type_comparator_context){
        .type_left = CCC_buffer_at(&priority_queue->buffer, index),
        .type_right = CCC_buffer_at(&priority_queue->buffer, (index - 1) / 2),
        .context = priority_queue->buffer.context,
    });
    if (parent_order == priority_queue->order) {
        return bubble_up(&priority_queue->buffer, priority_queue->order,
                         priority_queue->compare,
                         priority_queue->buffer.context, index, temp);
    }
    if (parent_order != CCC_ORDER_EQUAL) {
        return bubble_down(&priority_queue->buffer, priority_queue->order,
                           priority_queue->compare,
                           priority_queue->buffer.context, index, temp);
    }
    /* If the comparison is equal do nothing. Element is in right spot. */
    return index;
}

/* Returns the sorted position of the element starting at position i. */
static inline size_t
bubble_up(CCC_Buffer *const buffer, CCC_Order const order,
          CCC_Type_comparator *const compare, void *const context, size_t index,
          void *const temp) {
    for (size_t parent = (index - 1) / 2; index;
         index = parent, parent = (parent - 1) / 2) {
        void const *const parent_pointer = CCC_buffer_at(buffer, parent);
        void const *const this_pointer = CCC_buffer_at(buffer, index);
        /* Not winning here means we are in correct order or equal. */
        if (!wins(order, compare, context, this_pointer, parent_pointer)) {
            return index;
        }
        (void)CCC_buffer_swap(buffer, temp, index, parent);
    }
    return 0;
}

/* Returns the sorted position of the element starting at position i. */
static inline size_t
bubble_down(CCC_Buffer *const buffer, CCC_Order const order,
            CCC_Type_comparator *const compare, void *const context,
            size_t index, void *const temp) {
    for (size_t next = 0, left = (index * 2) + 1, right = left + 1;
         left < buffer->count;
         index = next, left = (index * 2) + 1, right = left + 1) {
        void const *const left_pointer = CCC_buffer_at(buffer, left);
        next = left;
        if (right < buffer->count) {
            void const *const right_pointer = CCC_buffer_at(buffer, right);
            if (wins(order, compare, context, right_pointer, left_pointer)) {
                next = right;
            }
        }
        void const *const next_pointer = CCC_buffer_at(buffer, next);
        void const *const this_pointer = CCC_buffer_at(buffer, index);
        /* If the child beats the parent we must swap. Equal is OK to break. */
        if (!wins(order, compare, context, next_pointer, this_pointer)) {
            return index;
        }
        (void)CCC_buffer_swap(buffer, temp, index, next);
    }
    return index;
}

/* Returns true if the winner (the "left hand side") wins the comparison.
   Winning in a three-way comparison means satisfying the total order of the
   priority queue. So, there is no winner if the elements are equal and this
   function would return false. If the winner is in the wrong order, thus
   losing the total order comparison, the function also returns false. */
static inline CCC_Tribool
wins(CCC_Order const order, CCC_Type_comparator *const compare,
     void *const context, void const *const winner, void const *const loser) {
    return compare((CCC_Type_comparator_context){
               .type_left = winner,
               .type_right = loser,
               .context = context,
           })
        == order;
}

/* Flat priority queue code that uses indices of the underlying Buffer should
   always be within the Buffer range. It should never exceed the current size
   and start at or after the Buffer base. Only checked in debug. */
static inline size_t
index_of(struct CCC_Flat_priority_queue const *const priority_queue,
         void const *const slot) {
    assert(slot >= priority_queue->buffer.data);
    size_t const i = ((char *)slot - (char *)priority_queue->buffer.data)
                   / priority_queue->buffer.sizeof_type;
    assert(i < priority_queue->buffer.count);
    return i;
}

static inline void
destroy_each(struct CCC_Flat_priority_queue *const priority_queue,
             CCC_Type_destructor *const destroy) {
    size_t const count = priority_queue->buffer.count;
    for (size_t i = 0; i < count; ++i) {
        destroy((CCC_Type_context){
            .type = CCC_buffer_at(&priority_queue->buffer, i),
            .context = priority_queue->buffer.context,
        });
    }
}
