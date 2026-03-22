#ifndef CCC_CONFIGURATION_H
#define CCC_CONFIGURATION_H

/** The C Container Collection uses the following functions or macros that
must be supported by the user on freestanding targets.

Traditionally included via <string.h>:

- memcpy()
- memmove()
- memset()
- memcmp()

Traditionally included via <assert.h>:

- assert()

Any other headers such as <stdint.h>, <stddef.h>, etc. that are included
directly in source code now, or will be in the future, are those provided by the
C23 standard on freestanding targets. */
#ifdef CCC_USER_CONFIGURATION
#    include CCC_USER_CONFIGURATION
#else
#    include <assert.h> /* IWYU pragma: export */
#    include <string.h> /* IWYU pragma: export */
#endif

#endif /* CCC_CONFIGURATION_H */
