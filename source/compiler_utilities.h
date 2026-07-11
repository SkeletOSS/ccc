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
/** @internal
@file
@brief The Compiler Utilities Available to All Containers

This file is an internal private header that allows containers to use low level
compiler built-in functionality or extremely common helper functions. It is not
user facing and will not be exported as an interface. This file includes
detection of compiler built-ins such as bit utilities. It attempts to use bit
utilities from the new `__builtin_stdc_*` family when available as these are at
the cutting edge of C23's new `stdbit.h` standard. However, sane fallback
implementations are always provided. See the documented interface for all
available macros and functions. */
#ifndef CCC_COMPILER_UTILITIES_H
#define CCC_COMPILER_UTILITIES_H

/** @cond */
#include <limits.h>
#include <stdbool.h>
#include <stdckdint.h>
#include <stddef.h>
#include <stdint.h>
/** @endcond */

#include "private/private_compiler_utilities.h"

/** @internal
@name Macro Preprocessor Definitions
Compiler built ins for code manipulation. */
/**@{*/

/** @internal
@brief Force a function to be static inline, if available via compiler
directives. */
#define CCC_INLINE CCC_PRIVATE_INLINE

/** @internal
@brief Mark an expression as likely for compiler optimizations.
@param[in] expression the expression in code to mark. */
#define CCC_likely(expression) CCC_private_likely(expression)

/** @internal
@brief Mark an expression as unlikely for compiler optimizations.
@param[in] expression the expression in code to mark. */
#define CCC_unlikely(expression) CCC_private_unlikely(expression)

/**@}*/

/** @name Arithmetic
Type safe arithmetic functions. */
/**@{*/

/** @internal
@brief Round an integer up to a specified alignment boundary safely
evaluating the integer argument once. This function-style macro can be used at
compile time or runtime.
@param[in] integer the integer value to round up.
@param[in] alignment the alignment boundary that must be a power of two.
@return The rounded-up value, evaluated in the wider type of the two input
arguments.
@note Argument promotion to the wider type ensures that smaller integer types
are not accidentally truncated during alignment arithmetic.
@warning The alignment argument is evaluated twice to support compile time use
of this macro. Ensure the alignment expression does not have side-effects. This
should be easy as it is uncommon to determine alignment from an expression with
side-effects. */
#define CCC_roundup(integer, alignment) CCC_private_roundup(integer, alignment)

/** @internal
@brief Check for overflow while rounding an integer up to an alignment
boundary using C23 checked arithmetic.
@param[out] result_pointer pointer to a variable where the rounded result will
be stored.
@param[in] integer the integer value to round up.
@param[in] alignment the alignment boundary (must be a power of two).
@return true if an overflow occurred during rounding or if the result cannot
fit into the type pointed to by result_pointer; false otherwise.
@note This variant is only available at runtime. */
#define CCC_checked_roundup(result_pointer, integer, alignment)                \
    CCC_private_checked_roundup(result_pointer, integer, alignment)

/** @internal
@brief Returns the log base 2 of the provided integer at compile or runtime.
@param[in] integer the integer, up to 64 bits in width, of which to find the
log.
@return the unsigned count of the log base 2 of the provided integer.
@warning This macro accepts a maximum integer width of 64, otherwise bit
truncation will occur. */
#define CCC_log2(integer) CCC_private_log2(integer)

/**@}*/

/** @name Bit Manipulation
Type safe bit manipulation functions. */
/**@{*/

/** @internal
@brief Count the number of leading zero bits in an unsigned integer.
@param[in] integer the unsigned integer value to inspect.
@return The unsigned count of leading zeros. If the input is zero, the bit width
count of the input type is returned.
@note Behavior is well-defined for input 0. */
#define CCC_count_leading_zeros(integer)                                       \
    CCC_private_count_leading_zeros(integer)

/** @internal
@brief Count the number of trailing zero bits in an unsigned integer.
@param[in] integer the unsigned integer value to inspect.
@return The unsigned count of trailing zeros. If the input is zero, the bit
width count of the input type is returned.
@note Behavior is well-defined for input 0. */
#define CCC_count_trailing_zeros(integer)                                      \
    CCC_private_count_trailing_zeros(integer)

/** @internal
@brief Count the total number of set 1 bits in an unsigned integer.
@param[in] integer the unsigned integer value to inspect.
@return The unsigned count of bits set to the value 1. */
#define CCC_popcount(integer) CCC_private_popcount(integer)

/** @internal
@brief Find the next closest power of 2 ceiling for integer.
@param[in] integer the unsigned integer value to raise to its ceiling.
@return The smallest power of two greater than or equal to the given value, of
the same type as the input argument. If the next power of two overflows the
maximum representable value of the input type, 0 is returned to indicate
overflow failure.
@note Inputting 0 returns 1, as 1 (2^0) is the first power of two. Because 0
is not a valid power of two, it safely serves as an overflow sentinel. */
#define CCC_bit_ceiling(integer) CCC_private_bit_ceiling(integer)

/**@}*/

/** @name Comparison
Type safe comparison functions. */
/**@{*/

/** @internal
@brief Evaluate the minimum of two values, safely ensuring that both types
match in sign. Each argument is evaluated once to avoid side-effects.
@param[in] left the first value to compare.
@param[in] right the second value to compare.
@return The minimum between left and right. */
#define CCC_min(left, right) CCC_private_min(left, right)

/** @internal
@brief Evaluate the maximum of two values, safely ensuring that both types
match in sign. Each argument is evaluated once to avoid side-effects.
@param[in] left the first value to compare.
@param[in] right the second value to compare.
@return The maximum between left and right. */
#define CCC_max(left, right) CCC_private_max(left, right)

/**@}*/

#endif /* CCC_COMPILER_UTILITIES_H */
