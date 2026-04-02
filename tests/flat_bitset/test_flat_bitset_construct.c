#include <stddef.h>

#include "ccc/flat_bitset.h"
#include "ccc/types.h"
#include "checkers.h"
#include "str_view/str_view.h"
#include "utility/stack_allocator.h"

typedef typeof(*(CCC_Flat_bitset){}.blocks) Bitblocks;

static CCC_Flat_bitset static_bitset
    = CCC_flat_bitset_with_storage(32, (CCC_Bit[32]){});

check_static_begin(flat_bitset_test_static) {
    check(CCC_flat_bitset_data(NULL), NULL);
    check(CCC_flat_bitset_is_empty(NULL), CCC_TRIBOOL_ERROR);
    check(CCC_flat_bitset_capacity(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_flat_bitset_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(
        CCC_flat_bitset_blocks_capacity(NULL).error, CCC_RESULT_ARGUMENT_ERROR
    );
    check(CCC_flat_bitset_blocks_count(NULL).error, CCC_RESULT_ARGUMENT_ERROR);
    check(CCC_flat_bitset_data(&static_bitset) != NULL, CCC_TRUE);
    check(CCC_flat_bitset_popcount(&static_bitset).count, 0);
    check(CCC_flat_bitset_capacity(&static_bitset).count >= 32, CCC_TRUE);
    check(CCC_flat_bitset_blocks_capacity(&static_bitset).count >= 1, CCC_TRUE);
    check(CCC_flat_bitset_blocks_count(&static_bitset).count >= 1, CCC_TRUE);
    size_t i = 0;
    while (i < CCC_flat_bitset_capacity(&static_bitset).count) {
        check(CCC_flat_bitset_test(&static_bitset, i), CCC_FALSE);
        check(CCC_flat_bitset_test(&static_bitset, i), CCC_FALSE);
        ++i;
    }
    check(i >= 32, CCC_TRUE);
    check_end();
}

check_static_begin(flat_bitset_test_construct) {
    CCC_Flat_bitset bs = CCC_flat_bitset_for(
        10, 10, CCC_flat_bitset_storage_for((CCC_Bit[10]){})
    );
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    size_t i = 0;
    for (; i < CCC_flat_bitset_capacity(&bs).count; ++i) {
        check(CCC_flat_bitset_test(&bs, i), CCC_FALSE);
        check(CCC_flat_bitset_test(&bs, i), CCC_FALSE);
    }
    check(i, 10);
    check_end();
}

check_static_begin(flat_bitset_test_construct_with_literal) {
    CCC_Flat_bitset bs = CCC_flat_bitset_with_storage(10, (CCC_Bit[10]){});
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    size_t i = 0;
    for (; i < CCC_flat_bitset_count(&bs).count; ++i) {
        check(CCC_flat_bitset_test(&bs, i), CCC_FALSE);
        check(CCC_flat_bitset_test(&bs, i), CCC_FALSE);
    }
    check(i, 10);
    check_end();
}

check_static_begin(flat_bitset_test_construct_with_literal_full_block) {
    CCC_Flat_bitset bs = CCC_flat_bitset_with_storage(
        CCC_FLAT_BITSET_BLOCK_BITS, (CCC_Bit[CCC_FLAT_BITSET_BLOCK_BITS]){}
    );
    check(CCC_flat_bitset_popcount(&bs).count, 0);
    size_t i = 0;
    for (; i < CCC_flat_bitset_count(&bs).count; ++i) {
        check(CCC_flat_bitset_test(&bs, i), CCC_FALSE);
        check(CCC_flat_bitset_test(&bs, i), CCC_FALSE);
    }
    check(i, CCC_FLAT_BITSET_BLOCK_BITS);
    check_end();
}

check_static_begin(flat_bitset_test_copy_no_allocate) {
    CCC_Flat_bitset source = CCC_flat_bitset_with_storage(0, (CCC_Bit[512]){});
    check(CCC_flat_bitset_capacity(&source).count, 512);
    check(CCC_flat_bitset_count(&source).count, 0);
    CCC_Result push_status = CCC_RESULT_OK;
    for (size_t i = 0; push_status == CCC_RESULT_OK; ++i) {
        if (i % 2) {
            push_status = CCC_flat_bitset_push_back(
                &source, CCC_TRUE, &(CCC_Allocator){}
            );
        } else {
            push_status = CCC_flat_bitset_push_back(
                &source, CCC_FALSE, &(CCC_Allocator){}
            );
        }
    }
    check(push_status, CCC_RESULT_NO_ALLOCATION_FUNCTION);
    CCC_Flat_bitset destination
        = CCC_flat_bitset_with_storage(0, (CCC_Bit[513]){});
    check(
        CCC_flat_bitset_copy(NULL, &source, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_bitset_copy(&destination, NULL, &(CCC_Allocator){}),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_bitset_copy(&destination, &source, NULL),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check(
        CCC_flat_bitset_copy(
            &destination, &CCC_flat_bitset_default(), &(CCC_Allocator){}
        ),
        CCC_RESULT_OK
    );
    CCC_Result r
        = CCC_flat_bitset_copy(&destination, &source, &(CCC_Allocator){});
    check(r, CCC_RESULT_OK);
    check(
        CCC_flat_bitset_popcount(&source).count,
        CCC_flat_bitset_popcount(&destination).count
    );
    check(
        CCC_flat_bitset_count(&source).count,
        CCC_flat_bitset_count(&destination).count
    );
    while (!CCC_flat_bitset_is_empty(&source)
           && !CCC_flat_bitset_is_empty(&destination)) {
        CCC_Tribool const source_msb = CCC_flat_bitset_pop_back(&source);
        CCC_Tribool const destination_msb
            = CCC_flat_bitset_pop_back(&destination);
        if (CCC_flat_bitset_count(&source).count % 2) {
            check(source_msb, CCC_TRUE);
            check(source_msb, destination_msb);
        } else {
            check(source_msb, CCC_FALSE);
            check(source_msb, destination_msb);
        }
    }
    check(
        CCC_flat_bitset_is_empty(&source),
        CCC_flat_bitset_is_empty(&destination)
    );
    check_end();
}

check_static_begin(flat_bitset_test_copy_allocate) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[1024]){})),
    };
    CCC_Flat_bitset source = CCC_flat_bitset_with_capacity(allocator, 512, 0);
    for (size_t i = 0; i < 512; ++i) {
        if (i % 2) {
            check(
                CCC_flat_bitset_push_back(&source, CCC_TRUE, &allocator),
                CCC_RESULT_OK
            );
        } else {
            check(
                CCC_flat_bitset_push_back(&source, CCC_FALSE, &allocator),
                CCC_RESULT_OK
            );
        }
    }
    CCC_Flat_bitset destination = CCC_flat_bitset_default();
    CCC_Result r = CCC_flat_bitset_copy(&destination, &source, &allocator);
    check(r, CCC_RESULT_OK);
    check(
        CCC_flat_bitset_popcount(&source).count,
        CCC_flat_bitset_popcount(&destination).count
    );
    check(
        CCC_flat_bitset_count(&source).count,
        CCC_flat_bitset_count(&destination).count
    );
    while (!CCC_flat_bitset_is_empty(&source)
           && !CCC_flat_bitset_is_empty(&destination)) {
        CCC_Tribool const source_msb = CCC_flat_bitset_pop_back(&source);
        CCC_Tribool const destination_msb
            = CCC_flat_bitset_pop_back(&destination);
        if (CCC_flat_bitset_count(&source).count % 2) {
            check(source_msb, CCC_TRUE);
            check(source_msb, destination_msb);
        } else {
            check(source_msb, CCC_FALSE);
            check(source_msb, destination_msb);
        }
    }
    check(
        CCC_flat_bitset_is_empty(&source),
        CCC_flat_bitset_is_empty(&destination)
    );
    check_end();
}

check_static_begin(flat_bitset_test_copy_exhaustion) {
    CCC_Allocator allocator = {
        .allocate = stack_allocator_allocate,
        .context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[512]){})),
    };
    CCC_Flat_bitset source = CCC_flat_bitset_with_capacity(allocator, 512, 0);
    CCC_Flat_bitset destination = CCC_flat_bitset_default();
    check(
        CCC_flat_bitset_copy(&destination, &source, &allocator),
        CCC_RESULT_ALLOCATOR_ERROR
    );
    allocator.context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[1024]){}));
    source.blocks = NULL;
    check(
        CCC_flat_bitset_copy(&destination, &source, &allocator),
        CCC_RESULT_ARGUMENT_ERROR
    );
    check_end();
}

check_static_begin(flat_bitset_test_init_from) {
    SV_Str_view const input = SV_from("110110");
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[256]){})),
    };
    CCC_Flat_bitset b = CCC_flat_bitset_from(
        allocator, 0, SV_len(input), '1', SV_begin(input)
    );
    check(CCC_flat_bitset_count(&b).count, SV_len(input));
    check(CCC_flat_bitset_capacity(&b).count >= SV_len(input), CCC_TRUE);
    check(CCC_flat_bitset_popcount(&b).count, 4);
    check(CCC_flat_bitset_test(&b, 0), CCC_TRUE);
    check(CCC_flat_bitset_test(&b, SV_len(input) - 1), CCC_FALSE);
    check_end();
}

check_static_begin(flat_bitset_test_init_from_cap) {
    SV_Str_view input = SV_from("110110");
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[256]){})),
    };
    CCC_Flat_bitset b = CCC_flat_bitset_from(
        allocator, 0, SV_len(input), '1', SV_begin(input), SV_len(input) * 2
    );
    check(CCC_flat_bitset_count(&b).count, SV_len(input));
    check(CCC_flat_bitset_capacity(&b).count >= (SV_len(input)) * 2, CCC_TRUE);
    check(CCC_flat_bitset_popcount(&b).count, 4);
    check(CCC_flat_bitset_test(&b, 0), CCC_TRUE);
    check(CCC_flat_bitset_test(&b, SV_len(input) - 1), CCC_FALSE);
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, SV_len(input)));
    check(CCC_flat_bitset_push_back(&b, CCC_TRUE, &allocator), CCC_RESULT_OK);
    check(CCC_TRUE, CCC_flat_bitset_test(&b, SV_len(input)));
    check(CCC_flat_bitset_capacity(&b).count >= (SV_len(input)) * 2, CCC_TRUE);
    check_end();
}

check_static_begin(flat_bitset_test_init_from_fail) {
    SV_Str_view input = SV_from("110110");
    /* Forgot allocation function. */
    CCC_Flat_bitset b = CCC_flat_bitset_from(
        (CCC_Allocator){}, 0, SV_len(input), '1', SV_begin(input)
    );
    check(CCC_flat_bitset_count(&b).count, 0);
    check(CCC_flat_bitset_capacity(&b).count, 0);
    check(CCC_flat_bitset_popcount(&b).count, 0);
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, 0));
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, 99));
    check_end(CCC_flat_bitset_clear_and_free(&b, &(CCC_Allocator){}););
}

check_static_begin(flat_bitset_test_init_from_cap_fail) {
    SV_Str_view input = SV_from("110110");
    /* Forgot allocation function. */
    CCC_Flat_bitset b = CCC_flat_bitset_from(
        (CCC_Allocator){}, 0, SV_len(input), '1', SV_begin(input), 99
    );
    check(CCC_flat_bitset_count(&b).count, 0);
    check(CCC_flat_bitset_capacity(&b).count, 0);
    check(CCC_flat_bitset_popcount(&b).count, 0);
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, 0));
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, 99));
    check_end(CCC_flat_bitset_clear_and_free(&b, &(CCC_Allocator){}););
}

check_static_begin(flat_bitset_test_init_with_capacity) {
    CCC_Allocator const allocator = {
        .allocate = stack_allocator_allocate,
        .context
        = &stack_allocator_for(CCC_flat_bitset_storage_for((CCC_Bit[10]){})),
    };
    CCC_Flat_bitset b = CCC_flat_bitset_with_capacity(allocator, 10);
    check(CCC_flat_bitset_popcount(&b).count, 0);
    check(CCC_flat_bitset_set(&b, 0, CCC_TRUE), CCC_FALSE);
    check(CCC_flat_bitset_set(&b, 9, CCC_TRUE), CCC_FALSE);
    check(CCC_flat_bitset_test(&b, 0), CCC_TRUE);
    check(CCC_flat_bitset_test(&b, 9), CCC_TRUE);
    check_end();
}

check_static_begin(flat_bitset_test_init_with_capacity_fail) {
    CCC_Flat_bitset b = CCC_flat_bitset_with_capacity((CCC_Allocator){}, 10);
    check(CCC_flat_bitset_popcount(&b).count, 0);
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_set(&b, 0, CCC_TRUE));
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_set(&b, 9, CCC_TRUE));
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, 0));
    check(CCC_TRIBOOL_ERROR, CCC_flat_bitset_test(&b, 9));
    check_end(CCC_flat_bitset_clear_and_free(&b, &(CCC_Allocator){}););
}

int
main(void) {
    return check_run(
        flat_bitset_test_static(),
        flat_bitset_test_construct(),
        flat_bitset_test_construct_with_literal(),
        flat_bitset_test_construct_with_literal_full_block(),
        flat_bitset_test_copy_no_allocate(),
        flat_bitset_test_copy_allocate(),
        flat_bitset_test_copy_exhaustion(),
        flat_bitset_test_init_from(),
        flat_bitset_test_init_from_cap(),
        flat_bitset_test_init_from_fail(),
        flat_bitset_test_init_from_cap_fail(),
        flat_bitset_test_init_with_capacity(),
        flat_bitset_test_init_with_capacity_fail()
    );
}
