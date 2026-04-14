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
/** @internal
@file
@brief The Adaptive Map Private Interface

The adaptive map is currently implemented as a Splay Tree. A Splay Tree is a
self-optimizing data structure that "adapts" the usage pattern of the user
by moving frequently accessed elements to the root. In the process, the trees
height is also reduced through rotations.

Adaptive, is the word used for this container because there are many
self-optimizing data structures that could take over this implementation. It is
best not to tie the naming to any one type of tree or data structure. */
#ifndef CCC_PRIVATE_ADAPTIVE_MAP_H
#define CCC_PRIVATE_ADAPTIVE_MAP_H

/** @cond */
#include <stddef.h>
/** @endcond */

#include "../types.h"

/* NOLINTBEGIN(readability-identifier-naming) */

/** @internal An ordered map element in a splay tree requires no special fields.
In fact the parent could be eliminated, but it is important in providing clean
iterative traversals with the begin, end, next abstraction for the user. */
struct CCC_Adaptive_map_node {
    /** @internal The child nodes in a array unite left and right cases. */
    struct CCC_Adaptive_map_node *branch[2];
    /** @internal The parent is useful for iteration. Not required for splay. */
    struct CCC_Adaptive_map_node *parent;
};

/** @internal Runs the top down splay tree algorithm over a node based tree. A
Splay Tree offers amortized `O(log(N))` because it is a self-optimizing
structure that operates on assumptions about usage patterns. Often, these
assumptions result in frequently accessed elements remaining a constant distance
from the root for O(1) access. However, anti-patterns can arise that harm
performance. The user should carefully consider if their data access pattern
can benefit from a skewed distribution before choosing this container. */
struct CCC_Adaptive_map {
    /** @internal The root of the splay tree. The "hot" node after a query. */
    struct CCC_Adaptive_map_node *root;
    /** @internal The number of stored tree nodes. */
    size_t count;
    /** @internal The size of the user type stored in the tree. */
    size_t sizeof_type;
    /** @internal The byte offset of the intrusive element. */
    size_t type_intruder_offset;
    /** @internal The byte offset of the user key in the user type. */
    size_t key_offset;
    /** @internal The user defined comparison callback and context. */
    CCC_Key_comparator comparator;
};

/** @internal An entry is a way to store a node or the information needed to
insert a node without a second query. The user can then take different actions
depending on the Occupied or Vacant status of the entry.

Unlike all the other data structure that offer the entry abstraction the ordered
map does not need to store any special information for a more efficient second
query. The element, or its closest match, is splayed to the root upon each
query. If the user proceeds to insert a new element a second query will result
in a constant time operation to make the new element the new root. If
intervening operations take place between obtaining an entry and inserting the
new element, the best fit will still be close to the root and splaying it again
and then inserting this new element will not be too expensive. Intervening
operations unrelated this entry would also be considered an anti pattern of the
Entry API. */
struct CCC_Adaptive_map_entry {
    /** @internal The tree associated with this query. */
    struct CCC_Adaptive_map *map;
    /** @internal The stored node or empty if not found. */
    CCC_Entry entry;
};

/*==========================  Private Interface  ============================*/

/** @internal */
void *CCC_private_adaptive_map_key_in_slot(
    struct CCC_Adaptive_map const *, void const *
);
/** @internal */
struct CCC_Adaptive_map_node *CCC_private_adaptive_map_node_in_slot(
    struct CCC_Adaptive_map const *, void const *
);
/** @internal */
struct CCC_Adaptive_map_entry
CCC_private_adaptive_map_entry(struct CCC_Adaptive_map *, void const *);
/** @internal */
void *CCC_private_adaptive_map_insert(
    struct CCC_Adaptive_map *, struct CCC_Adaptive_map_node *
);

/*======================   Macro Implementations     ========================*/

/** @internal All other fields default to NULL or 0. */
#define CCC_private_adaptive_map_default(                                      \
    private_struct_name,                                                       \
    private_node_node_field,                                                   \
    private_key_node_field,                                                    \
    private_comparator...                                                      \
)                                                                              \
    (struct CCC_Adaptive_map) {                                                \
        .sizeof_type = sizeof(private_struct_name),                            \
        .type_intruder_offset                                                  \
            = offsetof(private_struct_name, private_node_node_field),          \
        .key_offset = offsetof(private_struct_name, private_key_node_field),   \
        .comparator = private_comparator,                                      \
    }

/** @internal */
#define CCC_private_adaptive_map_for(                                          \
    private_struct_name,                                                       \
    private_node_node_field,                                                   \
    private_key_node_field,                                                    \
    private_comparator...                                                      \
)                                                                              \
    (struct CCC_Adaptive_map) {                                                \
        .root = NULL, .count = 0, .sizeof_type = sizeof(private_struct_name),  \
        .type_intruder_offset                                                  \
            = offsetof(private_struct_name, private_node_node_field),          \
        .key_offset = offsetof(private_struct_name, private_key_node_field),   \
        .comparator = private_comparator,                                      \
    }

/** @internal */
#define CCC_private_adaptive_map_from(                                         \
    private_type_intruder_field_name,                                          \
    private_key_field_name,                                                    \
    private_comparator,                                                        \
    private_allocator,                                                         \
    private_destructor,                                                        \
    private_compound_literal_array...                                          \
)                                                                              \
    (struct { struct CCC_Adaptive_map private; }){(__extension__({             \
        typeof(*private_compound_literal_array)                                \
            *private_adaptive_map_type_array = private_compound_literal_array; \
        struct CCC_Adaptive_map private_map                                    \
            = CCC_private_adaptive_map_default(                                \
                typeof(*private_adaptive_map_type_array),                      \
                private_type_intruder_field_name,                              \
                private_key_field_name,                                        \
                private_comparator                                             \
            );                                                                 \
        CCC_Allocator const private_adaptive_map_allocator                     \
            = private_allocator;                                               \
        if (private_adaptive_map_allocator.allocate) {                         \
            size_t const private_count                                         \
                = sizeof(private_compound_literal_array)                       \
                / sizeof(*private_adaptive_map_type_array);                    \
            for (size_t private_i = 0; private_i < private_count;              \
                 ++private_i) {                                                \
                struct CCC_Adaptive_map_entry private_adaptive_map_entry       \
                    = CCC_private_adaptive_map_entry(                          \
                        &private_map,                                          \
                        (void *)&private_adaptive_map_type_array[private_i]    \
                            .private_key_field_name                            \
                    );                                                         \
                if (!(private_adaptive_map_entry.entry.status                  \
                      & CCC_ENTRY_OCCUPIED)) {                                 \
                    typeof(*private_adaptive_map_type_array) *const            \
                        private_new_slot                                       \
                        = private_adaptive_map_allocator.allocate((            \
                            CCC_Allocator_arguments                            \
                        ){                                                     \
                            .input = NULL,                                     \
                            .bytes = private_map.sizeof_type,                  \
                            .context = private_adaptive_map_allocator.context, \
                        });                                                    \
                    if (!private_new_slot) {                                   \
                        (void)CCC_adaptive_map_clear(                          \
                            &private_map,                                      \
                            &(private_destructor),                             \
                            &private_adaptive_map_allocator                    \
                        );                                                     \
                        break;                                                 \
                    }                                                          \
                    *private_new_slot                                          \
                        = private_adaptive_map_type_array[private_i];          \
                    CCC_private_adaptive_map_insert(                           \
                        &private_map,                                          \
                        CCC_private_adaptive_map_node_in_slot(                 \
                            &private_map, private_new_slot                     \
                        )                                                      \
                    );                                                         \
                } else {                                                       \
                    struct CCC_Adaptive_map_node private_node_saved            \
                        = *CCC_private_adaptive_map_node_in_slot(              \
                            &private_map,                                      \
                            private_adaptive_map_entry.entry.type              \
                        );                                                     \
                    *((typeof(*private_adaptive_map_type_array) *)             \
                          private_adaptive_map_entry.entry.type)               \
                        = private_adaptive_map_type_array[private_i];          \
                    *CCC_private_adaptive_map_node_in_slot(                    \
                        &private_map, private_adaptive_map_entry.entry.type    \
                    ) = private_node_saved;                                    \
                }                                                              \
            }                                                                  \
        }                                                                      \
        private_map;                                                           \
    }))}.private

/** @internal */
#define CCC_private_adaptive_map_new(adaptive_map_entry, private_allocator)    \
    (__extension__({                                                           \
        void *private_adaptive_map_ins_allocate_ret = NULL;                    \
        if ((private_allocator)->allocate) {                                   \
            private_adaptive_map_ins_allocate_ret                              \
                = (private_allocator)                                          \
                      ->allocate((CCC_Allocator_arguments){                    \
                          .input = NULL,                                       \
                          .bytes = (adaptive_map_entry)->map->sizeof_type,     \
                          .context = (private_allocator)->context,             \
                      });                                                      \
        }                                                                      \
        private_adaptive_map_ins_allocate_ret;                                 \
    }))

/** @internal */
#define CCC_private_adaptive_map_insert_key_val(                               \
    adaptive_map_entry, new_data, type_compound_literal...                     \
)                                                                              \
    (__extension__({                                                           \
        if (new_data) {                                                        \
            *new_data = type_compound_literal;                                 \
            new_data = CCC_private_adaptive_map_insert(                        \
                (adaptive_map_entry)->map,                                     \
                CCC_private_adaptive_map_node_in_slot(                         \
                    (adaptive_map_entry)->map, new_data                        \
                )                                                              \
            );                                                                 \
        }                                                                      \
    }))

/** @internal */
#define CCC_private_adaptive_map_insert_and_copy_key(                          \
    om_insert_entry,                                                           \
    om_insert_entry_ret,                                                       \
    key,                                                                       \
    private_allocator,                                                         \
    type_compound_literal...                                                   \
)                                                                              \
    (__extension__({                                                           \
        typeof(type_compound_literal) *private_adaptive_map_new_ins_base       \
            = CCC_private_adaptive_map_new(                                    \
                (&om_insert_entry), private_allocator                          \
            );                                                                 \
        om_insert_entry_ret = (CCC_Entry){                                     \
            .type = private_adaptive_map_new_ins_base,                         \
            .status = CCC_ENTRY_INSERT_ERROR,                                  \
        };                                                                     \
        if (private_adaptive_map_new_ins_base) {                               \
            om_insert_entry_ret.status = CCC_ENTRY_VACANT;                     \
            *((typeof(type_compound_literal) *)                                \
                  private_adaptive_map_new_ins_base) = type_compound_literal;  \
            *((typeof(key) *)CCC_private_adaptive_map_key_in_slot(             \
                om_insert_entry.map, private_adaptive_map_new_ins_base         \
            )) = key;                                                          \
            (void)CCC_private_adaptive_map_insert(                             \
                om_insert_entry.map,                                           \
                CCC_private_adaptive_map_node_in_slot(                         \
                    om_insert_entry.map, private_adaptive_map_new_ins_base     \
                )                                                              \
            );                                                                 \
        }                                                                      \
    }))

/*=====================     Core Macro Implementations     ==================*/

/** @internal */
#define CCC_private_adaptive_map_and_modify_with(                              \
    adaptive_map_entry_pointer,                                                \
    closure_parameter,                                                         \
    closure_over_closure_parameter...                                          \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Adaptive_map_entry const *const                             \
            private_adaptive_map_ent_pointer = (adaptive_map_entry_pointer);   \
        struct CCC_Adaptive_map_entry private_adaptive_map_mod_ent             \
            = {.entry = {.status = CCC_ENTRY_ARGUMENT_ERROR}};                 \
        if (private_adaptive_map_ent_pointer) {                                \
            private_adaptive_map_mod_ent = *private_adaptive_map_ent_pointer;  \
            if (private_adaptive_map_mod_ent.entry.status                      \
                & CCC_ENTRY_OCCUPIED) {                                        \
                closure_parameter = private_adaptive_map_mod_ent.entry.type;   \
                closure_over_closure_parameter                                 \
            }                                                                  \
        }                                                                      \
        private_adaptive_map_mod_ent;                                          \
    }))

/** @internal */
#define CCC_private_adaptive_map_or_insert_with(                               \
    adaptive_map_entry_pointer,                                                \
    private_allocator_pointer,                                                 \
    type_compound_literal...                                                   \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Adaptive_map_entry const *const                             \
            private_or_ins_entry_pointer = (adaptive_map_entry_pointer);       \
        typeof(type_compound_literal) *private_or_ins_ret = NULL;              \
        CCC_Allocator const *const private_adaptive_map_allocator              \
            = (private_allocator_pointer);                                     \
        if (private_adaptive_map_allocator && private_or_ins_entry_pointer) {  \
            if (private_or_ins_entry_pointer->entry.status                     \
                == CCC_ENTRY_OCCUPIED) {                                       \
                private_or_ins_ret = private_or_ins_entry_pointer->entry.type; \
            } else {                                                           \
                private_or_ins_ret = CCC_private_adaptive_map_new(             \
                    private_or_ins_entry_pointer,                              \
                    private_adaptive_map_allocator                             \
                );                                                             \
                CCC_private_adaptive_map_insert_key_val(                       \
                    private_or_ins_entry_pointer,                              \
                    private_or_ins_ret,                                        \
                    type_compound_literal                                      \
                );                                                             \
            }                                                                  \
        }                                                                      \
        private_or_ins_ret;                                                    \
    }))

/** @internal */
#define CCC_private_adaptive_map_insert_entry_with(                            \
    adaptive_map_entry_pointer,                                                \
    private_allocator_pointer,                                                 \
    type_compound_literal...                                                   \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Adaptive_map_entry const *const private_ins_entry_pointer   \
            = (adaptive_map_entry_pointer);                                    \
        typeof(type_compound_literal) *private_adaptive_map_ins_ent_ret        \
            = NULL;                                                            \
        CCC_Allocator const *const private_adaptive_map_allocator              \
            = (private_allocator_pointer);                                     \
        if (private_adaptive_map_allocator && private_ins_entry_pointer) {     \
            if (!(private_ins_entry_pointer->entry.status                      \
                  & CCC_ENTRY_OCCUPIED)) {                                     \
                private_adaptive_map_ins_ent_ret                               \
                    = CCC_private_adaptive_map_new(                            \
                        private_ins_entry_pointer,                             \
                        private_adaptive_map_allocator                         \
                    );                                                         \
                CCC_private_adaptive_map_insert_key_val(                       \
                    private_ins_entry_pointer,                                 \
                    private_adaptive_map_ins_ent_ret,                          \
                    type_compound_literal                                      \
                );                                                             \
            } else if (private_ins_entry_pointer->entry.status                 \
                       == CCC_ENTRY_OCCUPIED) {                                \
                struct CCC_Adaptive_map_node private_ins_ent_saved             \
                    = *CCC_private_adaptive_map_node_in_slot(                  \
                        private_ins_entry_pointer->map,                        \
                        private_ins_entry_pointer->entry.type                  \
                    );                                                         \
                *((typeof(type_compound_literal) *)                            \
                      private_ins_entry_pointer->entry.type)                   \
                    = type_compound_literal;                                   \
                *CCC_private_adaptive_map_node_in_slot(                        \
                    private_ins_entry_pointer->map,                            \
                    private_ins_entry_pointer->entry.type                      \
                ) = private_ins_ent_saved;                                     \
                private_adaptive_map_ins_ent_ret                               \
                    = private_ins_entry_pointer->entry.type;                   \
            }                                                                  \
        }                                                                      \
        private_adaptive_map_ins_ent_ret;                                      \
    }))

/** @internal */
#define CCC_private_adaptive_map_try_insert_with(                              \
    adaptive_map_pointer,                                                      \
    key,                                                                       \
    private_allocator_pointer,                                                 \
    type_compound_literal...                                                   \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Adaptive_map *const private_try_ins_map_pointer             \
            = (adaptive_map_pointer);                                          \
        CCC_Entry private_adaptive_map_try_ins_ent_ret                         \
            = {.status = CCC_ENTRY_ARGUMENT_ERROR};                            \
        CCC_Allocator const *const private_adaptive_map_allocator              \
            = (private_allocator_pointer);                                     \
        if (private_adaptive_map_allocator && private_try_ins_map_pointer) {   \
            __auto_type private_adaptive_map_key = (key);                      \
            struct CCC_Adaptive_map_entry private_adaptive_map_try_ins_ent     \
                = CCC_private_adaptive_map_entry(                              \
                    private_try_ins_map_pointer,                               \
                    (void *)&private_adaptive_map_key                          \
                );                                                             \
            if (!(private_adaptive_map_try_ins_ent.entry.status                \
                  & CCC_ENTRY_OCCUPIED)) {                                     \
                CCC_private_adaptive_map_insert_and_copy_key(                  \
                    private_adaptive_map_try_ins_ent,                          \
                    private_adaptive_map_try_ins_ent_ret,                      \
                    private_adaptive_map_key,                                  \
                    private_adaptive_map_allocator,                            \
                    type_compound_literal                                      \
                );                                                             \
            } else if (private_adaptive_map_try_ins_ent.entry.status           \
                       == CCC_ENTRY_OCCUPIED) {                                \
                private_adaptive_map_try_ins_ent_ret                           \
                    = private_adaptive_map_try_ins_ent.entry;                  \
            }                                                                  \
        }                                                                      \
        private_adaptive_map_try_ins_ent_ret;                                  \
    }))

/** @internal */
#define CCC_private_adaptive_map_insert_or_assign_with(                        \
    adaptive_map_pointer,                                                      \
    key,                                                                       \
    private_allocator_pointer,                                                 \
    type_compound_literal...                                                   \
)                                                                              \
    (__extension__({                                                           \
        struct CCC_Adaptive_map *const private_ins_or_assign_map_pointer       \
            = (adaptive_map_pointer);                                          \
        CCC_Entry private_adaptive_map_ins_or_assign_ent_ret                   \
            = {.status = CCC_ENTRY_ARGUMENT_ERROR};                            \
        CCC_Allocator const *const private_adaptive_map_allocator              \
            = (private_allocator_pointer);                                     \
        if (private_adaptive_map_allocator                                     \
            && private_ins_or_assign_map_pointer) {                            \
            __auto_type private_adaptive_map_key = (key);                      \
            struct CCC_Adaptive_map_entry                                      \
                private_adaptive_map_ins_or_assign_ent                         \
                = CCC_private_adaptive_map_entry(                              \
                    private_ins_or_assign_map_pointer,                         \
                    (void *)&private_adaptive_map_key                          \
                );                                                             \
            if (!(private_adaptive_map_ins_or_assign_ent.entry.status          \
                  & CCC_ENTRY_OCCUPIED)) {                                     \
                CCC_private_adaptive_map_insert_and_copy_key(                  \
                    private_adaptive_map_ins_or_assign_ent,                    \
                    private_adaptive_map_ins_or_assign_ent_ret,                \
                    private_adaptive_map_key,                                  \
                    private_adaptive_map_allocator,                            \
                    type_compound_literal                                      \
                );                                                             \
            } else if (private_adaptive_map_ins_or_assign_ent.entry.status     \
                       == CCC_ENTRY_OCCUPIED) {                                \
                struct CCC_Adaptive_map_node private_ins_ent_saved             \
                    = *CCC_private_adaptive_map_node_in_slot(                  \
                        private_adaptive_map_ins_or_assign_ent.map,            \
                        private_adaptive_map_ins_or_assign_ent.entry.type      \
                    );                                                         \
                *((typeof(type_compound_literal) *)                            \
                      private_adaptive_map_ins_or_assign_ent.entry.type)       \
                    = type_compound_literal;                                   \
                *CCC_private_adaptive_map_node_in_slot(                        \
                    private_adaptive_map_ins_or_assign_ent.map,                \
                    private_adaptive_map_ins_or_assign_ent.entry.type          \
                ) = private_ins_ent_saved;                                     \
                private_adaptive_map_ins_or_assign_ent_ret                     \
                    = private_adaptive_map_ins_or_assign_ent.entry;            \
                *((typeof(private_adaptive_map_key) *)                         \
                      CCC_private_adaptive_map_key_in_slot(                    \
                          private_ins_or_assign_map_pointer,                   \
                          private_adaptive_map_ins_or_assign_ent_ret.type      \
                      )) = private_adaptive_map_key;                           \
            }                                                                  \
        }                                                                      \
        private_adaptive_map_ins_or_assign_ent_ret;                            \
    }))

/* NOLINTEND(readability-identifier-naming) */

#endif /* CCC_PRIVATE_ADAPTIVE_MAP_H */
