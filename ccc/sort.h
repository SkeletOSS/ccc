#ifndef CCC_SORT_H
#define CCC_SORT_H

#include "buffer.h"
#include "doubly_linked_list.h"
#include "singly_linked_list.h"
#include "types.h"

/** @name Sorting Algorithms
Various sorting algorithms for different containers in the collection. */
/**@{*/

/** @brief Sorts the input buffer in `O(N * log(N))` time and `O(1)` space
according to the desired input order.
@param[in] buffer the buffer to be modified and sorted in place.
@param[in] temp a pointer to a dummy user type that will be used for swapping.
@param[in] order the desired order of the sorted buffer. CCC_ORDER_LESSER places
elements in non-decreasing order starting from index `[0, N)`, where N is the
count of the buffer. CCC_ORDER_GREATER places elements in non-increasing order
starting from index `[0, N)`, where N is the count of the buffer. This is done
to remain consistent with heap order, where CCC_ORDER_LESSER places the minimum
element at index 0 and CCC_ORDER_GREATER places the maximum element at index 0
because 0 is the root of the tree in either heap ordering.
@param[in] comparator the comparator context for comparing buffer elements.
@return the result of the sorting operation. If an argument input error occurs
an input error result is provided. Allocation is not needed to sort elements in
the buffer so memory related errors are not possible if the provided buffer is
initialized correctly.
@warning Assumes the input buffer has been correctly initialized via its
interface.

An easy way to provide a temp slot is with an anonymous compound literal such
as `&(My_type){}`, passed directly as an argument.

The sort is not inherently stable and uses the provided comparison function to
order the elements. */
CCC_Result CCC_sort_heapsort(CCC_Buffer *buffer, void *temp, CCC_Order order,
                             CCC_Type_comparator_context const *comparator);

/** @brief Sorts the doubly linked list in non-decreasing order as defined by
the provided comparison function. `O(N * log(N))` time, `O(1)` space.
@param[in] list a pointer to the doubly linked list to sort.
@param[in] order the desired order of the sorted list. CCC_ORDER_LESSER places
elements in non-decreasing order starting from index `[0, N)`, where N is the
count of the list. CCC_ORDER_GREATER places elements in non-increasing order
starting from index `[0, N)`, where N is the count of the list. This is done
to remain consistent with heap order, where CCC_ORDER_LESSER places the minimum
element at index 0 and CCC_ORDER_GREATER places the maximum element at index 0
because 0 is the first node in the list.
@param[in] comparator the comparator context for comparing list elements.
@return the result of the sort, usually OK. An arg error if doubly_linked_list
is null. */
CCC_Result CCC_sort_doubly_linked_list_mergesort(
    CCC_Doubly_linked_list *list, CCC_Order order,
    CCC_Type_comparator_context const *comparator);

/** @brief Sorts the singly linked list in non-decreasing order as defined by
the provided comparison function. `O(N * log(N))` time, `O(1)` space.
@param[in] list a pointer to the singly linked list to sort.
@param[in] order the desired order of the sorted list. CCC_ORDER_LESSER places
elements in non-decreasing order starting from index `[0, N)`, where N is the
count of the list. CCC_ORDER_GREATER places elements in non-increasing order
starting from index `[0, N)`, where N is the count of the list. This is done
to remain consistent with heap order, where CCC_ORDER_LESSER places the minimum
element at index 0 and CCC_ORDER_GREATER places the maximum element at index 0
because 0 is the first node in the list.
@param[in] comparator the comparator context for comparing list elements.
@return the result of the sort, usually OK. An arg error if singly_linked_list
is null. */
CCC_Result CCC_sort_singly_linked_list_mergesort(
    CCC_Singly_linked_list *list, CCC_Order order,
    CCC_Type_comparator_context const *comparator);

/** @brief Sorts the list in non-decreasing order as defined by
the provided comparison function. `O(N * log(N))` time, `O(1)` space.
@param[in] list a pointer to the list to sort.
@param[in] order the desired order of the sorted list. CCC_ORDER_LESSER places
elements in non-decreasing order starting from index `[0, N)`, where N is the
count of the list. CCC_ORDER_GREATER places elements in non-increasing order
starting from index `[0, N)`, where N is the count of the list. This is done
to remain consistent with heap order, where CCC_ORDER_LESSER places the minimum
element at index 0 and CCC_ORDER_GREATER places the maximum element at index 0
because 0 is the first node in the list.
@param[in] comparator_pointer the pointer to the comparator context for
comparing list elements.
@return the result of the sort, usually OK. An arg error if list is null. */
#define CCC_sort_mergesort(list_pointer, order, comparator_pointer)            \
    _Generic((list_pointer),                                                   \
        CCC_Singly_linked_list *: CCC_sort_singly_linked_list_mergesort,       \
        CCC_Doubly_linked_list *: CCC_sort_doubly_linked_list_mergesort)(      \
        list_pointer, order, comparator_pointer)

/**@}*/

/** Define this preprocessor directive if shorter names are helpful. Ensure
 no namespace clashes occur before shortening. */
#ifdef SORT_USING_NAMESPACE_CCC
/* NOLINTBEGIN(readability-identifier-naming) */
#    define sort_heapsort(args...) CCC_sort_heapsort(args)
#    define sort_singly_linked_list_mergesort(args...)                         \
        CCC_sort_singly_linked_list_mergesort(args)
#    define sort_doubly_linked_list_mergesort(args...)                         \
        CCC_sort_doubly_linked_list_mergesort(args)
#    define sort_mergesort(args...) CCC_sort_mergesort(args)
/* NOLINTEND(readability-identifier-naming) */
#endif /* SORT_USING_NAMESPACE_CCC */

#endif /* CCC_SORT_H */
