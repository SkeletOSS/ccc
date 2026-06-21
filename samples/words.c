/** The words program is a simple word counter that aims to demonstrate
the benefits of flexible memory layouts and context data.

Please specify a command as follows:
./build/[debug/]bin/words [flag] -f=[path/to/file]
[flag]:
-f=[/path/to/file]
-c reports the words by count in descending order.
-rc reports words by count in ascending order.
-top=N reports the top N words by frequency.
-last=N reports the last N words by frequency
-alph=N reports the first N words alphabetically with counts.
-ralph=N reports the last N words alphabetically with counts.
-find=[WORD] reports the count of the specified word. */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAT_BUFFER_USING_NAMESPACE_CCC
#define ARRAY_ADAPTIVE_MAP_USING_NAMESPACE_CCC
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC
#define TYPES_USING_NAMESPACE_CCC

#include "ccc/flat_buffer.h"
#include "ccc/sort.h"
#include "ccc/specialized/array_adaptive_map.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "str_view/str_view.h"
#include "utility/cli.h"
#include "utility/defer.h"
#include "utility/std_allocator.h"
#include "utility/string_arena.h"

enum Action_type {
    COUNT,
    FIND,
};

/* A word will be counted with a map and then sorted with a Flat_buffer and flat
priority queue. One type works well for both because they don't need intrusive
elements to operate those containers. */
typedef struct {
    struct String_offset ofs;
    int freq;
} Word;

typedef struct {
    Handle handle;
} Word_handle;

typedef struct {
    Array_adaptive_map map;
} Word_map;

typedef struct {
    Flat_buffer buffer;
} Word_buffer;

struct Action_pack {
    SV_Str_view file;
    enum Action_type type;
    union {
        size_t n;
        SV_Str_view w;
    };
    union {
        void (*freq_fn)(FILE *file, size_t n, CCC_Allocator const *);
        void (*find_fn)(FILE *file, SV_Str_view w, CCC_Allocator const *);
    };
};

/*=======================     Constants    ==================================*/

enum : size_t {
    ARENA_START_CAP = 4096,
    ALL_FREQUENCIES = 0,
};

static SV_Str_view const space = SV_from(" ");
static SV_Str_view const directions = SV_from(
    "\nPlease specify a command as follows:\n"
    "./build/[debug/]bin/words [flag] -f=[path/to/file]\n"
    "[flag]:\n"
    "-f=[/path/to/file]\n"
    "-c\n"
    "\treports the words by count in descending order.\n"
    "-rc\n"
    "\treports words by count in ascending order.\n"
    "-top=N\n"
    "\treports the top N words by frequency.\n"
    "-last=N\n"
    "\treports the last N words by frequency\n"
    "-alph=N\n"
    "\treports the first N words alphabetically with counts.\n"
    "-ralph=N\n"
    "\treports the last N words alphabetically with counts.\n"
    "-find=[WORD]\n"
    "\treports the count of the specified word.\n"
);

/*=======================   Prototypes     ==================================*/

#define logerr(string...) (void)fprintf(stderr, string)

#define check(cond, ...)                                                       \
    do {                                                                       \
        if (!(cond)) {                                                         \
            __VA_OPT__(__VA_ARGS__)                                            \
            (void)fprintf(                                                     \
                stderr,                                                        \
                "%s, %d, condition is false: %s\n",                            \
                __FILE__,                                                      \
                __LINE__,                                                      \
                #cond                                                          \
            );                                                                 \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

#define foreach_map_word(                                                      \
    word_map_pointer, word_iterator_type_declaration, code_block...            \
)                                                                              \
    for (Handle_index word_map_handle_index = begin(&(word_map_pointer)->map); \
         word_map_handle_index != end(&(word_map_pointer)->map);               \
         word_map_handle_index                                                 \
         = next(&(word_map_pointer)->map, word_map_handle_index)) {            \
        word_iterator_type_declaration = array_adaptive_map_at(                \
            &(word_map_pointer)->map, word_map_handle_index                    \
        );                                                                     \
        code_block                                                             \
    }

#define foreach_map_word_reverse(                                              \
    word_map_pointer, word_iterator_type_declaration, code_block...            \
)                                                                              \
    for (Handle_index word_map_handle_index                                    \
         = reverse_begin(&(word_map_pointer)->map);                            \
         word_map_handle_index != reverse_end(&(word_map_pointer)->map);       \
         word_map_handle_index                                                 \
         = reverse_next(&(word_map_pointer)->map, word_map_handle_index)) {    \
        word_iterator_type_declaration = array_adaptive_map_at(                \
            &(word_map_pointer)->map, word_map_handle_index                    \
        );                                                                     \
        code_block                                                             \
    }

#define foreach_buffer_word(                                                   \
    word_buffer_pointer, word_iterator_type_declaration, code_block...         \
)                                                                              \
    for (Word *foreach_buffer_word_iterator                                    \
         = begin(&(word_buffer_pointer)->buffer);                              \
         foreach_buffer_word_iterator != end(&(word_buffer_pointer)->buffer);  \
         foreach_buffer_word_iterator = next(                                  \
             &(word_buffer_pointer)->buffer, foreach_buffer_word_iterator      \
         )) {                                                                  \
        word_iterator_type_declaration = foreach_buffer_word_iterator;         \
        code_block                                                             \
    }

static void print_found(FILE *, SV_Str_view, CCC_Allocator const *);
static void print_top_n(FILE *, size_t, CCC_Allocator const *);
static void print_last_n(FILE *, size_t, CCC_Allocator const *);
static void print_alpha_n(FILE *, size_t, CCC_Allocator const *);
static void print_ralpha_n(FILE *, size_t, CCC_Allocator const *);
static Word_buffer map_to_buffer(Word_map const *, CCC_Allocator const *);
static void print_n(
    Word_map *, CCC_Order, struct String_arena *, size_t, CCC_Allocator const *
);
static struct Int_conversion parse_n_ranks(SV_Str_view);
static struct String_offset
clean_word(struct String_arena *, SV_Str_view, CCC_Allocator const *);
static Word_map
create_frequency_map(struct String_arena *, FILE *, CCC_Allocator const *);
static Order order_string_keys(Key_comparator_arguments);
static Order order_words(Comparator_arguments);
static FILE *open_file(SV_Str_view);
static void print_str_view(FILE *, SV_Str_view);
static Word_handle
word_try_insert(Word_map *, Word const *, CCC_Allocator const *);
static Word *word_at(Word_map const *, Word_handle const *);
static Word *word_get_key_value(Word_map *, struct String_offset const *);
static CCC_Result
word_buffer_sort(Word_buffer const *, CCC_Order, struct String_arena *);

/*=======================     Main         ==================================*/

int
main(int argc, char *argv[]) {
    if (argc == 2
        && SV_starts_with(SV_from_terminated(argv[1]), SV_from("-h"))) {
        print_str_view(stdout, directions);
        return 0;
    }
    if (argc < 3) {
        return 0;
    }
    struct Action_pack exe = {};
    for (int arg = 1; arg < argc; ++arg) {
        SV_Str_view const sv_arg = SV_from_terminated(argv[arg]);
        if (SV_starts_with(sv_arg, SV_from("-c"))) {
            exe.type = COUNT;
            exe.freq_fn = print_top_n;
            exe.n = ALL_FREQUENCIES;
        } else if (SV_starts_with(sv_arg, SV_from("-rc"))) {
            exe.type = COUNT;
            exe.freq_fn = print_last_n;
            exe.n = ALL_FREQUENCIES;
        } else if (SV_starts_with(sv_arg, SV_from("-top="))) {
            exe.type = COUNT;
            exe.freq_fn = print_top_n;
            struct Int_conversion c = parse_n_ranks(sv_arg);
            check(c.status != CONV_ER,
                  logerr("cannot convert -top= flag to int"););
            exe.n = (size_t)c.conversion;
        } else if (SV_starts_with(sv_arg, SV_from("-last="))) {
            exe.type = COUNT;
            exe.freq_fn = print_last_n;
            struct Int_conversion c = parse_n_ranks(sv_arg);
            check(c.status != CONV_ER,
                  logerr("cannot convert -last= flat to int"););
            exe.n = (size_t)c.conversion;
        } else if (SV_starts_with(sv_arg, SV_from("-alph="))) {
            exe.type = COUNT;
            exe.freq_fn = print_alpha_n;
            struct Int_conversion c = parse_n_ranks(sv_arg);
            check(c.status != CONV_ER,
                  logerr("cannot convert -alph= flag to int"););
            exe.n = (size_t)c.conversion;
        } else if (SV_starts_with(sv_arg, SV_from("-ralph="))) {
            exe.type = COUNT;
            exe.freq_fn = print_ralpha_n;
            struct Int_conversion c = parse_n_ranks(sv_arg);
            check(c.status != CONV_ER,
                  logerr("cannot convert -ralph= flat to int"););
            exe.n = (size_t)c.conversion;
        } else if (SV_starts_with(sv_arg, SV_from("-find="))) {
            SV_Str_view const raw_word = SV_substr(
                sv_arg, SV_find(sv_arg, 0, SV_from("=")) + 1, SV_len(sv_arg)
            );
            check(!SV_is_empty(raw_word),
                  logerr("-find= flag has invalid entry"););
            exe.type = FIND;
            exe.find_fn = print_found;
            exe.w = raw_word;
        } else if (SV_starts_with(sv_arg, SV_from("-f="))) {
            SV_Str_view const raw_file = SV_substr(
                sv_arg, SV_find(sv_arg, 0, SV_from("=")) + 1, SV_len(sv_arg)
            );
            check(!SV_is_empty(raw_file), logerr("file string is empty"););
            exe.file = raw_file;
        } else if (SV_starts_with(sv_arg, SV_from("-h"))) {
            print_str_view(stdout, directions);
            return 0;
        } else {
            if (SV_begin(sv_arg)) {
                logerr("unrecognized argument: %s\n", SV_begin(sv_arg));
            }
            return 1;
        }
    }
    FILE *const f = open_file(exe.file);
    defer {
        (void)fclose(f);
    }
    if (!f) {
        if (SV_begin(exe.file)) {
            logerr("error opening: %s\n", SV_begin(exe.file));
        }
        return 1;
    }
    if (exe.type == COUNT) {
        exe.freq_fn(f, exe.n, &std_allocator);
    } else if (exe.type == FIND && SV_begin(exe.w)) {
        exe.find_fn(f, exe.w, &std_allocator);
    } else {
        logerr("invalid count or empty word searched\n");
        return 1;
    }
    return 0;
}

/*=======================   Static Impl    ==================================*/

static void
print_found(
    FILE *const f, SV_Str_view w, CCC_Allocator const *const allocator
) {
    struct String_arena a = string_arena_create(ARENA_START_CAP, allocator);
    Word_map map = create_frequency_map(&a, f, allocator);
    defer {
        string_arena_free(&a, allocator);
        (void)array_adaptive_map_clear_and_free(
            &map.map, &(CCC_Destructor){}, allocator
        );
    }
    check(a.arena);
    check(!is_empty(&map.map));
    struct String_offset wc = clean_word(&a, w, allocator);
    if (!wc.error) {
        Word const *const found_w = word_get_key_value(&map, &wc);
        if (found_w) {
            printf(
                "%s %d\n", string_arena_at(&a, &found_w->ofs), found_w->freq
            );
        }
    }
}

static void
print_top_n(FILE *const f, size_t n, CCC_Allocator const *const allocator) {
    struct String_arena a = string_arena_create(ARENA_START_CAP, allocator);
    Word_map map = create_frequency_map(&a, f, allocator);
    defer {
        (void)clear_and_free(&map.map, &(CCC_Destructor){}, allocator);
        string_arena_free(&a, allocator);
    }
    check(a.arena);
    check(!is_empty(&map.map));
    print_n(&map, CCC_ORDER_GREATER, &a, n, allocator);
}

static void
print_last_n(FILE *const f, size_t n, CCC_Allocator const *const allocator) {
    struct String_arena a = string_arena_create(ARENA_START_CAP, allocator);
    Word_map map = create_frequency_map(&a, f, allocator);
    defer {
        (void)clear_and_free(&map.map, &(CCC_Destructor){}, allocator);
        string_arena_free(&a, allocator);
    }
    check(a.arena);
    check(!is_empty(&map.map));
    print_n(&map, CCC_ORDER_LESSER, &a, n, allocator);
}

static void
print_alpha_n(FILE *const f, size_t n, CCC_Allocator const *const allocator) {
    struct String_arena a = string_arena_create(ARENA_START_CAP, allocator);
    Word_map map = create_frequency_map(&a, f, allocator);
    defer {
        (void)clear_and_free(&map.map, &(CCC_Destructor){}, allocator);
        string_arena_free(&a, allocator);
    }
    check(a.arena);
    check(!is_empty(&map.map));
    if (!n) {
        n = count(&map.map).count;
    }
    /* The ordered nature of the map comes in handy for alpha printing. */
    size_t word = 0;
    foreach_map_word(&map, Word const *const w, {
        printf("%s %d\n", string_arena_at(&a, &w->ofs), w->freq);
        if (++word >= n) {
            return;
        }
    });
}

static void
print_ralpha_n(FILE *const f, size_t n, CCC_Allocator const *const allocator) {
    struct String_arena a = string_arena_create(ARENA_START_CAP, allocator);
    Word_map map = create_frequency_map(&a, f, allocator);
    defer {
        (void)clear_and_free(&map.map, &(CCC_Destructor){}, allocator);
        string_arena_free(&a, allocator);
    }
    check(a.arena);
    check(!is_empty(&map.map));
    if (!n) {
        n = count(&map.map).count;
    }
    /* The ordered nature of the map comes in handy for reverse iteration. */
    size_t word = 0;
    foreach_map_word_reverse(&map, Word const *const w, {
        printf("%s %d\n", string_arena_at(&a, &w->ofs), w->freq);
        if (++word >= n) {
            return;
        }
    });
}

static void
print_n(
    Word_map *const map,
    CCC_Order const order,
    struct String_arena *const arena,
    size_t n,
    CCC_Allocator const *const allocator
) {
    Word_buffer freqs = map_to_buffer(map, allocator);
    defer {
        (void)clear_and_free(&freqs.buffer, &(CCC_Destructor){}, allocator);
    }
    check(!flat_buffer_is_empty(&freqs.buffer));
    if (!n) {
        n = count(&freqs.buffer).count;
    }
    CCC_Result const result = word_buffer_sort(&freqs, order, arena);
    check(result == CCC_RESULT_OK);
    size_t w = 0;
    foreach_buffer_word(&freqs, Word const *const i, {
        char const *const arena_str = string_arena_at(arena, &i->ofs);
        if (arena_str) {
            printf("%zu. %s %d\n", w + 1, arena_str, i->freq);
        }
        if (++w >= n) {
            return;
        }
    });
}

static Word_buffer
map_to_buffer(Word_map const *const map, CCC_Allocator const *const allocator) {
    check(!is_empty(&map->map));
    Word_buffer freqs = {CCC_flat_buffer_with_capacity(
        Word, *allocator, count(&map->map).count
    )};
    size_t cap = capacity(&freqs.buffer).count;
    foreach_map_word(map, Word const *const w, {
        Word const *const pushed = flat_buffer_emplace_back(
            &freqs.buffer,
            &(CCC_Allocator){},
            (Word){.ofs = w->ofs, .freq = w->freq}
        );
        check(pushed);
        if (cap-- == 0) {
            return freqs;
        }
    });
    return freqs;
}

/*=====================    Container Construction     =======================*/

static Word_map
create_frequency_map(
    struct String_arena *const arena,
    FILE *const f,
    CCC_Allocator const *const allocator
) {
    char *linepointer = NULL;
    defer {
        free(linepointer);
    }
    size_t len = 0;
    ptrdiff_t read = 0;
    Word_map map = {array_adaptive_map_default(
        Word,
        ofs,
        (CCC_Key_comparator){
            .compare = order_string_keys,
            .context = arena,
        }
    )};
    while ((read = getline(&linepointer, &len, f)) > 0) {
        SV_Str_view const line = {.str = linepointer, .len = (size_t)read - 1};
        for (SV_Str_view word_view = SV_token_begin(line, space);
             !SV_token_end(line, word_view);
             word_view = SV_token_next(line, word_view, space)) {
            struct String_offset const cw
                = clean_word(arena, word_view, allocator);
            if (cw.error) {
                continue;
            }
            Word_handle const handle = word_try_insert(
                &map, &(Word){.ofs = cw, .freq = 1}, allocator
            );
            if (occupied(&handle.handle)) {
                word_at(&map, &handle)->freq++;
            }
        }
    }
    return map;
}

static struct String_offset
clean_word(
    struct String_arena *const a,
    SV_Str_view wv,
    CCC_Allocator const *const allocator
) {
    /* It is hard to know how many characters will make it to a cleaned word
       and one pass is ideal so arena api allows push back on last alloc. */
    struct String_offset str = string_arena_allocate(a, 0, allocator);
    if (str.error) {
        return str;
    }
    for (char const *c = SV_begin(wv); c != SV_end(wv); c = SV_next(c)) {
        if (!isalpha(*c) && *c != '-') {
            string_arena_pop_back(a, &str);
            return (struct String_offset){.error = STRING_ARENA_INVALID};
        }
        enum String_arena_result const pushed_char
            = string_arena_push_back(a, &str, (char)tolower(*c), allocator);
        check(pushed_char == STRING_ARENA_OK);
    }
    if (!str.len) {
        return (struct String_offset){.error = STRING_ARENA_INVALID};
    }
    char const *const w = string_arena_at(a, &str);
    check(w);
    if (!isalpha(*w) || !isalpha(*(w + (str.len - 1)))) {
        enum String_arena_result const pop = string_arena_pop_back(a, &str);
        check(pop == STRING_ARENA_OK);
        return (struct String_offset){.error = STRING_ARENA_INVALID};
    }
    return str;
}

/*=======================   Container Helpers    ============================*/

static inline Word *
word_get_key_value(Word_map *const map, struct String_offset const *const key) {
    return array_adaptive_map_at(
        &map->map, array_adaptive_map_get_key_value(&map->map, key)
    );
}

static inline Word_handle
word_try_insert(
    Word_map *const map,
    Word const *const word,
    CCC_Allocator const *const allocator
) {
    return (Word_handle){
        array_adaptive_map_try_insert(&map->map, word, allocator)};
}

static inline Word *
word_at(Word_map const *const map, Word_handle const *const handle) {
    Word *const word
        = array_adaptive_map_at(&map->map, unwrap(&handle->handle));
    check(word);
    return word;
}

static inline CCC_Result
word_buffer_sort(
    Word_buffer const *const words,
    CCC_Order const order,
    struct String_arena *const arena
) {
    return CCC_sort_heapsort(
        &words->buffer,
        &(Word){},
        order,
        &(CCC_Comparator){
            .compare = order_words,
            .context = arena,
        }
    );
}

static Order
order_string_keys(Key_comparator_arguments const c) {
    Word const *const w = c.type_right;
    struct String_arena const *const a = c.context;
    struct String_offset const *const id = c.key_left;
    char const *const key_word = string_arena_at(a, id);
    char const *const struct_word = string_arena_at(a, &w->ofs);
    check(key_word && struct_word);
    int const res = strcmp(key_word, struct_word);
    return (res > 0) - (res < 0);
}

/* Sorts by frequency then alphabetic order if frequencies are tied. */
static Order
order_words(Comparator_arguments const c) {
    Word const *const left = c.type_left;
    Word const *const right = c.type_right;
    Order const freq_order
        = (left->freq > right->freq) - (left->freq < right->freq);
    if (freq_order != CCC_ORDER_EQUAL) {
        return freq_order;
    }
    struct String_arena const *const arena = c.context;
    char const *const left_word = string_arena_at(arena, &left->ofs);
    char const *const right_word = string_arena_at(arena, &right->ofs);
    check(left_word && right_word);
    int const res = strcmp(left_word, right_word);
    /* Looks like we have chosen wrong order to return but not so: greater
       lexicographic order is sorted first in a min priority queue or
       CCC_ORDER_LESSER in this case. */
    return (res < 0) - (res > 0);
}

/*=======================   CLI Helpers    ==================================*/

static struct Int_conversion
parse_n_ranks(SV_Str_view arg) {
    size_t const eql = SV_reverse_find(arg, SV_npos(arg), SV_from("="));
    if (eql == SV_npos(arg)) {
        return (struct Int_conversion){.status = CONV_ER};
    }
    arg = SV_substr(arg, eql + 1, ULLONG_MAX);
    if (SV_is_empty(arg)) {
        (void)fprintf(stderr, "please specify word frequency range.\n");
        return (struct Int_conversion){.status = CONV_ER};
    }
    return convert_to_int(SV_begin(arg));
}

static void
print_str_view(FILE *const f, SV_Str_view const sv) {
    if (!SV_is_empty(sv)) {
        (void)fwrite(SV_begin(sv), sizeof(char), SV_len(sv), f);
    }
}

static FILE *
open_file(SV_Str_view file) {
    if (SV_is_empty(file)) {
        return NULL;
    }
    FILE *const f = fopen(SV_begin(file), "r");
    if (!f) {
        (void)fprintf(
            stderr,
            "Opening file [%s] failed with errno set to: %d\n",
            SV_begin(file),
            errno
        );
    }
    return f;
}
