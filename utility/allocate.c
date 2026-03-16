#include <stddef.h>
#include <stdlib.h>

#include "allocate.h"
#include "types.h"

/** Defined extern in allocate.h */
CCC_Allocator_context const std_allocator = {
    .allocate = std_allocate,
};

void *
std_allocate(CCC_Allocator_arguments const arguments) {
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
