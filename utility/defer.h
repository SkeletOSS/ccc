/** A header to provide the long awaited and upcoming defer C language feature.
This is a game changer for C. Try to avoid using it in any core CCC code until
it has some time to mature. For now it can be used in samples and tests. Tests,
samples, and utilities are not included in the release packages to more users
with older compilers can build the container library. But for my own code that
tests and uses the containers I will happily use defer. */
#ifndef CCC_DEFER_H
#define CCC_DEFER_H
#if __has_include(<stddefer.h>)
#    include <stddefer.h> /* IWYU pragma: export */
#elif defined(__GNUC__) || defined(__clang__)
#    define defer _Defer
#    define _Defer _Defer_A(__COUNTER__)
#    define _Defer_A(N) _Defer_B(N)
#    define _Defer_B(N) _Defer_C(_Defer_func_##N, _Defer_var_##N)
#    define _Defer_C(F, V)                                                     \
        auto void F(int *);                                                    \
        __attribute__((__cleanup__(F), __deprecated__, __unused__)) int V;     \
        __attribute__((__always_inline__, __deprecated__,                      \
                       __unused__)) inline auto void                           \
        F(__attribute__((__unused__)) int *)
#else
#    error "The _Defer feature seems not available"
#endif
#endif /* CCC_DEFER_H */
