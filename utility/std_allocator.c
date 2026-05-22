#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "std_allocator.h"

#include "ccc/types.h"

#include "ccc/flat_hash_map.h" /* IWYU pragma: keep */
static_assert(
    alignof(max_align_t) >= CCC_FLAT_HASH_MAP_GROUP_COUNT,
    "Standard library malloc and realloc default maximum alignment is "
    "sufficient for flat hash map SIMD group loads."
);

/** Defined extern in allocate.h */
CCC_Allocator const std_allocator = {
    .allocate = std_allocate,
};

void *
std_allocate(CCC_Allocator_arguments const arguments) {
    if (arguments.alignment && arguments.alignment > alignof(max_align_t)) {
        assert(
            arguments.alignment <= alignof(max_align_t)
            && "Any type provided to this allocator has alignment less than or "
               "equal to default malloc/realloc max alignment."
        );
        return NULL;
    }
    if (!arguments.input && !arguments.bytes) {
        return NULL;
    }
    if (!arguments.input) {
        return malloc(arguments.bytes);
    }
    if (!arguments.bytes) {
        free(arguments.input);
        return NULL;
    }
    return realloc(arguments.input, arguments.bytes);
}
