#include <stdbool.h>
#include <stddef.h>

#include "ccc/flat_bitset.h"
#include "ccc/types.h"
#include "checkers.h"
#include "utility/allocate.h"

check_static_begin(flat_bitset_test_push_pop_back_no_reallocate) {
    CCC_Flat_bitset bs = CCC_flat_bitset_for(
        16, 0, CCC_flat_bitset_storage_for((CCC_Bit[16]){})
    );
    check(CCC_flat_bitset_capacity(&bs).count, 16);
    check(CCC_flat_bitset_count(&bs).count, 0);
    CCC_Result push_status = CCC_RESULT_OK;
    for (size_t i = 0; push_status == CCC_RESULT_OK; ++i) {
        if (i % 2) {
            push_status
                = CCC_flat_bitset_push_back(&bs, CCC_TRUE, &(CCC_Allocator){});
        } else {
            push_status
                = CCC_flat_bitset_push_back(&bs, CCC_FALSE, &(CCC_Allocator){});
        }
    }
    check(push_status, CCC_RESULT_NO_ALLOCATION_FUNCTION);
    check(CCC_flat_bitset_count(&bs).count, 16);
    check(CCC_flat_bitset_popcount(&bs).count, 16 / 2);
    while (!CCC_flat_bitset_is_empty(&bs)) {
        CCC_Tribool const msb = CCC_flat_bitset_pop_back(&bs);
        if (CCC_flat_bitset_count(&bs).count % 2) {
            check(msb, CCC_TRUE);
        } else {
            check(msb, CCC_FALSE);
        }
    }
    check(CCC_flat_bitset_count(&bs).count, 0);
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    check(CCC_flat_bitset_capacity(&bs).count, 16);
    check(CCC_flat_bitset_clear(NULL), CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_flat_bitset_clear(&bs), CCC_RESULT_OK);
    check(CCC_flat_bitset_capacity(&bs).count, 16);
    check(
        CCC_flat_bitset_clear_and_free(NULL, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(CCC_flat_bitset_clear_and_free(&bs, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check(
        CCC_flat_bitset_clear_and_free(&bs, &(CCC_Allocator){}),
        CCC_RESULT_NO_ALLOCATION_FUNCTION
    );
    check(CCC_flat_bitset_capacity(&bs).count, 16);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check_end();
}

check_static_begin(flat_bitset_test_push_pop_back_allocate) {
    CCC_Flat_bitset bs = CCC_flat_bitset_default();
    check(CCC_flat_bitset_capacity(&bs).count, 0);
    check(CCC_flat_bitset_count(&bs).count, 0);
    for (size_t i = 0; CCC_flat_bitset_count(&bs).count < 16; ++i) {
        if (i % 2) {
            check(
                CCC_flat_bitset_push_back(&bs, CCC_TRUE, &std_allocator),
                CCC_RESULT_OK
            );
        } else {
            check(
                CCC_flat_bitset_push_back(&bs, CCC_FALSE, &std_allocator),
                CCC_RESULT_OK
            );
        }
    }
    check(CCC_flat_bitset_count(&bs).count, 16);
    check(CCC_flat_bitset_popcount(&bs).count, 16 / 2);
    while (!CCC_flat_bitset_is_empty(&bs)) {
        CCC_Tribool const msb_was = CCC_flat_bitset_pop_back(&bs);
        if (CCC_flat_bitset_count(&bs).count % 2) {
            check(msb_was, CCC_TRUE);
        } else {
            check(msb_was, CCC_FALSE);
        }
    }
    check(CCC_flat_bitset_pop_back(&bs), (CCC_Tribool)CCC_TRIBOOL_ERROR);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    check(CCC_flat_bitset_capacity(&bs).count != 0, true);
    check(CCC_flat_bitset_clear(&bs), CCC_RESULT_OK);
    check(CCC_flat_bitset_capacity(&bs).count != 0, true);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check_end((void)CCC_flat_bitset_clear_and_free(&bs, &std_allocator););
}

int
main(void) {
    return check_run(
        flat_bitset_test_push_pop_back_no_reallocate(),
        flat_bitset_test_push_pop_back_allocate()
    );
}
