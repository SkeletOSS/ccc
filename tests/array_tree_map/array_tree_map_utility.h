#ifndef CCC_HROMAP_UTIL_H
#define CCC_HROMAP_UTIL_H

#include <stddef.h>

#include "array_tree_map.h"
#include "checkers.h"
#include "types.h"

struct Val {
    int id;
    int val;
};

enum : size_t {
    SMALL_FIXED_CAP = 64,
    STANDARD_FIXED_CAP = 1024,
};

CCC_Order id_order(CCC_Key_comparator_arguments);

enum Check_result insert_shuffled(CCC_Array_tree_map *m, size_t size,
                                  int larger_prime);
size_t inorder_fill(int vals[], size_t size, CCC_Array_tree_map const *m);

#endif /* CCC_HROMAP_UTIL_H */
