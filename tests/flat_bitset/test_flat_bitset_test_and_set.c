#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#define FLAT_BITSET_USING_NAMESPACE_CCC
#include "ccc/flat_bitset.h"
#include "ccc/types.h"
#include "checkers.h"

check_static_begin(flat_bitset_test_set_one) {
    check(flat_bitset_set(NULL, 5, CCC_TRUE), CCC_TRIBOOL_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    /* Was false before. */
    check(flat_bitset_set(&bs, 5, CCC_TRUE), CCC_FALSE);
    check(flat_bitset_set(&bs, 5, CCC_TRUE), CCC_TRUE);
    check(flat_bitset_popcount(&bs).count, 1);
    check(flat_bitset_set(&bs, 5, CCC_FALSE), CCC_TRUE);
    check(flat_bitset_set(&bs, 5, CCC_FALSE), CCC_FALSE);
    check(flat_bitset_set_range(&bs, 0, 1, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_test(&bs, 0), CCC_TRUE);
    check(flat_bitset_popcount(&bs).count, 1);
    check_end();
}

check_static_begin(flat_bitset_test_set_shuffled) {
    check(flat_bitset_test(NULL, 0), CCC_TRIBOOL_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    size_t const larger_prime = 11;
    for (size_t i = 0, shuf_i = larger_prime % 10; i < 10;
         ++i, shuf_i = (shuf_i + larger_prime) % 10) {
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_TRUE);
    }
    check(flat_bitset_popcount(&bs).count, 10);
    for (size_t i = 0; i < 10; ++i) {
        check(flat_bitset_test(&bs, i), CCC_TRUE);
        check(flat_bitset_test(&bs, i), CCC_TRUE);
    }
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    check_end();
}

check_static_begin(flat_bitset_test_set_all) {
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    check(flat_bitset_set_all(NULL, CCC_TRUE), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 10);
    for (size_t i = 0; i < 10; ++i) {
        check(flat_bitset_test(&bs, i), CCC_TRUE);
        check(flat_bitset_test(&bs, i), CCC_TRUE);
    }
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    check_end();
}

check_static_begin(flat_bitset_test_set_range) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(
        flat_bitset_set_range(&bs, 0, 513, CCC_TRUE), CCC_RESULT_ARGUMENT_ERROR
    );
    /* Start with a full range and reduce from the end. */
    for (size_t i = 0; i < 512; ++i) {
        size_t const count = 512 - i;
        check(flat_bitset_set_range(&bs, 0, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, 0, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_set_range(&bs, 0, count, CCC_FALSE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, 0, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
    }
    /* Start with a full range and reduce by moving start forward. */
    for (size_t i = 0; i < 512; ++i) {
        size_t const count = 512 - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_set_range(&bs, i, count, CCC_FALSE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
    }
    /* Shrink range from both ends. */
    for (size_t i = 0, end = 512; i < end; ++i, --end) {
        size_t const count = end - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_set_range(&bs, i, count, CCC_FALSE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
    }
    check_end();
}

check_static_begin(flat_bitset_test_reset) {
    check(flat_bitset_reset(NULL, 11), CCC_TRIBOOL_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    check(flat_bitset_reset(&bs, 11), CCC_TRIBOOL_ERROR);
    check(flat_bitset_set(&bs, 0, CCC_TRUE), CCC_FALSE);
    check(flat_bitset_reset_range(&bs, 0, 1), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 0);
    size_t const larger_prime = 11;
    for (size_t i = 0, shuf_i = larger_prime % 10; i < 10;
         ++i, shuf_i = (shuf_i + larger_prime) % 10) {
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_TRUE);
    }
    check(flat_bitset_reset(&bs, 9), CCC_TRUE);
    check(flat_bitset_reset(&bs, 9), CCC_FALSE);
    check(flat_bitset_popcount(&bs).count, 9);
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    check_end();
}

check_static_begin(flat_bitset_test_reset_all) {
    check(flat_bitset_reset_all(NULL), CCC_RESULT_ARGUMENT_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 10);
    check(flat_bitset_reset_all(&bs), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 0);
    check(
        flat_bitset_push_back(&bs, CCC_FALSE, &(CCC_Allocator){}), CCC_RESULT_OK
    );
    check(flat_bitset_popcount(&bs).count, 0);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 11);
    check_end();
}

check_static_begin(flat_bitset_test_reset_range) {
    check(flat_bitset_reset_range(NULL, 0, 1), CCC_RESULT_ARGUMENT_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    /* Start with a full range and reduce from the end. */
    for (size_t i = 0; i < 512; ++i) {
        size_t const count = 512 - i;
        check(flat_bitset_set_range(&bs, 0, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, 0, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_reset_range(&bs, 0, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, 0, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
    }
    /* Start with a full range and reduce by moving start forward. */
    for (size_t i = 0; i < 512; ++i) {
        size_t const count = 512 - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_reset_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
    }
    /* Shrink range from both ends. */
    for (size_t i = 0, end = 512; i < end; ++i, --end) {
        size_t const count = end - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_reset_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
    }
    check_end();
}

check_static_begin(flat_bitset_test_flip) {
    check(flat_bitset_flip(NULL, 9), CCC_TRIBOOL_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    check(flat_bitset_flip(&bs, 11), CCC_TRIBOOL_ERROR);
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 10);
    check(flat_bitset_flip(&bs, 9), CCC_TRUE);
    check(flat_bitset_popcount(&bs).count, 9);
    check(flat_bitset_flip(&bs, 9), CCC_FALSE);
    check(flat_bitset_popcount(&bs).count, 10);
    check_end();
}

check_static_begin(flat_bitset_test_flip_all) {
    check(flat_bitset_flip_all(&flat_bitset_default()), CCC_RESULT_OK);
    check(flat_bitset_flip_all(NULL), CCC_RESULT_ARGUMENT_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(10, (Bit[10]){});
    check(flat_bitset_capacity(&bs).count >= 10, CCC_TRUE);
    for (size_t i = 0; i < 10; i += 2) {
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_popcount(&bs).count, 5);
    check(flat_bitset_flip_all(&bs), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 5);
    for (size_t i = 0; i < 10; ++i) {
        if (i % 2) {
            check(flat_bitset_test(&bs, i), CCC_TRUE);
            check(flat_bitset_test(&bs, i), CCC_TRUE);
        } else {
            check(flat_bitset_test(&bs, i), CCC_FALSE);
            check(flat_bitset_test(&bs, i), CCC_FALSE);
        }
    }
    check_end();
}

check_static_begin(flat_bitset_test_flip_range) {
    check(flat_bitset_flip_range(NULL, 0, 0), CCC_RESULT_ARGUMENT_ERROR);
    check(
        flat_bitset_popcount_range(NULL, 0, 0).error, CCC_RESULT_ARGUMENT_ERROR
    );
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(
        flat_bitset_popcount_range(&bs, 0, 0).error, CCC_RESULT_ARGUMENT_ERROR
    );
    check(flat_bitset_flip_range(&bs, 0, 513), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_flip_range(&bs, 513, 513), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t const orignal_popcount = flat_bitset_popcount(&bs).count;
    /* Start with a full range and reduce from the end. */
    for (size_t i = 0; i < 512; ++i) {
        size_t const count = 512 - i;
        check(flat_bitset_flip_range(&bs, 0, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, 0, count).count, 0);
        check(flat_bitset_popcount(&bs).count, orignal_popcount - count);
        check(flat_bitset_flip_range(&bs, 0, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, 0, count).count, count);
        check(flat_bitset_popcount(&bs).count, orignal_popcount);
    }
    /* Start with a full range and reduce by moving start forward. */
    for (size_t i = 0; i < 512; ++i) {
        size_t const count = 512 - i;
        check(flat_bitset_flip_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, orignal_popcount - count);
        check(flat_bitset_flip_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, orignal_popcount);
    }
    /* Shrink range from both ends. */
    for (size_t i = 0, end = 512; i < end; ++i, --end) {
        size_t const count = end - i;
        check(flat_bitset_flip_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, orignal_popcount - count);
        check(flat_bitset_flip_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, orignal_popcount);
    }
    check_end();
}

check_static_begin(flat_bitset_test_any) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_any_range(&bs, 0, 0), CCC_TRIBOOL_ERROR);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t const cap = flat_bitset_capacity(&bs).count;
    /* Start with a full range and reduce by moving start forward. */
    for (size_t i = 0, end = 512; i < end; ++i, --end) {
        size_t const count = end - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_any(&bs), CCC_TRUE);
        check(flat_bitset_any_range(&bs, 0, end), CCC_TRUE);
        check(flat_bitset_any_range(&bs, 0, cap), CCC_TRUE);
        check(flat_bitset_reset_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
        check(flat_bitset_any(&bs), CCC_FALSE);
        check(flat_bitset_any_range(&bs, 0, cap), CCC_FALSE);
        check(flat_bitset_any_range(&bs, 0, count), CCC_FALSE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_none) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t const cap = flat_bitset_capacity(&bs).count;
    /* Start with a full range and reduce by moving start forward. */
    for (size_t i = 0, end = 512; i < end; ++i, --end) {
        size_t const count = end - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_none(&bs), CCC_FALSE);
        check(flat_bitset_none_range(&bs, 0, cap), CCC_FALSE);
        check(flat_bitset_reset_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
        check(flat_bitset_none(&bs), CCC_TRUE);
        check(flat_bitset_none_range(&bs, 0, cap), CCC_TRUE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_all_three_blocks) {
    Flat_bitset bs = flat_bitset_with_storage(128, (Bit[128]){});
    check(flat_bitset_all_range(&bs, 0, 0), CCC_TRIBOOL_ERROR);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_set(&bs, 64, CCC_FALSE), CCC_TRUE);
    check(flat_bitset_all_range(&bs, 0, 127), CCC_FALSE);
    check(flat_bitset_set(&bs, 64, CCC_TRUE), CCC_FALSE);
    check(flat_bitset_set(&bs, 126, CCC_FALSE), CCC_TRUE);
    check(flat_bitset_all_range(&bs, 0, 127), CCC_FALSE);
    check_end();
}

check_static_begin(flat_bitset_test_all) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    size_t const cap = flat_bitset_capacity(&bs).count;
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_all(&bs), CCC_TRUE);
    check(flat_bitset_all_range(&bs, 0, cap), CCC_TRUE);
    check(flat_bitset_reset_all(&bs), CCC_RESULT_OK);
    /* Start with an almost full range and reduce by moving start forward. */
    for (size_t i = 1, end = 512; i < end; ++i, --end) {
        size_t const count = end - i;
        check(flat_bitset_set_range(&bs, i, count, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, count);
        check(flat_bitset_popcount(&bs).count, count);
        check(flat_bitset_all(&bs), CCC_FALSE);
        check(flat_bitset_all_range(&bs, i, count), CCC_TRUE);
        check(flat_bitset_reset_range(&bs, i, count), CCC_RESULT_OK);
        check(flat_bitset_popcount_range(&bs, i, count).count, 0);
        check(flat_bitset_popcount(&bs).count, 0);
        check(flat_bitset_all(&bs), CCC_FALSE);
        check(flat_bitset_all_range(&bs, i, count), CCC_FALSE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_trailing_one) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(
        flat_bitset_first_trailing_one_range(&bs, 0, 0).error,
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    /* Start with an almost full range and reduce by moving start forward. */
    for (size_t i = 0, end = 512; i < end - 1; ++i) {
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
        check(flat_bitset_first_trailing_one(&bs).count, i + 1);
        check(
            flat_bitset_first_trailing_one_range(&bs, 0, i + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_one_range(&bs, i, end - i).count, i + 1
        );
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_trailing_ones) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    size_t window = FLAT_BITSET_BLOCK_BITS;
    /* Slide a group of int size as a window across the set. */
    for (size_t i = 0; i < (512 - window - 1); ++i) {
        check(flat_bitset_set_range(&bs, i, window, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_first_trailing_ones(&bs, window).count, i);
        check(flat_bitset_first_trailing_ones(&bs, window - 1).count, i);
        check(
            flat_bitset_first_trailing_ones(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, i, window, window).count,
            i
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, i + 1, window, window)
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
    }
    check(flat_bitset_reset_all(&bs), CCC_RESULT_OK);
    window /= 4;
    /* Slide a very small group across the set. */
    for (size_t i = 0; i < (512 - window - 1); ++i) {
        check(flat_bitset_set_range(&bs, i, window, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_first_trailing_ones(&bs, window).count, i);
        check(flat_bitset_first_trailing_ones(&bs, window - 1).count, i);
        check(
            flat_bitset_first_trailing_ones(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, i, window, window).count,
            i
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, i + 1, window, window)
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
    }
    check(flat_bitset_reset_all(&bs), CCC_RESULT_OK);
    window *= 8;
    /* Slide a very large group across the set. */
    for (size_t i = 0; i < (512 - window - 1); ++i) {
        check(flat_bitset_set_range(&bs, i, window, CCC_TRUE), CCC_RESULT_OK);
        check(flat_bitset_first_trailing_ones(&bs, window).count, i);
        check(flat_bitset_first_trailing_ones(&bs, window - 1).count, i);
        check(
            flat_bitset_first_trailing_ones(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, i, window, window).count,
            i
        );
        check(
            flat_bitset_first_trailing_ones_range(&bs, i + 1, window, window)
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_trailing_ones_fail) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(
        flat_bitset_first_trailing_ones_range(&bs, 0, 0, 0).error,
        CCC_RESULT_ARGUMENT_ERROR
    );
    size_t const end = flat_bitset_block_count(512);
    size_t const first_half = FLAT_BITSET_BLOCK_BITS / 2;
    size_t const second_half = first_half - 1;
    /* We are going to search for a group of 16 which we will be very close
       to finding every time but it will be broken by an off bit before the
       16th in a group in every block. */
    for (size_t block = 0, i = 0; block < end;
         ++block, i = block * FLAT_BITSET_BLOCK_BITS) {
        check(
            flat_bitset_set_range(&bs, i, first_half, CCC_TRUE), CCC_RESULT_OK
        );
        check(
            flat_bitset_set_range(
                &bs, i + first_half + 1, second_half, CCC_TRUE
            ),
            CCC_RESULT_OK
        );
        check(
            flat_bitset_first_trailing_ones_range(
                &bs, i, FLAT_BITSET_BLOCK_BITS, first_half + 1
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
    }
    /* Then we will search for a full block worth which we will never find
       thanks to the off bit embedded in each block. */
    check(
        flat_bitset_first_trailing_ones(&bs, FLAT_BITSET_BLOCK_BITS).error
            != CCC_RESULT_OK,
        true
    );
    /* Now fix the last group and we should pass. */
    check(
        flat_bitset_set(
            &bs, ((end - 1) * FLAT_BITSET_BLOCK_BITS) + first_half, CCC_TRUE
        ),
        CCC_FALSE
    );
    /* Now the solution crosses the block border from second to last to last
       block. */
    check(
        flat_bitset_first_trailing_ones(&bs, FLAT_BITSET_BLOCK_BITS).count,
        (((end - 2) * FLAT_BITSET_BLOCK_BITS) + first_half + 1)
    );
    (void)flat_bitset_reset_all(&bs);
    (void)flat_bitset_set_range(&bs, 0, FLAT_BITSET_BLOCK_BITS * 3, CCC_TRUE);
    check(flat_bitset_set(&bs, first_half, CCC_FALSE), CCC_TRUE);
    check(
        flat_bitset_first_trailing_ones_range(
            &bs, 0, FLAT_BITSET_BLOCK_BITS, FLAT_BITSET_BLOCK_BITS
        )
                .error
            != CCC_RESULT_OK,
        true
    );
    check_end();
}

check_static_begin(flat_bitset_test_first_trailing_zero) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    /* Start with an almost full range and reduce by moving start forward. */
    for (size_t i = 0, end = 512; i < end - 1; ++i) {
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
        check(flat_bitset_first_trailing_zero(&bs).count, i + 1);
        check(
            flat_bitset_first_trailing_zero_range(&bs, 0, i + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zero_range(&bs, i, end - i).count, i + 1
        );
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_trailing_zeros) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t window = FLAT_BITSET_BLOCK_BITS;
    /* Slide a group of int size as a window across the set. */
    for (size_t i = 0; i < (512 - window - 1); ++i) {
        check(flat_bitset_set_range(&bs, i, window, CCC_FALSE), CCC_RESULT_OK);
        check(flat_bitset_first_trailing_zeros(&bs, window).count, i);
        check(flat_bitset_first_trailing_zeros(&bs, window - 1).count, i);
        check(
            flat_bitset_first_trailing_zeros(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, i, window, window)
                .count,
            i
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, i + 1, window, window)
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    window /= 4;
    /* Slide a very small group across the set. */
    for (size_t i = 0; i < (512 - window - 1); ++i) {
        check(flat_bitset_set_range(&bs, i, window, CCC_FALSE), CCC_RESULT_OK);
        check(flat_bitset_first_trailing_zeros(&bs, window).count, i);
        check(flat_bitset_first_trailing_zeros(&bs, window - 1).count, i);
        check(
            flat_bitset_first_trailing_zeros(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, i, window, window)
                .count,
            i
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, i + 1, window, window)
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    window *= 8;
    /* Slide a very large group across the set. */
    for (size_t i = 0; i < (512 - window - 1); ++i) {
        check(flat_bitset_set_range(&bs, i, window, CCC_FALSE), CCC_RESULT_OK);
        check(flat_bitset_first_trailing_zeros(&bs, window).count, i);
        check(flat_bitset_first_trailing_zeros(&bs, window - 1).count, i);
        check(
            flat_bitset_first_trailing_zeros(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, i, window, window)
                .count,
            i
        );
        check(
            flat_bitset_first_trailing_zeros_range(&bs, i + 1, window, window)
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_trailing_zeros_fail) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t const end = flat_bitset_block_count(512);
    size_t const first_half = FLAT_BITSET_BLOCK_BITS / 2;
    size_t const second_half = first_half - 1;
    /* We are going to search for a group of 16 which we will be very close
       to finding every time but it will be broken by an off bit before the
       16th in a group in every block. */
    for (size_t block = 0, i = 0; block < end;
         ++block, i = block * FLAT_BITSET_BLOCK_BITS) {
        check(
            flat_bitset_set_range(&bs, i, first_half, CCC_FALSE), CCC_RESULT_OK
        );
        check(
            flat_bitset_set_range(
                &bs, i + first_half + 1, second_half, CCC_FALSE
            ),
            CCC_RESULT_OK
        );
        check(
            flat_bitset_first_trailing_zeros_range(
                &bs, i, FLAT_BITSET_BLOCK_BITS, first_half + 1
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
    }
    /* Then we will search for a full block worth which we will never find
       thanks to the off bit embedded in each block. */
    check(
        flat_bitset_first_trailing_zeros(&bs, FLAT_BITSET_BLOCK_BITS).error
            != CCC_RESULT_OK,
        true
    );
    /* Now fix the last group and we should pass. */
    check(
        flat_bitset_set(
            &bs, ((end - 1) * FLAT_BITSET_BLOCK_BITS) + first_half, CCC_FALSE
        ),
        CCC_TRUE
    );
    /* Now the solution crosses the block border from second to last to last
       block. */
    check(
        flat_bitset_first_trailing_zeros(&bs, FLAT_BITSET_BLOCK_BITS).count,
        (((end - 2) * FLAT_BITSET_BLOCK_BITS) + first_half + 1)
    );
    (void)flat_bitset_reset_all(&bs);
    (void)flat_bitset_set_range(&bs, 0, FLAT_BITSET_BLOCK_BITS * 3, CCC_FALSE);
    check(flat_bitset_set(&bs, first_half, CCC_TRUE), CCC_FALSE);
    check(
        flat_bitset_first_trailing_zeros_range(
            &bs, 0, FLAT_BITSET_BLOCK_BITS, FLAT_BITSET_BLOCK_BITS
        )
                .error
            != CCC_RESULT_OK,
        true
    );
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_one) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    /* Start with an almost full range and reduce by moving start backwards. */
    for (size_t i = 512; i-- > 1;) {
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
        check(flat_bitset_first_leading_one(&bs).count, i - 1);
        check(
            flat_bitset_first_leading_one_range(&bs, i, 512 - i + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(flat_bitset_first_leading_one_range(&bs, 0, i + 1).count, i - 1);
    }
    check(flat_bitset_first_leading_one(&bs).count, 0);
    check(flat_bitset_first_leading_one_range(&bs, 0, 1).count, 0);
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_one_range) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    size_t const bit_of_interest = 4;
    check(flat_bitset_set(&bs, bit_of_interest, CCC_TRUE), CCC_FALSE);
    for (size_t i = 0; i < bit_of_interest; ++i) {
        /* Testing our code paths that include only a single block to read. */
        check(
            flat_bitset_first_leading_one_range(
                &bs, i, (bit_of_interest - i) + 1
            )
                .count,
            bit_of_interest
        );
    }
    /* It is important that our bit set not report a false positive here. No
       matter the block size, a single bit matching our query will be loaded
       with the block. But the implementation must ensure that bit is not
       a match if it is out of range. Here the bit is not in our searched range
       so we do not find any bits matching our query. */
    check(
        flat_bitset_first_leading_one_range(
            &bs,
            bit_of_interest + 1,
            flat_bitset_count(&bs).count - (bit_of_interest + 1)
        )
                .error
            != CCC_RESULT_OK,
        true
    );
    check(
        flat_bitset_first_leading_one_range(&bs, 0, bit_of_interest).error
            != CCC_RESULT_OK,
        true
    );
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_ones) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(
        flat_bitset_first_leading_ones_range(&bs, 0, 0, 0).error,
        CCC_RESULT_ARGUMENT_ERROR
    );
    size_t window = FLAT_BITSET_BLOCK_BITS;
    /* Slide a group of int size as a window across the set. */
    for (size_t i = 511; i > window + 1; --i) {
        check(
            flat_bitset_set_range(&bs, i - window + 1, window, CCC_TRUE),
            CCC_RESULT_OK
        );
        check(flat_bitset_first_leading_ones(&bs, window).count, i);
        check(flat_bitset_first_leading_ones(&bs, window - 1).count, i);
        check(
            flat_bitset_first_leading_ones(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_ones_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs, i - window + 1, window, window
            )
                .count,
            i
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs, i - window, window, window
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
    }
    check(flat_bitset_reset_all(&bs), CCC_RESULT_OK);
    window /= 4;
    /* Slide a very small group across the set. */
    for (size_t i = 511; i > window + 1; --i) {
        check(
            flat_bitset_set_range(&bs, i - window + 1, window, CCC_TRUE),
            CCC_RESULT_OK
        );
        check(flat_bitset_first_leading_ones(&bs, window).count, i);
        check(flat_bitset_first_leading_ones(&bs, window - 1).count, i);
        check(
            flat_bitset_first_leading_ones(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_ones_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs, i - window + 1, window, window
            )
                .count,
            i
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs, i - window, window, window
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
    }
    check(flat_bitset_reset_all(&bs), CCC_RESULT_OK);
    window *= 8;
    /* Slide a very large group across the set. */
    for (size_t i = 511; i > window + 1; --i) {
        check(
            flat_bitset_set_range(&bs, i - window + 1, window, CCC_TRUE),
            CCC_RESULT_OK
        );
        check(flat_bitset_first_leading_ones(&bs, window).count, i);
        check(flat_bitset_first_leading_ones(&bs, window - 1).count, i);
        check(
            flat_bitset_first_leading_ones(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_ones_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs, i - window + 1, window, window
            )
                .count,
            i
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs, i - window, window, window
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_FALSE), CCC_TRUE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_ones_fail) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    size_t const first_half = FLAT_BITSET_BLOCK_BITS / 2;
    size_t const second_half = first_half - 1;
    /* We are going to search for a group of 17 which we will be very close
       to finding every time but it will be broken by an off bit before the
       17th in a group in every block. */
    for (size_t block = flat_bitset_block_count(512); block--;) {
        check(
            flat_bitset_set_range(
                &bs, (block * FLAT_BITSET_BLOCK_BITS), first_half, CCC_TRUE
            ),
            CCC_RESULT_OK
        );
        check(
            flat_bitset_set_range(
                &bs,
                (block * FLAT_BITSET_BLOCK_BITS) + first_half + 1,
                second_half,
                CCC_TRUE
            ),
            CCC_RESULT_OK
        );
        check(
            flat_bitset_first_leading_ones_range(
                &bs,
                (block * FLAT_BITSET_BLOCK_BITS),
                FLAT_BITSET_BLOCK_BITS,
                first_half + 1
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
    }
    /* Then we will search for a full block worth which we will never find
       thanks to the off bit embedded in each block. */
    check(
        flat_bitset_first_leading_ones(&bs, FLAT_BITSET_BLOCK_BITS).error
            != CCC_RESULT_OK,
        true
    );
    /* Now fix the last group and we should pass. */
    check(flat_bitset_set(&bs, first_half, CCC_TRUE), CCC_FALSE);
    /* Now the solution crosses the block border from second to last to last
       block. */
    check(
        flat_bitset_first_leading_ones(&bs, FLAT_BITSET_BLOCK_BITS).count,
        FLAT_BITSET_BLOCK_BITS + first_half - 1
    );
    (void)flat_bitset_reset_all(&bs);
    (void)flat_bitset_set_range(
        &bs,
        512 - (FLAT_BITSET_BLOCK_BITS * 3),
        FLAT_BITSET_BLOCK_BITS * 3,
        CCC_TRUE
    );
    check(flat_bitset_set(&bs, 512 - first_half, CCC_FALSE), CCC_TRUE);
    check(
        flat_bitset_first_leading_ones_range(
            &bs,
            512 - FLAT_BITSET_BLOCK_BITS,
            FLAT_BITSET_BLOCK_BITS,
            FLAT_BITSET_BLOCK_BITS
        )
                .error
            != CCC_RESULT_OK,
        true
    );
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_zero) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    /* Start with an almost full range and reduce by moving start backwards. */
    for (size_t i = 512; i-- > 1;) {
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
        check(flat_bitset_first_leading_zero(&bs).count, i - 1);
        check(
            flat_bitset_first_leading_zero_range(&bs, i, 512 - i + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(flat_bitset_first_leading_zero_range(&bs, 0, i + 1).count, i - 1);
    }
    check(flat_bitset_first_leading_zero(&bs).count, 0);
    check(flat_bitset_first_leading_zero_range(&bs, 0, 1).count, 0);
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_zero_range) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    (void)flat_bitset_set_all(&bs, CCC_TRUE);
    size_t const bit_of_interest = 4;
    check(flat_bitset_set(&bs, bit_of_interest, CCC_FALSE), CCC_TRUE);
    for (size_t i = 0; i < bit_of_interest; ++i) {
        /* Testing our code paths that include only a single block to read. */
        check(
            flat_bitset_first_leading_zero_range(
                &bs, i, (bit_of_interest - i) + 1
            )
                .count,
            bit_of_interest
        );
    }
    /* It is important that our bit set not report a false positive here. No
       matter the block size, a single bit matching our query will be loaded
       with the block. But the implementation must ensure that bit is not
       a match if it is out of range. Here the bit is not in our searched range
       so we do not find any bits matching our query. */
    check(
        flat_bitset_first_leading_zero_range(
            &bs,
            bit_of_interest + 1,
            flat_bitset_count(&bs).count - (bit_of_interest + 1)
        )
                .error
            != CCC_RESULT_OK,
        true
    );
    check(
        flat_bitset_first_leading_zero_range(&bs, 0, bit_of_interest).error
            != CCC_RESULT_OK,
        true
    );
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_zeros) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t window = FLAT_BITSET_BLOCK_BITS;
    /* Slide a group of int size as a window across the set. */
    for (size_t i = 511; i > window + 1; --i) {
        check(
            flat_bitset_set_range(&bs, i - window + 1, window, CCC_FALSE),
            CCC_RESULT_OK
        );
        check(flat_bitset_first_leading_zeros(&bs, window).count, i);
        check(flat_bitset_first_leading_zeros(&bs, window - 1).count, i);
        check(
            flat_bitset_first_leading_zeros(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_zeros_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs, i - window + 1, window, window
            )
                .count,
            i
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs, i - window, window, window
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    window /= 4;
    /* Slide a very small group across the set. */
    for (size_t i = 511; i > window + 1; --i) {
        check(
            flat_bitset_set_range(&bs, i - window + 1, window, CCC_FALSE),
            CCC_RESULT_OK
        );
        check(flat_bitset_first_leading_zeros(&bs, window).count, i);
        check(flat_bitset_first_leading_zeros(&bs, window - 1).count, i);
        check(
            flat_bitset_first_leading_zeros(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_zeros_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs, i - window + 1, window, window
            )
                .count,
            i
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs, i - window, window, window
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    window *= 8;
    /* Slide a very large group across the set. */
    for (size_t i = 511; i > window + 1; --i) {
        check(
            flat_bitset_set_range(&bs, i - window + 1, window, CCC_FALSE),
            CCC_RESULT_OK
        );
        check(flat_bitset_first_leading_zeros(&bs, window).count, i);
        check(flat_bitset_first_leading_zeros(&bs, window - 1).count, i);
        check(
            flat_bitset_first_leading_zeros(&bs, window + 1).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_zeros_range(&bs, 0, i, window).error
                != CCC_RESULT_OK,
            true
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs, i - window + 1, window, window
            )
                .count,
            i
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs, i - window, window, window
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
        /* Cleanup behind as we go. */
        check(flat_bitset_set(&bs, i, CCC_TRUE), CCC_FALSE);
    }
    check_end();
}

check_static_begin(flat_bitset_test_first_leading_zeros_fail) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t const first_half = FLAT_BITSET_BLOCK_BITS / 2;
    size_t const second_half = first_half - 1;
    /* We are going to search for a group of 17 which we will be very close
       to finding every time but it will be broken by an off bit before the
       17th in a group in every block. */
    for (size_t block = flat_bitset_block_count(512); block--;) {
        check(
            flat_bitset_set_range(
                &bs, (block * FLAT_BITSET_BLOCK_BITS), first_half, CCC_FALSE
            ),
            CCC_RESULT_OK
        );
        check(
            flat_bitset_set_range(
                &bs,
                (block * FLAT_BITSET_BLOCK_BITS) + first_half + 1,
                second_half,
                CCC_FALSE
            ),
            CCC_RESULT_OK
        );
        check(
            flat_bitset_first_leading_zeros_range(
                &bs,
                (block * FLAT_BITSET_BLOCK_BITS),
                FLAT_BITSET_BLOCK_BITS,
                first_half + 1
            )
                    .error
                != CCC_RESULT_OK,
            true
        );
    }
    /* Then we will search for a full block worth which we will never find
       thanks to the off bit embedded in each block. */
    check(
        flat_bitset_first_leading_zeros(&bs, FLAT_BITSET_BLOCK_BITS).error
            != CCC_RESULT_OK,
        true
    );
    /* Now fix the last group and we should pass. */
    check(flat_bitset_set(&bs, first_half, CCC_FALSE), CCC_TRUE);
    /* Now the solution crosses the block border from second to last to last
       block. */
    check(
        flat_bitset_first_leading_zeros(&bs, FLAT_BITSET_BLOCK_BITS).count,
        FLAT_BITSET_BLOCK_BITS + first_half - 1
    );
    (void)flat_bitset_reset_all(&bs);
    (void)flat_bitset_set_range(
        &bs,
        512 - (FLAT_BITSET_BLOCK_BITS * 3),
        FLAT_BITSET_BLOCK_BITS * 3,
        CCC_FALSE
    );
    check(flat_bitset_set(&bs, 512 - first_half, CCC_TRUE), CCC_FALSE);
    check(
        flat_bitset_first_leading_zeros_range(
            &bs,
            512 - FLAT_BITSET_BLOCK_BITS,
            FLAT_BITSET_BLOCK_BITS,
            FLAT_BITSET_BLOCK_BITS
        )
                .error
            != CCC_RESULT_OK,
        true
    );
    check_end();
}

check_static_begin(flat_bitset_test_or_same_size) {
    check(
        flat_bitset_or(&flat_bitset_default(), &flat_bitset_default()),
        CCC_RESULT_OK
    );
    Flat_bitset source = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset destination = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_or(&destination, &flat_bitset_default()), CCC_RESULT_OK);
    check(flat_bitset_or(&source, &flat_bitset_default()), CCC_RESULT_OK);
    check(flat_bitset_or(NULL, &source), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_or(&destination, NULL), CCC_RESULT_ARGUMENT_ERROR);
    size_t const size = 512;
    for (size_t i = 0; i < size; i += 2) {
        check(flat_bitset_set(&destination, i, CCC_TRUE), CCC_FALSE);
    }
    for (size_t i = 1; i < size; i += 2) {
        check(flat_bitset_set(&source, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_popcount(&source).count, size / 2);
    check(flat_bitset_popcount(&destination).count, size / 2);
    check(flat_bitset_or(&destination, &source), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, size);
    check_end();
}

check_static_begin(flat_bitset_test_or_diff_size) {
    Flat_bitset destination = flat_bitset_with_storage(512, (Bit[512]){});
    /* Make it slightly harder by not ending on a perfect block boundary. */
    Flat_bitset source = flat_bitset_with_storage(244, (Bit[244]){});
    check(flat_bitset_set_all(&source, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&source).count, 244);
    check(flat_bitset_popcount(&destination).count, 0);
    check(flat_bitset_or(&destination, &source), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, 244);
    check_end();
}

check_static_begin(flat_bitset_test_and_same_size) {
    Flat_bitset zero = flat_bitset_default();
    Flat_bitset to_zero = flat_bitset_with_storage(32, (Bit[32]){});
    check(flat_bitset_and(&zero, &to_zero), CCC_RESULT_OK);
    check(flat_bitset_popcount(&zero).count, 0);
    check(flat_bitset_and(&to_zero, &zero), CCC_RESULT_OK);
    check(flat_bitset_popcount(&to_zero).count, 0);
    Flat_bitset source = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset destination = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_and(NULL, &source), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_and(&destination, NULL), CCC_RESULT_ARGUMENT_ERROR);
    size_t const size = 512;
    for (size_t i = 0; i < size; i += 2) {
        check(flat_bitset_set(&destination, i, CCC_TRUE), CCC_FALSE);
    }
    for (size_t i = 1; i < size; i += 2) {
        check(flat_bitset_set(&source, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_popcount(&source).count, size / 2);
    check(flat_bitset_popcount(&destination).count, size / 2);
    check(flat_bitset_and(&destination, &source), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, 0);
    check_end();
}

check_static_begin(flat_bitset_test_and_diff_size) {
    Flat_bitset destination = flat_bitset_with_storage(512, (Bit[512]){});
    /* Make it slightly harder by not ending on a perfect block boundary. */
    Flat_bitset source = flat_bitset_with_storage(244, (Bit[244]){});
    check(flat_bitset_set_all(&destination, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_set_all(&source, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, 512);
    check(flat_bitset_popcount(&source).count, 244);
    check(flat_bitset_and(&destination, &source), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, 244);
    check(flat_bitset_count(&destination).count, 512);
    check_end();
}

check_static_begin(flat_bitset_test_xor_same_size) {
    check(
        flat_bitset_or(&flat_bitset_default(), &flat_bitset_default()),
        CCC_RESULT_OK
    );
    Flat_bitset source = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset destination = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_xor(NULL, &destination), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_xor(&destination, NULL), CCC_RESULT_ARGUMENT_ERROR);
    check(flat_bitset_xor(&destination, &flat_bitset_default()), CCC_RESULT_OK);
    check(flat_bitset_xor(&source, &flat_bitset_default()), CCC_RESULT_OK);
    size_t const size = 512;
    for (size_t i = 0; i < size; i += 2) {
        check(flat_bitset_set(&destination, i, CCC_TRUE), CCC_FALSE);
    }
    for (size_t i = 1; i < size; i += 2) {
        check(flat_bitset_set(&source, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_popcount(&source).count, size / 2);
    check(flat_bitset_popcount(&destination).count, size / 2);
    check(flat_bitset_xor(&destination, &source), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, size);
    check_end();
}

check_static_begin(flat_bitset_test_xor_diff_size) {
    Flat_bitset destination = flat_bitset_with_storage(512, (Bit[512]){});
    /* Make it slightly harder by not ending on a perfect block boundary. */
    Flat_bitset source = flat_bitset_with_storage(244, (Bit[244]){});
    check(flat_bitset_set_all(&destination, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_set_all(&source, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, 512);
    check(flat_bitset_popcount(&source).count, 244);
    check(flat_bitset_xor(&destination, &source), CCC_RESULT_OK);
    check(flat_bitset_popcount(&destination).count, 512 - 244);
    check(flat_bitset_count(&destination).count, 512);
    check_end();
}

check_static_begin(flat_bitset_test_shift_left) {
    check(flat_bitset_shift_left(NULL, 0), CCC_RESULT_ARGUMENT_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_shift_left(&bs, 0), CCC_RESULT_OK);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 512);
    check(flat_bitset_shift_left(&bs, 512), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 0);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t ones = 512;
    check(flat_bitset_shift_left(&bs, FLAT_BITSET_BLOCK_BITS), CCC_RESULT_OK);
    check(flat_bitset_popcount_range(&bs, 0, FLAT_BITSET_BLOCK_BITS).count, 0);
    ones -= FLAT_BITSET_BLOCK_BITS;
    check(flat_bitset_popcount(&bs).count, ones);
    check(
        flat_bitset_shift_left(&bs, FLAT_BITSET_BLOCK_BITS / 2), CCC_RESULT_OK
    );
    ones -= (FLAT_BITSET_BLOCK_BITS / 2);
    check(flat_bitset_popcount(&bs).count, ones);
    check(
        flat_bitset_shift_left(&bs, FLAT_BITSET_BLOCK_BITS * 2), CCC_RESULT_OK
    );
    ones -= (FLAT_BITSET_BLOCK_BITS * 2);
    check(flat_bitset_popcount(&bs).count, ones);
    check(
        flat_bitset_shift_left(&bs, (FLAT_BITSET_BLOCK_BITS - 3) * 3),
        CCC_RESULT_OK
    );
    ones -= ((FLAT_BITSET_BLOCK_BITS - 3) * 3);
    check(flat_bitset_popcount(&bs).count, ones);
    check_end();
}

check_static_begin(flat_bitset_test_shift_left_edgecase) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 512);
    check(flat_bitset_shift_left(&bs, 510), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 2);
    check_end();
}

check_static_begin(flat_bitset_test_shift_left_edgecase_small) {
    Flat_bitset bs = flat_bitset_with_storage(8, (Bit[8]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 8);
    check(flat_bitset_shift_left(&bs, 7), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 1);
    check_end();
}

check_static_begin(flat_bitset_test_shift_right) {
    check(flat_bitset_shift_right(NULL, 0), CCC_RESULT_ARGUMENT_ERROR);
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_shift_right(&bs, 0), CCC_RESULT_OK);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 512);
    check(flat_bitset_shift_right(&bs, 512), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 0);
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    size_t ones = 512;
    check(flat_bitset_shift_right(&bs, FLAT_BITSET_BLOCK_BITS), CCC_RESULT_OK);
    check(
        flat_bitset_popcount_range(
            &bs, 512 - FLAT_BITSET_BLOCK_BITS, FLAT_BITSET_BLOCK_BITS
        )
            .count,
        0
    );
    ones -= FLAT_BITSET_BLOCK_BITS;
    check(flat_bitset_popcount(&bs).count, ones);
    check(
        flat_bitset_shift_right(&bs, FLAT_BITSET_BLOCK_BITS / 2), CCC_RESULT_OK
    );
    ones -= (FLAT_BITSET_BLOCK_BITS / 2);
    check(flat_bitset_popcount(&bs).count, ones);
    check(
        flat_bitset_shift_right(&bs, FLAT_BITSET_BLOCK_BITS * 2), CCC_RESULT_OK
    );
    ones -= (FLAT_BITSET_BLOCK_BITS * 2);
    check(flat_bitset_popcount(&bs).count, ones);
    check(
        flat_bitset_shift_right(&bs, (FLAT_BITSET_BLOCK_BITS - 3) * 3),
        CCC_RESULT_OK
    );
    ones -= ((FLAT_BITSET_BLOCK_BITS - 3) * 3);
    check(flat_bitset_popcount(&bs).count, ones);
    check_end();
}

check_static_begin(flat_bitset_test_shift_right_edgecase) {
    Flat_bitset bs = flat_bitset_with_storage(512, (Bit[512]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 512);
    check(flat_bitset_shift_right(&bs, 510), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 2);
    check_end();
}

check_static_begin(flat_bitset_test_shift_right_edgecase_small) {
    Flat_bitset bs = flat_bitset_with_storage(8, (Bit[8]){});
    check(flat_bitset_set_all(&bs, CCC_TRUE), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 8);
    check(flat_bitset_shift_right(&bs, 7), CCC_RESULT_OK);
    check(flat_bitset_popcount(&bs).count, 1);
    check_end();
}

check_static_begin(flat_bitset_test_subset) {
    Flat_bitset set = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset subset1 = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset subset2 = flat_bitset_with_storage(244, (Bit[244]){});
    for (size_t i = 0; i < 512; i += 2) {
        check(flat_bitset_set(&set, i, CCC_TRUE), CCC_FALSE);
        check(flat_bitset_set(&subset1, i, CCC_TRUE), CCC_FALSE);
    }
    for (size_t i = 0; i < 244; i += 2) {
        check(flat_bitset_set(&subset2, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_is_subset(&subset1, &set), CCC_TRUE);
    check(flat_bitset_is_subset(&subset2, &set), CCC_TRUE);
    for (size_t i = 0; i < 244; i += 2) {
        check(flat_bitset_set(&set, i, CCC_FALSE), CCC_TRUE);
    }
    check(flat_bitset_is_subset(&subset1, &set), CCC_FALSE);
    check(flat_bitset_is_subset(&subset2, &set), CCC_FALSE);
    check_end();
}

check_static_begin(flat_bitset_test_proper_subset) {
    Flat_bitset set = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset subset1 = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset subset2 = flat_bitset_with_storage(244, (Bit[244]){});
    check(flat_bitset_is_proper_subset(NULL, &set), CCC_TRIBOOL_ERROR);
    check(flat_bitset_is_proper_subset(&subset1, NULL), CCC_TRIBOOL_ERROR);
    check(flat_bitset_is_subset(NULL, &set), CCC_TRIBOOL_ERROR);
    check(flat_bitset_is_subset(&subset1, NULL), CCC_TRIBOOL_ERROR);
    for (size_t i = 0; i < 512; i += 2) {
        check(flat_bitset_set(&set, i, CCC_TRUE), CCC_FALSE);
        check(flat_bitset_set(&subset1, i, CCC_TRUE), CCC_FALSE);
    }
    for (size_t i = 0; i < 244; i += 2) {
        check(flat_bitset_set(&subset2, i, CCC_TRUE), CCC_FALSE);
    }
    check(flat_bitset_is_proper_subset(&subset1, &set), CCC_FALSE);
    check(flat_bitset_is_subset(&subset1, &set), CCC_TRUE);
    check(flat_bitset_is_subset(&subset2, &set), CCC_TRUE);
    check(flat_bitset_is_subset(&set, &subset2), CCC_FALSE);
    check(flat_bitset_is_proper_subset(&subset2, &set), CCC_TRUE);
    for (size_t i = 0; i < 244; i += 2) {
        check(flat_bitset_set(&set, i, CCC_FALSE), CCC_TRUE);
    }
    check(flat_bitset_is_subset(&subset1, &set), CCC_FALSE);
    check(flat_bitset_is_subset(&subset2, &set), CCC_FALSE);
    check_end();
}

check_static_begin(flat_bitset_test_is_equal) {
    Flat_bitset a = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset b = flat_bitset_with_storage(512, (Bit[512]){});
    Flat_bitset c = flat_bitset_with_storage(244, (Bit[244]){});
    check(flat_bitset_is_equal(NULL, &b), CCC_TRIBOOL_ERROR);
    check(flat_bitset_is_equal(&a, NULL), CCC_TRIBOOL_ERROR);
    check(flat_bitset_is_equal(&a, &b), CCC_TRUE);
    check(flat_bitset_is_equal(&a, &c), CCC_FALSE);
    (void)flat_bitset_set(&a, 511, CCC_TRUE);
    check(flat_bitset_is_equal(&a, &b), CCC_FALSE);
    (void)flat_bitset_set(&b, 511, CCC_TRUE);
    check(flat_bitset_is_equal(&a, &b), CCC_TRUE);
    check(
        flat_bitset_is_equal(&flat_bitset_default(), &flat_bitset_default()),
        CCC_TRUE
    );
    check_end();
}

/* Returns if the box is valid. 1 for valid, 0 for invalid, -1 for an error */
static CCC_Tribool
validate_sudoku_box(
    int board[9][9],
    Flat_bitset *const row_check,
    Flat_bitset *const col_check,
    size_t const row_start,
    size_t const col_start
) {
    Flat_bitset box_check = flat_bitset_with_storage(9, (Bit[9]){});
    CCC_Tribool was_on = CCC_FALSE;
    for (size_t r = row_start; r < row_start + 3; ++r) {
        for (size_t c = col_start; c < col_start + 3; ++c) {
            if (!board[r][c]) {
                continue;
            }
            /* Need the zero based digit. */
            size_t const digit = (size_t)board[r][c] - 1;
            was_on = flat_bitset_set(&box_check, digit, CCC_TRUE);
            if (was_on != CCC_FALSE) {
                goto done;
            }
            was_on = flat_bitset_set(row_check, (r * 9) + digit, CCC_TRUE);
            if (was_on != CCC_FALSE) {
                goto done;
            }
            was_on = flat_bitset_set(col_check, (c * 9) + digit, CCC_TRUE);
            if (was_on != CCC_FALSE) {
                goto done;
            }
        }
    }
done:
    switch (was_on) {
        case CCC_TRUE:
            return CCC_FALSE;
            break;
        case CCC_FALSE:
            return CCC_TRUE;
            break;
        case CCC_TRIBOOL_ERROR:
            return CCC_TRIBOOL_ERROR;
            break;
        default:
            return CCC_TRIBOOL_ERROR;
    }
}

/* A small problem like this is a perfect use case for a stack based bit set.
   All sizes are known at compile time meaning we get memory management for
   free and the optimal space and time complexity for this problem. */

check_static_begin(flat_bitset_test_valid_sudoku) {
    /* clang-format off */
    int valid_board[9][9] =
    {{5,3,0, 0,7,0, 0,0,0}
    ,{6,0,0, 1,9,5, 0,0,0}
    ,{0,9,8, 0,0,0, 0,6,0}

    ,{8,0,0, 0,6,0, 0,0,3}
    ,{4,0,0, 8,0,3, 0,0,1}
    ,{7,0,0, 0,2,0, 0,0,6}

    ,{0,6,0, 0,0,0, 2,8,0}
    ,{0,0,0, 4,1,9, 0,0,5}
    ,{0,0,0, 0,8,0, 0,7,9}};
    /* clang-format on */
    Flat_bitset row_check = flat_bitset_with_storage(9L * 9L, (Bit[9L * 9L]){});
    Flat_bitset col_check = flat_bitset_with_storage(9L * 9L, (Bit[9L * 9L]){});
    size_t const box_step = 3;
    for (size_t row = 0; row < 9; row += box_step) {
        for (size_t col = 0; col < 9; col += box_step) {
            CCC_Tribool const valid = validate_sudoku_box(
                valid_board, &row_check, &col_check, row, col
            );
            check(valid, CCC_TRUE);
        }
    }
    check_end();
}

check_static_begin(flat_bitset_test_invalid_sudoku) {
    /* clang-format off */
    int invalid_board[9][9] =
    {{8,3,0, 0,7,0, 0,0,0} /* 8 in first box top left. */
    ,{6,0,0, 1,9,5, 0,0,0}
    ,{0,9,8, 0,0,0, 0,6,0} /* 8 in first box bottom right. */

    ,{8,0,0, 0,6,0, 0,0,3} /* 8 also overlaps with 8 in top left by row. */
    ,{4,0,0, 8,0,3, 0,0,1}
    ,{7,0,0, 0,2,0, 0,0,6}

    ,{0,6,0, 0,0,0, 2,8,0}
    ,{0,0,0, 4,1,9, 0,0,5}
    ,{0,0,0, 0,8,0, 0,7,9}};
    /* clang-format on */
    Flat_bitset row_check = flat_bitset_with_storage(9L * 9L, (Bit[9L * 9L]){});
    Flat_bitset col_check = flat_bitset_with_storage(9L * 9L, (Bit[9L * 9L]){});
    size_t const box_step = 3;
    CCC_Tribool pass = CCC_TRUE;
    for (size_t row = 0; row < 9; row += box_step) {
        for (size_t col = 0; col < 9; col += box_step) {
            pass = validate_sudoku_box(
                invalid_board, &row_check, &col_check, row, col
            );
            check(pass != CCC_TRIBOOL_ERROR, true);
            if (!pass) {
                goto done;
            }
        }
    }
done:
    check(pass, CCC_FALSE);
    check_end();
}

int
main(void) {
    return check_run(
        flat_bitset_test_set_one(),
        flat_bitset_test_set_shuffled(),
        flat_bitset_test_set_all(),
        flat_bitset_test_set_range(),
        flat_bitset_test_reset(),
        flat_bitset_test_flip(),
        flat_bitset_test_flip_all(),
        flat_bitset_test_flip_range(),
        flat_bitset_test_reset_all(),
        flat_bitset_test_reset_range(),
        flat_bitset_test_any(),
        flat_bitset_test_all_three_blocks(),
        flat_bitset_test_all(),
        flat_bitset_test_none(),
        flat_bitset_test_first_trailing_one(),
        flat_bitset_test_first_trailing_ones(),
        flat_bitset_test_first_trailing_ones_fail(),
        flat_bitset_test_first_trailing_zero(),
        flat_bitset_test_first_trailing_zeros(),
        flat_bitset_test_first_trailing_zeros_fail(),
        flat_bitset_test_first_leading_one(),
        flat_bitset_test_first_leading_one_range(),
        flat_bitset_test_first_leading_ones(),
        flat_bitset_test_first_leading_ones_fail(),
        flat_bitset_test_first_leading_zero(),
        flat_bitset_test_first_leading_zero_range(),
        flat_bitset_test_first_leading_zeros(),
        flat_bitset_test_first_leading_zeros_fail(),
        flat_bitset_test_or_same_size(),
        flat_bitset_test_or_diff_size(),
        flat_bitset_test_and_same_size(),
        flat_bitset_test_and_diff_size(),
        flat_bitset_test_xor_same_size(),
        flat_bitset_test_xor_diff_size(),
        flat_bitset_test_shift_left(),
        flat_bitset_test_shift_right(),
        flat_bitset_test_shift_left_edgecase(),
        flat_bitset_test_shift_right_edgecase(),
        flat_bitset_test_shift_left_edgecase_small(),
        flat_bitset_test_shift_right_edgecase_small(),
        flat_bitset_test_subset(),
        flat_bitset_test_proper_subset(),
        flat_bitset_test_valid_sudoku(),
        flat_bitset_test_invalid_sudoku(),
        flat_bitset_test_is_equal(),
    );
}
