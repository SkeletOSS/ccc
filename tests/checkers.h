/** @file The checkers test harness. Various macros are available to create
test functions and check expected behavior.
@note This test harness holds a global random seed used to seed `srand` on
behalf of the user. This means the user can use functions from the standard
library for randomness directly with out seeding as long as this file is
included. This is important for immediately reproducible failures. The random
seed is reported along with any failing check for a test program that uses this
file. */
#ifndef CHECKERS_H
#define CHECKERS_H

#include <stdint.h>
#include <stdlib.h> /* IWYU pragma: keep */
#include <time.h>   /* IWYU pragma: keep */

#define CHECK_RED "\033[38;5;9m"
#define CHECK_GREEN "\033[38;5;10m"
#define CHECK_CYAN "\033[38;5;14m"
#define CHECK_NONE "\033[0m"

/** POSIX style return mechanism with passing as 0 and failing as 1. Errors
are negative 1 and can occur on some rare occasions or if the user wants to
provide this value to the test runner manually. */
enum Check_result {
    CHECK_ERROR = -1,
    CHECK_PASS,
    CHECK_FAIL,
};

/** To pass the correct byte information to the print function in the C file
we must portably, and according to the C23 standard, provide the bytes of any
possible type comparison that our checkers perform. The two possibilities that
cover all types are those in this union. Any address or any integer value. We
will copy the bytes of any integer into the widest possible type or we will
copy a pointer to any object as its well defined integer equivalent. We
will then be able to print the expression in the check provided by the user
and the byte value output. */
union Check_bytes {
    /** The accepting field of any object pointer. */
    uintptr_t as_address;
    /** The accepting field of any integer width. */
    uintmax_t as_bytes;
};

/** Internal printer to save on binary size and speed up compilation. We have
one c file translation unit where all calls to printing functions exist and the
necessary branch to check for the type of bytes we are printing. This way static
analysis and sanitizer instrumentation is rapidly reduced when compared to
thousands of inline print statements. Builds and tests should be faster. */
void check_print_fail_message(
    char const *function_string,
    int line,
    char const *result_expression_string,
    char const *expected_expression_string,
    union Check_bytes result_output_bytes,
    union Check_bytes expected_output_bytes,
    bool is_address,
    unsigned random_seed
);

/** The global variable seed for the entire test checker harness. This is
seeded before any tests run in the check_run macro. */
extern unsigned check_random_seed;

/** Provides the correct type to the union for a check expression while
silencing compiler warnings for non-compiled generic branches. Substitution
failure is an error in C, unlike C++. The casting of an integer to uintptr_t
in the non-compiled void branch could cut off bytes according to the standard
but that generic case would never get compiled anyway. */
#define check_to_bytes(x)                                                      \
    _Generic(                                                                  \
        (x),                                                                   \
        void *: (union Check_bytes){.as_address = (uintptr_t)(x)},             \
        void const *: (union Check_bytes){.as_address = (uintptr_t)(x)},       \
        default: (union Check_bytes){.as_bytes = (uintmax_t)(x)}               \
    )

/** Returns the bool flag needed to correctly print the test result in the
printing function. */
#define check_is_address(x)                                                    \
    _Generic((x), void *: 1, void const *: 1, default: 0)

#define check_non_check_default_params(...) __VA_ARGS__
#define check_default_params(...) void
#define check_optional_params(...)                                             \
    __VA_OPT__(check_non_)##check_default_params(__VA_ARGS__)

/** @brief Define a static test function that has a name and optionally
additional parameters that one may wish to define for a test function.
@param[in] test_name the name of the static function.
@param[in] ... any additional parameters required for the function.
@return see the end test macro. This will return a enum Check_result that is
CHECK_PASS or CHECK_FAIL to be handled as the user sees fit.

It is possible to return early from a test before the end test macro, but it
is discouraged, especially if any memory allocations need to be cleaned up if
a test fails.

Example with no additional parameters:

check_static_begin(fhash_test_empty)
{
    struct Val vals[5] = {};
    ccc_flat_hash_map fh;
    ccc_result const res
        = fhm_init(&fh, vals, sizeof(vals) / sizeof(vals[0]), struct Val, id, e,
                   NULL, fhash_int_zero, fhash_id_eq, NULL);
    check(is_empty(&fh), true);
    check_end();
}

Example with multiple parameters:

enum Check_result insert_shuffled(ccc_ordered_multimap *,
                                 struct Val[], size_t, int);

check_static_begin(insert_shuffled, ccc_ordered_multimap *pq,
                      struct Val vals[], size_t const size,
                      int const larger_prime)
{
    for (int i = 0 shuffled_index = larger_prime % size; i < size; ++i)
    {
        vals[shuffled_index].val = shuffled_index;
        push(pq, &vals[shuffled_index].elem);
        check(ccc_omm_validate(pq), true);
        shuffled_index = (shuffled_index + larger_prime) % size;
    }
    check_end();
}*/
#define check_static_begin(test_name, ...)                                     \
    static enum Check_result(test_name)(check_optional_params(__VA_ARGS__)) {  \
        enum Check_result check_private_macro_res = CHECK_PASS;

/** @brief Define a test function that has a name and optionally
additional parameters that one may wish to define a test function.
@param[in] test_name the name of the function.
@param[in] ... any additional parameters required for the function.
@return see the end test macro. This will return a enum Check_result.

It is possible to return early froa test before the end test macro, but it
is discouraged, especially if any memory allocations need to be cleaned up if
a test fails. See begin static test for examples. */
#define check_begin(test_name, ...)                                            \
    enum Check_result(test_name)(check_optional_params(__VA_ARGS__)) {         \
        enum Check_result check_private_macro_res = CHECK_PASS;

/** @brief execute a check within the context of a test.
@param[in] test_result the result value of some action.
@param[in] test_expected the expection of what the result is equal to.
@param[in] ... any additional function call that should execute on failure
that cannot be performed by the end test macro.
@note The check macro also reports the seed used for `srand` in case this test
relies on randomness. This way the user can reproduce the failed result.

test_result and test_expected should be comparable with ==/!=. If a test
passes nothing happens. If a test fails output shows the failure. Upon
failure any cleanup function specified by the end test macro will execute
to prevent memory leaks. See the end test macro for more. If the test end
macro will not have access to some memory or operation in the scope of the
current check one may provide an additional function call they wish to have
executed on failure. This should only be used in special cases where such a
function is unusable by the end test macro due to scoping. Even then, consider
tracking allocations in an array of some sort to be cleaned up upon failure
or success in the end test macro. Function calls should be a semicolon
seperated list of calls with their appropriate arguments. If more complex code
needs to be written in the cleanup case a code block can be surrounded by
{...any code goes here...} braces for the formatting assistance braces provide,
though the braces are not required. */
#define check(test_result, test_expected, ...)                                 \
    do {                                                                       \
        const __auto_type check_private_result = (test_result);                \
        const typeof(check_private_result) check_private_expected              \
            = (typeof(check_private_result))(test_expected);                   \
        if (check_private_result != check_private_expected) {                  \
            check_print_fail_message(                                          \
                __func__,                                                      \
                __LINE__,                                                      \
                #test_result,                                                  \
                #test_expected,                                                \
                check_to_bytes(check_private_result),                          \
                check_to_bytes(check_private_expected),                        \
                check_is_address(check_private_result),                        \
                check_random_seed                                              \
            );                                                                 \
            check_private_macro_res = CHECK_FAIL;                              \
            __VA_OPT__((void)(__extension__({__VA_ARGS__}));)                  \
            goto please_use_at_least_one_check_and_a_check_end_macro;          \
        }                                                                      \
    } while (0)

/** @brief execute a check within the context of a test that sets an error.
@param[in] test_result the result value of some action.
@param[in] test_expected the expection of what the result is equal to.
@param[in] ... any additional function call that should execute on failure
that cannot be performed by the end test macro.
@note The check macro also reports the seed used for `srand` in case this test
relies on randomness. This way the user can reproduce the failed result.

test_result and test_expected should be comparable with ==/!=. If a test
passes nothing happens. If a test fails output shows the failure and sets the
test status to an error rather than a failure. This should be used when a check
failure is the result of some operation not directly tied to the behavior being
tested. For example a failed system call would be a good reason to use this
check. Upon failure any cleanup function specified by the end test macro will
execute to prevent memory leaks. See the end test macro for more. If the test
end macro will not have access to some memory or operation in the scope of the
current check one may provide an additional function call they wish to have
executed on failure. This should only be used in special cases where such a
function is unusable by the end test macro due to scoping. Even then, consider
tracking allocations in an array of some sort to be cleaned up upon failure or
success in the end test macro. Function calls should be a semicolon seperated
list of calls with their appropriate arguments. If more complex code needs to be
written in the cleanup case a code block can be surrounded by
{...any code goes here...} braces for the formatting assistance braces provide,
though the braces are not required. */
#define check_error(test_result, test_expected, ...)                           \
    do {                                                                       \
        const __auto_type check_private_result = (test_result);                \
        const typeof(check_private_result) check_private_expected              \
            = (test_expected);                                                 \
        if (check_private_result != check_private_expected) {                  \
            check_print_fail_message(                                          \
                __func__,                                                      \
                __LINE__,                                                      \
                #test_result,                                                  \
                #test_expected,                                                \
                check_to_bytes(check_private_result),                          \
                check_to_bytes(check_private_expected),                        \
                check_is_address(check_private_result),                        \
                check_random_seed                                              \
            );                                                                 \
            check_private_macro_res = CHECK_ERROR;                             \
            __VA_OPT__((void)(__extension__({__VA_ARGS__}));)                  \
            goto please_use_at_least_one_check_and_a_check_end_macro;          \
        }                                                                      \
    } while (0)

/** Returns the current test status. If the end pass and end fail macros to
finish a test do not provide the necessary flow control for various test
scenarios, then check this status in the end test macro and act accordingly.

Example:
check_begin(my_test)
{
    ...test code ommited...
    check_end({
        switch(CHECK_STATUS)
        {
            case CHECK_PASS:
                ...action...
            break;
            case CHECK_FAIL:
                ...action...
            break;
            case CHECK_ERROR:
                ...action...
            break;
        }
    });
}

This is not recommended as complex tests should be completed such that one
of the end test macros is sufficient. */
#define CHECK_STATUS check_private_macro_res

/** @brief End every test started with the end test macro.
@param[in] ... optional code block with arbitrary cleanup code to be executed
to clean up any allocated memory upon test completion.
@return the test result from the currently executing test. CHECK_FAIL upon the
first check failure or CHECK_PASS if all checks have passed.

One may enter two braces and any code within them check_end({..code
here...}). This will help with formatting for more complicated cleanup
procedures. This end test macro is best to use when some code must execute every
time a test finishes whether it fails early or runs to completion. The code
written will execute whether all tests pass, or the first failure quits the test
early. Keep in mind that any function calls need to have access to variables
from earlier in the test if used. Standard scoping rules apply. This check
operates at function scope so does not have access to nested conditionals,
loops, or blocks from earlier in the test. See the check macro if more fine
grained control over nested scope is required upon a failure.*/
#define check_end(...)                                                         \
please_use_at_least_one_check_and_a_check_end_macro:                           \
    __VA_OPT__((void)(__extension__({__VA_ARGS__}));)                          \
    return check_private_macro_res;                                            \
    }

/** @brief End every test started with the end pass macro.
@param[in] ... optional code block with arbitrary code to be executed only upon
all tests passing.
@return the test result from the currently executing test. CHECK_FAIL upon the
first check failure or CHECK_PASS if all checks have passed.

One may enter two braces and any code within them check_pass_end({..code
here...}). This will help with formatting for more complicated cleanup
procedures. This end test macro is best to use when some code should only
execute upon all checks passing. Keep in mind that any function calls need to
have access to variables from earlier in the test if used. Standard scoping
rules apply. This check operates at function scope so does not have access to
nested conditionals, loops, or blocks from earlier in the test. See the check
macro if more fine grained control over nested scope is required upon a failure.
*/
#define check_pass_end(...)                                                    \
please_use_at_least_one_check_and_a_check_end_macro:                           \
    __VA_OPT__((void)(__extension__({                                          \
                   if (check_private_macro_res == CHECK_PASS) {                \
                       __VA_ARGS__                                             \
                   }                                                           \
               }));)                                                           \
    return check_private_macro_res;                                            \
    }

/** @brief End every test started with the end fail macros.
@param[in] ... optional code block with arbitrary cleanup code to be executed
only upon the first test failure.
@return the test result from the currently executing test. CHECK_FAIL upon the
first check failure or CHECK_PASS if all checks have passed.

One may enter two braces and any code within them check_fail_end({..code
here...}). This will help with formatting for more complicated cleanup
procedures. This end test macro is best to use when some code should only
execute upon the first test failure. Keep in mind that any function calls need
to have access to variables from earlier in the test if used. Standard scoping
rules apply. This check operates at function scope so does not have access to
nested conditionals, loops, or blocks from earlier in the test. See the check
macro if more fine grained control over nested scope is required upon a
failure.*/
#define check_fail_end(...)                                                    \
please_use_at_least_one_check_and_a_check_end_macro:                           \
    __VA_OPT__((void)(__extension__({                                          \
                   if (check_private_macro_res == CHECK_FAIL) {                \
                       __VA_ARGS__                                             \
                   }                                                           \
               }));)                                                           \
    return check_private_macro_res;                                            \
    }

/** @brief End every test started with the end error macro.
@param[in] ... optional code block with arbitrary cleanup code to be executed
only upon the first test failure.
@return the test result from the currently executing test. CHECK_FAIL upon the
first check failure or CHECK_PASS if all checks have passed.

One may enter two braces and any code within them check_error_end({..code
here...}). This will help with formatting for more complicated cleanup
procedures. This end test macro is best to use when some code should only
execute upon the first test error. Keep in mind that any function calls need to
have access to variables from earlier in the test if used. Standard scoping
rules apply. This check operates at function scope so does not have access to
nested conditionals, loops, or blocks from earlier in the test. See the check
macro if more fine grained control over nested scope is required upon a
failure.*/
#define check_error_end(...)                                                   \
please_use_at_least_one_check_and_a_check_end_macro:                           \
    __VA_OPT__((void)(__extension__({                                          \
                   if (check_private_macro_res == CHECK_ERROR) {               \
                       __VA_ARGS__                                             \
                   }                                                           \
               }));)                                                           \
    return check_private_macro_res;                                            \
    }

/** @brief Runs a list of test functions and returns the result.
@param[in] test_fn_list all test functions to run in main.
@return a passing test result if all tests pass or failing result if any fail.
All tests to completion even if the overall result is a failure.
@note The check runner also seed the global random `srand` function with a
global static seed that will be reported along with any failed check throughout
the program. This allows for any test that relies on randomness to reproduce the
failed result for debugging.

Return this macro from the main function of the test program. All tests
will run but the testing result for the entire program will be set to CHECK_FAIL
upon the first failure. All functions must return an enum Check_result though
their argument signatures may vary. If a test fails with an error, this runner
will set the overall test state to fail and the user should examine the
individual test that failed with CHECK_FAIL or CHECK_ERROR. */
#define check_run(test_fn_list...)                                             \
    (__extension__({                                                           \
        /* The compiler will maintain the order of calling these seeding       \
           functions before the array initializer function calls because the   \
           observable behavior must be maintained. Critical that seed occurs   \
           first. */                                                           \
        check_random_seed = (unsigned)time(NULL); /* NOLINT */                 \
        srand(check_random_seed);                 /* NOLINT */                 \
        enum Check_result const check_private_all_checks[] = {test_fn_list};   \
        enum Check_result check_private_all_checks_res = CHECK_PASS;           \
        for (unsigned long long check_run_index = 0;                           \
             check_run_index                                                   \
             < sizeof(check_private_all_checks) / sizeof(enum Check_result);   \
             ++check_run_index) {                                              \
            if (check_private_all_checks[check_run_index] != CHECK_PASS) {     \
                check_private_all_checks_res = CHECK_FAIL;                     \
            }                                                                  \
        }                                                                      \
        check_private_all_checks_res;                                          \
    }))

#endif /* CHECKERS_H */
