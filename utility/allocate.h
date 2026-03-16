#ifndef CCC_ALLOC_H
#define CCC_ALLOC_H
#include <stddef.h>
#include <types.h>

/** @brief Standard allocator using platform malloc, realloc, and free.
@param[in] arguments the arguments to the appropriate allocator function.
@return the result of the operation according to the following CCC_Allocator
semantics.

- If input is NULL and bytes 0, NULL is returned.
- If input is NULL with non-zero bytes, new memory is allocated/returned.
- If input is non-NULL it has been previously allocated by the Allocator.
- If input is non-NULL with non-zero size, input is resized to at least bytes
  size. The pointer returned is NULL if resizing fails. Upon success, the
  pointer returned might not be equal to the pointer provided.
- If input is non-NULL and size is 0, input is freed and NULL is returned. */
void *std_allocate(CCC_Allocator_arguments arguments);

/** @brief A convenience wrapper for the standard library allocator. This can
be passed by reference anywhere CCC_Allocator_context is requested. */
extern CCC_Allocator_context const std_allocator;

#endif /* CCC_ALLOC_H */
