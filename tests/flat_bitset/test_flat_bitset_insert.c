#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TRAITS_USING_NAMESPACE_CCC

#include "ccc/flat_bitset.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "tests/checkers.h"
#include "utility/stack_allocator.h"
#include "utility/std_allocator.h"

check_static_begin(flat_bitset_test_push_back_no_reallocate) {
    check(CCC_flat_bitset_popcount(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(
        CCC_flat_bitset_push_back(NULL, CCC_TRUE, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    CCC_Flat_bitset bs = CCC_flat_bitset_for(
        16, 0, CCC_flat_bitset_storage_for((CCC_Bit[16]){})
    );
    check(
        CCC_flat_bitset_push_back(&bs, CCC_TRIBOOL_ERROR, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_bitset_push_back(&bs, CCC_TRUE, NULL),
        CCC_RESULT_ARGUMENT_ERROR
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
    check(CCC_flat_bitset_clear(&bs), CCC_RESULT_OK);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    check(CCC_flat_bitset_capacity(&bs).count, 16);
    check(
        CCC_flat_bitset_clear_and_free(&bs, &(CCC_Allocator){}),
        CCC_RESULT_NO_ALLOCATION_FUNCTION
    );
    check(CCC_flat_bitset_capacity(&bs).count, 16);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check_end();
}

check_static_begin(flat_bitset_test_push_back_allocate) {
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
    check(CCC_flat_bitset_clear(&bs), CCC_RESULT_OK);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    check(CCC_flat_bitset_capacity(&bs).count != 0, true);
    check_end(CCC_flat_bitset_clear_and_free(&bs, &std_allocator););
}

check_static_begin(flat_bitset_test_push_back_reserve) {
    CCC_Flat_bitset bs = CCC_flat_bitset_default();
    CCC_Result const r = CCC_flat_bitset_reserve(&bs, 512, &std_allocator);
    check(r, CCC_RESULT_OK);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check(CCC_flat_bitset_capacity(&bs).count != 0, true);
    for (size_t i = 0; CCC_flat_bitset_count(&bs).count < 512; ++i) {
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
    check(CCC_flat_bitset_count(&bs).count, 512);
    check(CCC_flat_bitset_popcount(&bs).count, 512 / 2);
    check(clear(&bs), CCC_RESULT_OK);
    check(CCC_flat_bitset_count(&bs).count, 0);
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    check(CCC_flat_bitset_capacity(&bs).count != 0, true);
    check_end((void)clear_and_free(&bs, &std_allocator););
}

check_static_begin(flat_bitset_test_reserve_fail) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[32]){})),
    };
    CCC_Flat_bitset bs = CCC_flat_bitset_default();
    check(
        CCC_flat_bitset_reserve(&bs, 64, &allocator), CCC_RESULT_ALLOCATOR_ERROR
    );
    check(
        CCC_flat_bitset_reserve(&bs, SIZE_MAX, &allocator),
        CCC_RESULT_ALLOCATOR_ERROR
    );
    check_end((void)clear_and_free(&bs, &std_allocator););
}

int
main(void) {
    return check_run(
        flat_bitset_test_push_back_no_reallocate(),
        flat_bitset_test_push_back_allocate(),
        flat_bitset_test_push_back_reserve(),
        flat_bitset_test_reserve_fail()
    );
}
