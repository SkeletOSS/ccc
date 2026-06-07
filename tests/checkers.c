#include <stdio.h>

#include "checkers.h"

void
check_print_fail_message(
    char const *const function_string,
    int const line,
    char const *const result_expression_string,
    char const *const expected_expression_string,
    union Check_bytes const result_output_bytes,
    union Check_bytes const expected_output_bytes,
    bool const is_address,
    unsigned const random_seed
) {
    if (is_address) {
        (void)fprintf(
            stderr,
            "%s\n--\nfailure in %s, line %d%s\n%scheck: result( %s ) == "
            "expected( "
            "%s )%s\n%serror: result( 0x%lx ) != expected( 0x%lx )\nrandom "
            "seed: %u\n%s",
            CHECK_CYAN,
            function_string,
            line,
            CHECK_NONE,
            CHECK_GREEN,
            result_expression_string,
            expected_expression_string,
            CHECK_NONE,
            CHECK_RED,
            result_output_bytes.as_address,
            expected_output_bytes.as_address,
            random_seed,
            CHECK_NONE
        );
    } else {
        (void)fprintf(
            stderr,
            "%s\n--\nfailure in %s, line %d%s\n%scheck: result( %s ) == "
            "expected( "
            "%s )%s\n%serror: result( %ju ) != expected( %ju )\nrandom seed: "
            "%u\n%s",
            CHECK_CYAN,
            function_string,
            line,
            CHECK_NONE,
            CHECK_GREEN,
            result_expression_string,
            expected_expression_string,
            CHECK_NONE,
            CHECK_RED,
            result_output_bytes.as_bytes,
            expected_output_bytes.as_bytes,
            random_seed,
            CHECK_NONE
        );
    }
}
