#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define FLAT_BUFFER_USING_NAMESPACE_CCC

#include "ccc/flat_buffer.h"
#include "ccc/types.h"
#include "checkers.h"
#include "flat_buffer_utility.h"

check_static_begin(flat_buffer_test_iterate_empty) {
    Flat_buffer const b = flat_buffer_default(int);
    size_t count = 0;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        ++count;
    }
    check(count, 0);
    for (int const *i = flat_buffer_reverse_begin(&b);
         i != flat_buffer_reverse_end(&b);
         i = flat_buffer_reverse_next(&b, i)) {
        ++count;
    }
    check(count, 0);
    check_end();
}

check_static_begin(flat_buffer_test_iterator_forward) {
    Flat_buffer const b
        = flat_buffer_with_storage(6, (int[6]){1, 2, 3, 4, 5, 6});
    size_t count = 0;
    int prev = 0;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        check(*i > prev, CCC_TRUE);
        ++count;
    }
    check(count, 6);
    check_end();
}

check_static_begin(flat_buffer_test_iterator_forward_count_vs_capacity) {
    Flat_buffer const b
        = flat_buffer_with_storage(3, (int[6]){1, 2, 3, 4, 5, 6});
    size_t count = 0;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        ++count;
    }
    check(count, 3);
    check_end();
}

check_static_begin(flat_buffer_test_iterator_reverse) {
    Flat_buffer const b
        = flat_buffer_with_storage(6, (int[6]){1, 2, 3, 4, 5, 6});
    size_t count = 0;
    int prev = 7;
    for (int const *i = flat_buffer_reverse_begin(&b);
         i != flat_buffer_reverse_end(&b);
         i = flat_buffer_reverse_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        check(*i < prev, CCC_TRUE);
        ++count;
    }
    check(count, 6);
    check_end();
}

check_static_begin(flat_buffer_test_reverse_buf) {
    Flat_buffer b = flat_buffer_with_storage(6, (int[6]){1, 2, 3, 4, 5, 6});
    int prev = 0;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        check(*i > prev, CCC_TRUE);
    }
    for (int *l = flat_buffer_begin(&b), *r = flat_buffer_reverse_begin(&b);
         l < r;
         l = flat_buffer_next(&b, l), r = flat_buffer_reverse_next(&b, r)) {
        CCC_Result const res = flat_buffer_swap(
            &b,
            &(int){},
            flat_buffer_index(&b, l).count,
            flat_buffer_index(&b, r).count
        );
        check(res, CCC_RESULT_OK);
    }
    prev = 7;
    for (int const *i = flat_buffer_begin(&b); i != flat_buffer_end(&b);
         i = flat_buffer_next(&b, i)) {
        check(i != NULL, CCC_TRUE);
        check(*i < prev, CCC_TRUE);
    }
    check_end();
}

/** The concept of two pointers can be implemented quite cleanly with the buffer
iterator abstraction, especially because we don't force a foreach macro use
onto the user. They are able to set up a while/for loop freely. */
check_static_begin(flat_buffer_test_trap_rainwater_two_pointers) {
    enum : size_t {
        HCAP = 12,
    };
    Flat_buffer const heights = flat_buffer_with_storage(
        HCAP, (int[HCAP]){0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1}
    );
    int const correct_trapped = 6;
    int trapped = 0;
    check(flat_buffer_is_empty(&heights), CCC_FALSE);
    int lpeak = *flat_buffer_front_as(&heights, int);
    int rpeak = *flat_buffer_back_as(&heights, int);
    /* Easy way to have a "skip first" iterator because the iterator is
       returned from each iterator function. */
    int const *l = flat_buffer_next(&heights, flat_buffer_begin(&heights));
    int const *r = flat_buffer_reverse_next(
        &heights, flat_buffer_reverse_begin(&heights)
    );
    while (l <= r) {
        if (lpeak < rpeak) {
            lpeak = maxint(lpeak, *l);
            trapped += (lpeak - *l);
            l = flat_buffer_next(&heights, l);
        } else {
            rpeak = maxint(rpeak, *r);
            trapped += (rpeak - *r);
            r = flat_buffer_reverse_next(&heights, r);
        }
    }
    check(trapped, correct_trapped);
    check_end();
}

int
main(void) {
    return check_run(
        flat_buffer_test_iterate_empty(),
        flat_buffer_test_iterator_forward(),
        flat_buffer_test_iterator_forward_count_vs_capacity(),
        flat_buffer_test_iterator_reverse(),
        flat_buffer_test_reverse_buf(),
        flat_buffer_test_trap_rainwater_two_pointers()
    );
}
