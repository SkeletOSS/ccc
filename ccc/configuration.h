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
/** @file
@brief The C Container Collection Configuration Header. For full download and
install instructions using a user configuration see INSTALL.md.

The C Container Collection uses the following functions or macros that must be
supported by the user on freestanding targets.

Traditionally included via `<string.h>`:

- `memcpy()`
- `memmove()`
- `memset()`
- `memcmp()`

Traditionally included via `<assert.h>`:

- `assert()`

To provide these functions, the user may create a header. For example,
`my_ccc_configuration.h`. In this header the user has two options: provide the
listed functions directly or include their versions of the headers that provide
the needed functionality. It is common for freestanding environments to provide
their own `<string.h>` and `<assert.h>` that implement these functions. Then,
define `CCC_USER_CONFIGURATION="my_ccc_configuration.h"` at some point in the
build steps. Either provide it in a CMakeLists.txt file, in a CMakePresets.json
file, or as a command line argument. The C Container Collection is then fully
configured for a freestanding environment. See INSTALL.md for the full download
and build instructions.

Any other headers such as `<stdint.h>`, `<stddef.h>`, etc. that are included
directly in source code now, or will be in the future, are those provided by the
C23 standard on freestanding targets.

See the flat hash map documentation for how to select the optimal Single
Instruction Multiple Data or Single Register Multiple Data implementation. Even
on freestanding targets, the compiler intrinsics used on x86-64 and ARM NEON
platforms should be available. However, `CCC_FLAT_HASH_MAP_PORTABLE` is an
available directive for users that need to force a portable fallback
implementation when such compiler platform intrinsics are not available. See
`ccc/flat_hash_map.h` for more. */
#ifndef CCC_PRIVATE_HOSTED_VS_FREESTANDING_CONFIGURATION_H
#define CCC_PRIVATE_HOSTED_VS_FREESTANDING_CONFIGURATION_H

#ifdef CCC_USER_CONFIGURATION
#    include CCC_USER_CONFIGURATION
#else
#    include <assert.h> /* IWYU pragma: export */
#    include <string.h> /* IWYU pragma: export */
#endif

#endif /* CCC_PRIVATE_HOSTED_VS_FREESTANDING_CONFIGURATION_H */
