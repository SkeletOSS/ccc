#ifndef CCC_RANDOM_H
#define CCC_RANDOM_H

#include <stddef.h>

/** Provides an integer within range [min, max]. The user must seed the global
random function from the standard library before calling. */
int rand_range(int min, int max);

/** Shuffles n elems of elem_sz randomly. User must provide a pointer to temp
storage that is elem_sz bytes large. User must seed random before calling
this function (e.g. srand((unsigned)time(NULL));). */
void rand_shuffle(size_t elem_size, void *elems, size_t n, void *temp);

/** Fills an array of integers with values increasing from start_val. If adding
n values to array will exceed the max value of an integer, the value wraps. */
void iota(int *array, size_t n, unsigned start_val);

#endif /* CCC_RANDOM_H */
