#ifndef CCC_FLAT_HASH_MAP_UTIL_H
#define CCC_FLAT_HASH_MAP_UTIL_H

#include <stddef.h>
#include <stdint.h>

#include "types.h"

struct Val {
    int key;
    int val;
};

enum : size_t {
    SMALL_FIXED_CAP = 64,
    STANDARD_FIXED_CAP = 1024,
};

uint64_t flat_hash_map_int_zero(CCC_Key_arguments);
uint64_t flat_hash_map_int_last_digit(CCC_Key_arguments);
uint64_t flat_hash_map_int_to_u64(CCC_Key_arguments);
CCC_Order flat_hash_map_id_order(CCC_Key_comparator_arguments);

void flat_hash_map_modplus(CCC_Type_arguments);
struct Val flat_hash_map_create(int id, int val);
void flat_hash_map_swap_val(CCC_Type_arguments u);

#endif /* CCC_FLAT_HASH_MAP_UTIL_H */
