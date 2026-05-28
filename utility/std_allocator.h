#ifndef CCC_STD_ALLOCATOR_H
#define CCC_STD_ALLOCATOR_H
#include <stddef.h>
#include <types.h>

/** @brief A wrapper around the standard library allocator that accommodates
user alignment requests.
@param[in] arguments the arguments to the allocator function that are
interpreted as follows.
- If input is NULL and bytes 0, NULL is returned.
- If input is NULL with non-zero bytes, at least bytes of new memory are
  allocated and returned aligned to the requested alignment.
- If input is non-NULL it is previously allocated by the Allocator_interface and
  alignment is greater than or equal to the previously requested alignment.
- If input is non-NULL with non-zero size, input is resized to at least bytes
  capacity. The pointer returned is NULL if resizing fails. Upon success, the
  pointer returned might not be equal to the pointer provided. The returned
  allocation is aligned to the requested alignment.
- If input is non-NULL and size is 0, input is freed and NULL is returned.
@return a pointer according to the semantics of the input arguments.
@note This allocator does not zero out allocated bytes. */
void *std_aligned_allocate(CCC_Allocator_arguments arguments);

/** @brief A convenience wrapper for the standard library allocator. This can
be passed by reference anywhere CCC_Allocator is requested. It accommodates
alignment requests and is therefore slightly slower and less space efficient
than if a custom allocator implemented aligned_alloc, aligned_realloc, and
aligned_free. This simply wraps aligned_alloc and manually manages internal
logic necessary for resizing to respect alignment. */
extern CCC_Allocator const std_allocator;

#endif /* CCC_STD_ALLOCATOR_H */
