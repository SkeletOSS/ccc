/** Author: Alexander G. Lopez
This file implements data compression algorithms over simple files, primarily
text files for demonstration purposes. Various compression algorithms such as
Huffman and LZW are implemented using CCC containers. */
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#    include <linux/limits.h>
#    define FILESYS_MAX_PATH PATH_MAX
#endif
#ifdef __APPLE__
#    include <sys/syslimits.h>
#    define FILESYS_MAX_PATH PATH_MAX
#endif

#define FLAT_BITSET_USING_NAMESPACE_CCC
#define FLAT_BUFFER_USING_NAMESPACE_CCC
#define FLAT_HASH_MAP_USING_NAMESPACE_CCC
#define FLAT_PRIORITY_QUEUE_USING_NAMESPACE_CCC
#define TRAITS_USING_NAMESPACE_CCC

#include "ccc/flat_bitset.h"
#include "ccc/flat_buffer.h"
#include "ccc/flat_hash_map.h"
#include "ccc/flat_priority_queue.h"
#include "ccc/traits.h"
#include "ccc/types.h"
#include "str_view/str_view.h"
#include "utility/defer.h"
#include "utility/std_allocator.h"
#include "utility/string_arena.h"

/*===========================   Type Declarations  ==========================*/

enum Print_branch {
    BRANCH = 0, /* ├── */
    LEAF = 1,   /* └── */
};

/** A thin wrapper around a bit set to provide queue/deq like behavior. For
Huffman Compression we only need to push and pop from back and pop from the
front so we don't need the full suite of deq functionality. The bit set provides
almost all the functionality needed we just need to manage what happens when
we pop from bit set but do not manually decrease the size of the underlying bit
set container. Instead we manage our own front and size fields. */
struct Bit_queue {
    Flat_bitset bs;
    size_t front;
    size_t count;
};

/*=======================   Huffman Encoding Types   ========================*/

/** Simple entry in the flat hash map while counting character occurrences. */
struct Huff_character_frequency {
    char ch;
    size_t freq;
};

/** Type safe wrapper for mapping character frequencies. */
struct Huff_character_frequency_map {
    CCC_Flat_hash_map map;
};

enum : uint8_t {
    /** Caching for iterative traversals uses this position as end sentinel. */
    HUFF_MAX_CHILD_COUNT = 2,
};

/** Tree nodes will be pushed into a CCC_Flat_buffer. This is the same concept
as freely allocating them in the heap, but much more efficient and convenient.
We push elements to the back of the Flat_buffer to allocate and because we never
free any nodes until we are done with the entire tree this is an optimal bump
allocator. All memory is freed at once in one contiguous allocation. The only
detail to manage is that due to resizing of the Flat_buffer elements must track
each other through indices not pointers. */
struct Huff_node {
    /** The parent for backtracking during DFS and Pre-Order traversal. */
    size_t parent;
    /** The necessary links needed to build the encoding tree. */
    size_t link[HUFF_MAX_CHILD_COUNT];
    /** The leaf character if this node is a leaf. */
    char ch;
    /** The caching iterator to help emulate recursion with iteration. */
    uint8_t child_index;
};

/** Type safe wrapper for buffering Huffman nodes. */
struct Huff_node_buffer {
    CCC_Flat_buffer buffer;
};

/** Element intended for the flat priority queue during the tree building phase.
Pair nodes are paired in twos as they are popped from the priority queue and
pushed back as the sum of their frequencies. This is how the tree is built.
Having a small simple type in the contiguous flat priority queue is good for
performance and the entire Flat_buffer can be freed when the algorithm
completes. The priority queue is only needed while building the tree. */
struct Huff_pair_node {
    size_t frequency;
    size_t node_index;
};

/** Type safe wrapper for tracking Pair nodes in a queue. */
struct Huff_pair_node_priority_queue {
    CCC_Flat_priority_queue priority_queue;
};

/** Type safe wrapper for pushing pair nodes into buffer. */
struct Huff_pair_node_buffer {
    CCC_Flat_buffer buffer;
};

/** It is helpful to know how many leaves and total nodes there are for
reserving the appropriate space for helper data structures. */
struct Huff_tree {
    struct Huff_node_buffer tree_storage;
    size_t root;
    size_t num_nodes;
    size_t num_leaves;
};

/** As we build the bit queue representing the paths to all characters in the
text we encode, we can memoize known sequences we have already seen in the
bit queue. We will record the first encounter with every character in the bit
queue and simply append this range of bits to the end of the queue as we build
the sequence. */
struct Huff_path_memo {
    /** The key for the path memo. */
    char ch;
    /** The index in the bit queue where we have seen this path before. */
    size_t bitq_start_index;
    /** The length of the known path we have already pushed to the queue. */
    size_t path_len;
};

/** Type safe wrapper for mapping paths. */
struct Huff_path_memo_map {
    CCC_Flat_hash_map map;
};

enum : size_t {
    /** Number of ascii characters can be a good starting cap for the arena. */
    HUFF_START_STRING_ARENA_CAP = 256,
};

/** The compact representation of the tree structure we will encode to a
compressed file for later reconstruction. */
struct Huff_compressed_tree {
    /** The Pre-Order traversal of internal nodes and leaves. Every internal
    node encountered on the way down is a 1 and every leaf is a 0. */
    struct Bit_queue tree_paths;
    /** The arena that allows us to form the dynamic string of leaves. */
    struct String_arena arena;
    /** The handle to the leaf string in the string arena. */
    struct String_offset leaf_string;
};

enum : uint32_t {
    /** File format "cccz" in header. */
    CCCZ_MAGIC = 0x6363637A,
};

/** The header representing the compressed file structure of a compressed file
via Huffman Encoding. The fields of this struct should be handled in order as
we write to or read from a file. It is not strictly necessary to use this but
it helps keep logic consistent across compression and decompression. */
struct Huff_encoding {
    /** Magic to recognize our cccz files "cccz" */
    uint32_t magic;
    /** The number of leaves in the tree minus 1. Subtract 1 in case we actually
        have all 256 characters used as leaves in which case they could only
        be recorded in the file as 255, the number that fits in uint8_t. */
    uint8_t leaves_minus_one;
    /** Number of bits in the file content bit queue we encoded. */
    size_t file_bits_count;
    /** The compact representation of the tree to be reconstructed later. */
    struct Huff_compressed_tree blueprint;
    /** The path to every character encountered in the file text in order. */
    struct Bit_queue file_bits;
};

/*===========================   LZW Encoding Types   ========================*/

/** The smallest byte compatible type available for LZW codes. */
typedef uint16_t Lzw_code;

enum : Lzw_code {
    /** The default number of single characters we can encode as prefixes. */
    LZW_STARTING_PREFIX_COUNT = UCHAR_MAX + 1,
    /** We limit the dictionary to the maximum number that 12 bits can count. */
    LZW_CODE_LIMIT = 4096,
    /** No code can hold this value so we use it as the sentinel. */
    LZW_NULL_PARENT_CODE = 4097,
    /** During encoding we cannot encounter this character so it signals we need
    to clear and reset the dictionary. */
    LZW_CLEAR_CODE = 256,
    /** This is a signal to stop encoding and decoding. The file is complete. */
    LZW_STOP_CODE = LZW_CLEAR_CODE + 1,
    /** Prefix codes the grow towards 4096 start at this value. */
    LZW_NEW_CODE_START = LZW_STOP_CODE + 1,
    /** This many bits is a reasonable balance between RAM usage and storage
    efficiency. */
    LZW_FILE_CODE_BIT_COUNT = 12,
    /** The rounded up bytes needed to store our bit packing size. */
    LZW_FILE_CODE_BIT_PACK_BYTES = (LZW_FILE_CODE_BIT_COUNT + (CHAR_BIT - 1))
        / CHAR_BIT,
    /** A mask with all bits on covering the range possible for codes. */
    LZW_CODE_WIDTH_MASK = LZW_CODE_LIMIT - 1,
};
static_assert(
    (sizeof(Lzw_code) * CHAR_BIT) >= LZW_FILE_CODE_BIT_COUNT,
    "Bit packing size for file compression is smaller than type used to store "
    "codes."
);

/** This prefix is used for encoding to hash unique code combinations and during
decoding to walk back up the prefix tree to rebuild strings in reverse. The
single next character field is critical for actually building the strings. */
struct Lzw_prefix {
    /** The prefix or parent code of this current character. This will always
    lead back to our table of single characters at the root. */
    Lzw_code prefix;
    /** The next character in the string. A real char we can write to a string
    before continuing on the prefix parent path in reverse. */
    unsigned char next_char;
};

/** The hash table uses this type during encoding to uniquely identify new
strings that are generating new codes we have not seen. */
struct Lzw_key_value {
    /** The hash combination of prefix and character used to encode. */
    struct Lzw_prefix key;
    /** The code we have given out to this uniquely hashed element in our
    encoding hash map. */
    Lzw_code code_value;
};

/** The thin wrapper around the map for type safe usage. */
struct Lzw_map {
    /** The flat hash map type from the container collection. */
    CCC_Flat_hash_map map;
};

/* Used to help put 12 bit codes back together into a uint16_t while
decompressing a file. */
struct Lzw_code_accumulator {
    /** The sliding window of bits we gain from a file. */
    uint32_t accumulator;
    /** The current count of active bits in the accumulator sliding window. */
    uint8_t bit_count;
    /** The current 12 bit code we just obtained from the sliding window. */
    Lzw_code current_code;
};

/*=============================   File Types   ==============================*/

/** Type to help categorize command line arguments for algorithms. */
enum Compression_algorithm : uint8_t {
    HUFFMAN = 0,
    LZW,
    ALGORITHM_COUNT,
};

/** Files the user wants zipped or unzipped. */
struct Zip_actions {
    enum Compression_algorithm algorithm;
    SV_Str_view zip;
    SV_Str_view unzip;
};

/** A type to help indicate errors while obtaining file characters. */
struct File_char {
    unsigned char c;
    bool error;
};

/** A type to open files and remember their paths. */
struct New_file {
    SV_Str_view path;
    FILE *file;
};

typedef void Algorithm(SV_Str_view, CCC_Allocator const *);

static SV_Str_view const output_dir = SV_from("samples/output/");
static SV_Str_view const cccz_suffix = SV_from(".cccz");

/*===========================      Prototypes      ==========================*/

static void huff_encode(SV_Str_view, CCC_Allocator const *);
static struct Huff_pair_node_priority_queue huff_build_encoding_priority_queue(
    FILE *, struct Huff_tree *, CCC_Allocator const *
);
static CCC_Tribool huff_bitq_is_empty(struct Bit_queue const *);
static CCC_Result
huff_bitq_push_back(struct Bit_queue *, CCC_Tribool, CCC_Allocator const *);
static CCC_Tribool huff_bitq_pop_back(struct Bit_queue *);
static CCC_Tribool huff_bitq_pop_front(struct Bit_queue *);
static CCC_Tribool huff_bitq_test(struct Bit_queue const *, size_t);
static CCC_Result huff_bitq_maybe_resize(
    struct Bit_queue *bq, size_t bits_to_add, CCC_Allocator const *allocator
);
static size_t huff_bitq_count(struct Bit_queue const *);
static void huff_bitq_clear_and_free(struct Bit_queue *, CCC_Allocator const *);
static uint64_t hash_char(CCC_Key_arguments);
static CCC_Order char_order(CCC_Key_comparator_arguments);
static CCC_Order order_freqs(CCC_Comparator_arguments);
static CCC_Order path_memo_order(CCC_Key_comparator_arguments);
static struct Huff_path_memo huff_append_tree_path_to_bitq(
    struct Huff_tree *, struct Bit_queue *, char, CCC_Allocator const *
);
static struct Bit_queue
huff_build_encoding_bitq(FILE *, struct Huff_tree *, CCC_Allocator const *);
static struct Huff_tree huff_build_encoding_tree(FILE *, CCC_Allocator const *);
static struct Huff_compressed_tree
huff_compress_tree(struct Huff_tree *, CCC_Allocator const *);
static void huff_free_encode_tree(struct Huff_tree *, CCC_Allocator const *);
static void
huff_print_tree(struct Huff_tree const *, size_t, CCC_Allocator const *);
static void huff_print_inner_tree(
    struct Huff_tree const *,
    size_t,
    enum Print_branch,
    char const *,
    CCC_Allocator const *
);
static void huff_print_node(struct Huff_tree const *, size_t);
static bool huff_is_leaf(struct Huff_tree const *, size_t);
static void huff_print_bitq(struct Bit_queue const *);
static void huff_decode(SV_Str_view, CCC_Allocator const *);
static struct Huff_tree
huff_reconstruct_tree(struct Huff_compressed_tree *, CCC_Allocator const *);
static void
huff_reconstruct_text(FILE *, struct Huff_tree const *, struct Bit_queue *);
static size_t huff_branch_index(struct Huff_tree const *, size_t, uint8_t);
static size_t huff_parent_index(struct Huff_tree const *, size_t);
static char huff_char_index(struct Huff_tree const *, size_t);
static struct Huff_node *huff_node_at(struct Huff_tree const *, size_t);
static void huff_write_to_file(SV_Str_view, size_t, struct Huff_encoding *);
static void huff_write_bitq(FILE *, struct Bit_queue *);
static struct Huff_encoding
huff_read_from_file(SV_Str_view, CCC_Allocator const *);
static size_t huff_readbytes(FILE *, void *, size_t);
static size_t huff_write_bytes(FILE *, void const *, size_t);
static void
huff_fill_bitq(FILE *, struct Bit_queue *, size_t, CCC_Allocator const *);
static size_t huff_node_buffer_index(
    struct Huff_node_buffer const *, struct Huff_node const *
);
static struct Huff_pair_node *huff_pair_node_priority_queue_push(
    struct Huff_pair_node_priority_queue *,
    struct Huff_pair_node const *,
    CCC_Allocator const *
);
static CCC_Result
huff_pair_node_priority_queue_pop(struct Huff_pair_node_priority_queue *);
static struct Huff_pair_node *huff_pair_node_priority_queue_front(
    struct Huff_pair_node_priority_queue const *
);

#define huff_node_buffer_emplace_back(                                         \
    huff_node_buffer_pointer, allocator, compound_literal...                   \
)                                                                              \
    (__extension__({                                                           \
        struct Huff_node_buffer *const huff_node_buffer_emplace_back_buffer    \
            = (huff_node_buffer_pointer);                                      \
        CCC_flat_buffer_emplace_back(                                          \
            &huff_node_buffer_emplace_back_buffer->buffer,                     \
            (allocator),                                                       \
            compound_literal                                                   \
        );                                                                     \
    }))

#define huff_character_frequency_map_increment_entry_or_insert_with(           \
    character_frequency_map_pointer,                                           \
    key_pointer,                                                               \
    allocator,                                                                 \
    lazy_evaluated_key_value_default_insertion...                              \
)                                                                              \
    (__extension__({                                                           \
        struct Huff_character_frequency_map *const                             \
            character_frequency_map_evaluated_pointer                          \
            = (character_frequency_map_pointer);                               \
        typeof(*key_pointer) const *const character_frequency_map_key_pointer  \
            = (key_pointer);                                                   \
        flat_hash_map_or_insert_with(                                          \
            flat_hash_map_and_modify_with(                                     \
                flat_hash_map_entry_wrap(                                      \
                    &character_frequency_map_evaluated_pointer->map,           \
                    character_frequency_map_key_pointer,                       \
                    (allocator)                                                \
                ),                                                             \
                struct Huff_character_frequency                                \
                    * character_frequency_map_increment_element,               \
                { ++character_frequency_map_increment_element->freq; }         \
            ),                                                                 \
            lazy_evaluated_key_value_default_insertion                         \
        );                                                                     \
    }))

#define huff_foreach_character_frequency(                                      \
    map_pointer, character_frequency_iterator_declaration, codeblock...        \
)                                                                              \
    (__extension__({                                                           \
        struct Huff_character_frequency_map *const character_frequency_map     \
            = (map_pointer);                                                   \
        for (struct Huff_character_frequency *character_frequency_iterator     \
             = CCC_flat_hash_map_begin(&character_frequency_map->map);         \
             character_frequency_iterator                                      \
             != CCC_flat_hash_map_end(&character_frequency_map->map);          \
             character_frequency_iterator = CCC_flat_hash_map_next(            \
                 &character_frequency_map->map, character_frequency_iterator   \
             )) {                                                              \
            character_frequency_iterator_declaration                           \
                = character_frequency_iterator;                                \
            codeblock                                                          \
        }                                                                      \
    }))

#define huff_path_memo_map_entry_and_modify_with(                              \
    map_pointer,                                                               \
    character_key_pointer,                                                     \
    allocator,                                                                 \
    occupied_path_memo_entry_declaration,                                      \
    modification_closure_over_occupied_path_memo_entry...                      \
)                                                                              \
    &(struct { CCC_Flat_hash_map_entry private; }){(__extension__({            \
         struct Huff_path_memo_map *const path_memo_map_pointer                \
             = (map_pointer);                                                  \
         char const *const path_memo_map_character_key                         \
             = (character_key_pointer);                                        \
         CCC_Flat_hash_map_entry path_memo_map_entry                           \
             = *flat_hash_map_and_modify_with(                                 \
                 flat_hash_map_entry_wrap(                                     \
                     &path_memo_map_pointer->map,                              \
                     path_memo_map_character_key,                              \
                     (allocator)                                               \
                 ),                                                            \
                 occupied_path_memo_entry_declaration,                         \
                 modification_closure_over_occupied_path_memo_entry            \
             );                                                                \
         path_memo_map_entry;                                                  \
     }))}.private

#define huff_path_memo_map_or_insert_with(                                     \
    entry_pointer, lazy_evaluated_insertion...                                 \
)                                                                              \
    (__extension__({                                                           \
        flat_hash_map_or_insert_with(entry_pointer, lazy_evaluated_insertion); \
    }))

#define huff_pair_node_buffer_emplace_back(                                    \
    buffer_pointer, allocator, compound_literal...                             \
)                                                                              \
    (__extension__({                                                           \
        struct Huff_pair_node_buffer *const pair_node_buffer_pointer           \
            = (buffer_pointer);                                                \
        struct Huff_pair_node *pair_node_emplaced                              \
            = CCC_flat_buffer_emplace_back(                                    \
                &pair_node_buffer_pointer->buffer, allocator, compound_literal \
            );                                                                 \
        pair_node_emplaced;                                                    \
    }))

/*=============================== LZW Prototypes   ==========================*/

static void lzw_encode(SV_Str_view, CCC_Allocator const *);
static uint64_t lzw_hash(CCC_Key_arguments);
static CCC_Order lzw_compare(CCC_Key_comparator_arguments);
static CCC_Entry lzw_try_insert(
    struct Lzw_map *, struct Lzw_key_value const *, CCC_Allocator const *
);
static bool lzw_push_code(FILE *, Flat_bitset *, Lzw_code);
static void lzw_decode(SV_Str_view, CCC_Allocator const *);
static void lzw_reset_table(struct Lzw_prefix[const]);
static struct Lzw_code_accumulator *
lzw_read_code(FILE *, struct Lzw_code_accumulator *);

/*===========================   Misc Prototypes  ============================*/

static struct File_char peek_first_char(FILE *);
static struct New_file open_compressed_file(SV_Str_view);
static FILE *open_file_copy_in_output(SV_Str_view);
static size_t file_size(FILE *);
static size_t min(size_t, size_t);
static void print_help(void);

/** Asserts even in release mode. Run code in the second argument if needed. */
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

/** Helper iterator to to turn a file pointer into a character iterator,
setting up iteration of each character in the file. Name the iterator and then
use it in the code block. Wrapping the code block in brackets is recommended for
formatting, though not required. */
#define foreach_filechar(                                                      \
    file_pointer, start_index, char_iterator_name, codeblock...                \
)                                                                              \
    do {                                                                       \
        check(fseek(file_pointer, start_index, SEEK_SET) >= 0);                \
        char *linepointer = NULL;                                              \
        defer {                                                                \
            free(linepointer);                                                 \
        }                                                                      \
        size_t len = 0;                                                        \
        ptrdiff_t read = 0;                                                    \
        while ((read = getline(&linepointer, &len, f)) > 0) {                  \
            SV_Str_view const line                                             \
                = {.str = linepointer, .len = (size_t)read};                   \
            for (char const *char_iterator_name = SV_begin(line);              \
                 char_iterator_name != SV_end(line);                           \
                 char_iterator_name = SV_next(char_iterator_name)) {           \
                codeblock                                                      \
            }                                                                  \
        }                                                                      \
    } while (0)

/*===========================   Argument Handling  ==========================*/

static Algorithm(*encoders[ALGORITHM_COUNT]) = {
    huff_encode,
    lzw_encode,
};

static Algorithm(*decoders[ALGORITHM_COUNT]) = {
    huff_decode,
    lzw_decode,
};

int
main(int argc, char **argv) {
    if (argc < 2) {
        return 0;
    }
    struct Zip_actions todo = {};
    for (int arg = 1; arg < argc; ++arg) {
        SV_Str_view const sv_arg = SV_from_terminated(argv[arg]);
        if (SV_starts_with(sv_arg, SV_from("-h"))) {
            print_help();
            return 0;
        }
        if (SV_starts_with(sv_arg, SV_from("-c="))) {
            SV_Str_view const raw_file = SV_substr(
                sv_arg, SV_find(sv_arg, 0, SV_from("=")) + 1, SV_len(sv_arg)
            );
            check(!SV_is_empty(raw_file));
            todo.zip = raw_file;
        } else if (SV_starts_with(sv_arg, SV_from("-d="))) {
            SV_Str_view const raw_file = SV_substr(
                sv_arg, SV_find(sv_arg, 0, SV_from("=")) + 1, SV_len(sv_arg)
            );
            check(!SV_is_empty(raw_file));
            todo.unzip = raw_file;
        } else if (SV_starts_with(sv_arg, SV_from("-a=huffman"))) {
            todo.algorithm = HUFFMAN;
        } else if (SV_starts_with(sv_arg, SV_from("-a=lzw"))) {
            todo.algorithm = LZW;
        }
    }
    if (!SV_is_empty(todo.zip)) {
        encoders[todo.algorithm](todo.zip, &std_allocator);
    }
    if (!SV_is_empty(todo.unzip)) {
        decoders[todo.algorithm](todo.unzip, &std_allocator);
    }
    return 0;
}

/*=========================    Huffman Compression  =========================*/

/*=========================     Huffman Encoding    =========================*/

/** Zips the requested file via Huffman Encoding into the output directory. The
compressed file has a header that can be used to reconstruct the data. */
static void
huff_encode(
    SV_Str_view const to_compress, CCC_Allocator const *const allocator
) {
    FILE *const f = fopen(SV_begin(to_compress), "r");
    if (!f) {
        (void)fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
    defer {
        (void)fclose(f);
    }
    size_t const fsize = file_size(f);
    printf("Zip %s (%zu bytes).\n", SV_begin(to_compress), fsize);
    struct Huff_tree tree = huff_build_encoding_tree(f, allocator);
    defer {
        huff_free_encode_tree(&tree, allocator);
    }
    if (!tree.root) {
        (void)fprintf(stderr, "empty encoding tree cannot zip anything\n");
        return;
    }
    struct Huff_encoding encoding = {
        .magic = CCCZ_MAGIC,
        .file_bits = huff_build_encoding_bitq(f, &tree, allocator),
        .blueprint = huff_compress_tree(&tree, allocator),
    };
    defer {
        huff_bitq_clear_and_free(&encoding.file_bits, allocator);
        huff_bitq_clear_and_free(&encoding.blueprint.tree_paths, allocator);
        string_arena_free(&encoding.blueprint.arena, allocator);
    }
    encoding.leaves_minus_one = (uint8_t)encoding.blueprint.leaf_string.len - 1,
    encoding.file_bits_count = huff_bitq_count(&encoding.file_bits),
    huff_write_to_file(to_compress, fsize, &encoding);
}

/** Builds the Huffman Encoding tree that will allow us to encode file bytes
and later compress the tree structure. The tree is formed by pairing the two
least frequently occurring characters at a new root that is the sum of their
frequencies. This process takes O(log(N)) iterations because we pair nodes
at each step.

Because the priority queue is a min queue this means that high frequency
elements will be paired later in the algorithm and thus be closer to the root of
the encoding tree. */
static struct Huff_tree
huff_build_encoding_tree(FILE *const f, CCC_Allocator const *const allocator) {
    struct Huff_tree ret = {};
    struct Huff_pair_node_priority_queue pairings
        = huff_build_encoding_priority_queue(f, &ret, allocator);
    defer {
        (void)clear_and_free(
            &pairings.priority_queue, &(CCC_Destructor){}, allocator
        );
    }
    while (count(&pairings.priority_queue).count >= 2) {
        /* Small elements and we need the pair so we can't hold references. */
        struct Huff_pair_node const zero
            = *huff_pair_node_priority_queue_front(&pairings);
        CCC_Result r = huff_pair_node_priority_queue_pop(&pairings);
        check(r == CCC_RESULT_OK);
        struct Huff_pair_node const one
            = *huff_pair_node_priority_queue_front(&pairings);
        r = huff_pair_node_priority_queue_pop(&pairings);
        check(r == CCC_RESULT_OK);
        struct Huff_node const *const internal_one
            = huff_node_buffer_emplace_back(
                &ret.tree_storage,
                allocator,
                (struct Huff_node){
                    .link = {zero.node_index, one.node_index},
                }
            );
        size_t const pair_root
            = huff_node_buffer_index(&ret.tree_storage, internal_one);
        check(internal_one);
        huff_node_at(&ret, zero.node_index)->parent = pair_root;
        huff_node_at(&ret, one.node_index)->parent = pair_root;
        struct Huff_pair_node const *const pushed
            = huff_pair_node_priority_queue_push(
                &pairings,
                &(struct Huff_pair_node){
                    .frequency = zero.frequency + one.frequency,
                    .node_index = pair_root,
                },
                allocator
            );
        check(pushed);
        ret.root = pair_root;
    }
    return ret;
}

/** Returns a min priority queue sorted by frequency meaning the least frequent
character will be the root. The priority queue is built in O(N) time. It is
the caller's responsibility to free the priority queue memory when ready. */
static struct Huff_pair_node_priority_queue
huff_build_encoding_priority_queue(
    FILE *const f,
    struct Huff_tree *const tree,
    CCC_Allocator const *const allocator
) {
    struct Huff_character_frequency_map frequencies = {
        flat_hash_map_default(
            struct Huff_character_frequency,
            ch,
            (CCC_Hasher){.hash = hash_char, .compare = char_order}
        ),
    };
    defer {
        (void)clear_and_free(&frequencies.map, &(CCC_Destructor){}, allocator);
    }
    foreach_filechar(f, 0, c, {
        struct Huff_character_frequency *const ins
            = huff_character_frequency_map_increment_entry_or_insert_with(
                &frequencies,
                c,
                allocator,
                (struct Huff_character_frequency){
                    .ch = *c,
                    .freq = 1,
                }
            );
        check(ins);
    });
    size_t const leaves = count(&frequencies.map).count;
    check(leaves >= 2);
    tree->num_leaves = leaves;
    tree->num_nodes = (2 * leaves) - 1;
    /* For a Flat_buffer based tree 0 is the NULL node so we can't have actual
       data we want at that index in the tree. */
    tree->tree_storage = (struct Huff_node_buffer){
        flat_buffer_from(
            *allocator, tree->num_nodes + 1, (struct Huff_node[]){{}}
        ),
    };
    check(count(&tree->tree_storage.buffer).count == 1);
    /* Use a Flat_buffer to push back elements we will heapify at the end. */
    struct Huff_pair_node_buffer flat_priority_queue_storage = {
        flat_buffer_with_capacity(
            struct Huff_pair_node,
            *allocator,
            flat_hash_map_count(&frequencies.map).count
        ),
    };
    check(flat_buffer_capacity(&flat_priority_queue_storage.buffer).count);
    huff_foreach_character_frequency(
        &frequencies, struct Huff_character_frequency const *const i, {
            struct Huff_node const *const node = huff_node_buffer_emplace_back(
                &tree->tree_storage,
                &(CCC_Allocator){},
                (struct Huff_node){.ch = i->ch}
            );
            check(node);
            size_t const index
                = huff_node_buffer_index(&tree->tree_storage, node);
            struct Huff_pair_node const *const pushed
                = huff_pair_node_buffer_emplace_back(
                    &flat_priority_queue_storage,
                    &(CCC_Allocator){},
                    (struct Huff_pair_node){
                        .frequency = i->freq,
                        .node_index = index,
                    }
                );
            check(pushed);
        }
    );
    /* Now we steal the buffer's memory and heapify the data in O(N) time rather
       than pushing each element. */
    return (struct Huff_pair_node_priority_queue){
        flat_priority_queue_heapify(
            struct Huff_pair_node,
            CCC_ORDER_LESSER,
            (CCC_Comparator){.compare = order_freqs},
            capacity(&flat_priority_queue_storage.buffer).count,
            count(&flat_priority_queue_storage.buffer).count,
            begin(&flat_priority_queue_storage.buffer)
        ),
    };
}

/** Returns the bit queue representing the bit path to every character in the
file. This queue represents each byte of the file in order; the first path in
at the front of the queue represents the first character. */
static struct Bit_queue
huff_build_encoding_bitq(
    FILE *const f,
    struct Huff_tree *const tree,
    CCC_Allocator const *const allocator
) {
    struct Bit_queue character_paths = {
        .bs = flat_bitset_default(),
    };
    /* By memoizing known bit sequences we can save significant time by not
       performing a DFS over the tree. This is especially helpful for large
       alphabets aka trees with many leaves. An array of 0-256 elements as a map
       could achieve the same result faster but it would waste much more space.
       It is rare to have a file use all 256 possible character values. */
    struct Huff_path_memo_map path_memos = {
        CCC_flat_hash_map_with_capacity(
            struct Huff_path_memo,
            ch,
            ((CCC_Hasher){
                .hash = hash_char,
                .compare = path_memo_order,
            }),
            *allocator,
            tree->num_leaves
        ),
    };
    defer {
        (void)clear_and_free(&path_memos.map, &(CCC_Destructor){}, allocator);
    }
    check(capacity(&path_memos.map).count >= tree->num_leaves);
    foreach_filechar(f, 0, c, {
        /* The entry interface allows us to express the complete memoization
           technique with a single query into our hash map. We either find the
           element and re-record the path in the path bit queue or we insert a
           newly formed path and remember where it starts and ends in the bit
           queue. They _with variants offer lazy evaluation of the final
           argument. For the and_modify_with step we only execute if the entry
           is occupied. For the or_insert_with step we only execute if the entry
           is vacant. This means that logic or expensive function calls execute
           conditionally. For or_insert_with this is important because appending
           the bit queue is a side-effect. */
        struct Huff_path_memo const *const p
            = huff_path_memo_map_or_insert_with(
                huff_path_memo_map_entry_and_modify_with(
                    &path_memos,
                    c,
                    allocator,
                    struct Huff_path_memo const *const memo,
                    {
                        size_t const end
                            = memo->bitq_start_index + memo->path_len;
                        for (size_t i = memo->bitq_start_index; i < end; ++i) {
                            (void)huff_bitq_push_back(
                                &character_paths,
                                huff_bitq_test(&character_paths, i),
                                allocator
                            );
                        }
                    }
                ),
                huff_append_tree_path_to_bitq(
                    tree, &character_paths, *c, allocator
                )
            );
        check(p);
    });
    return character_paths;
}

/** Finds the path to the current character in the encoding tree and appends
it to the bit queue using the encoding tree. This function modifies the tree
nodes by altering their iterator field during the DFS, but it restores all nodes
to their original state before returning. The path memo representing this new
path's start position and length in the bit queue is returned. Assumes the
character can be found, otherwise exits the program. */
static struct Huff_path_memo
huff_append_tree_path_to_bitq(
    struct Huff_tree *const tree,
    struct Bit_queue *const bq,
    char const c,
    CCC_Allocator const *const allocator
) {
    struct Huff_path_memo path = {
        .ch = c,
        .bitq_start_index = huff_bitq_count(bq),
    };
    size_t cur = tree->root;
    /* An iterative depth first search is convenient because the bit path in
       the queue can represent the exact path we are currently on. Just be
       sure to backtrack up the path to cleanup iterators. */
    while (cur) {
        struct Huff_node *const node = huff_node_at(tree, cur);
        /* This is the leaf we want. */
        if (!node->link[1] && node->ch == c) {
            break;
        }
        /* Wrong leaf or we have explored both subtrees of an internal node. */
        if (!node->link[1] || node->child_index >= HUFF_MAX_CHILD_COUNT) {
            node->child_index = 0;
            cur = node->parent;
            huff_bitq_pop_back(bq);
            continue;
        }
        /* Depth progression of depth first search. */
        check(node->child_index <= CCC_TRUE);
        huff_bitq_push_back(bq, (CCC_Tribool)node->child_index, allocator);
        /* During backtracking this helps us know which child subtree needs to
           be explored or if we are done and can continue backtracking. */
        cur = node->link[node->child_index++];
    }
    /* Cleanup because we now have the correct path. */
    while (cur) {
        huff_node_at(tree, cur)->child_index = 0;
        cur = huff_parent_index(tree, cur);
    }
    path.path_len = huff_bitq_count(bq) - path.bitq_start_index;
    check(path.path_len);
    return path;
}

/** Compresses the Huffman tree by creating its representation as a Pre-Order
traversal of the tree. The traversal is entered into a bit queue. We also push
the leaves to a string as they are encountered during this traversal. By the
end of the operation, we have a bit queue of our traversal where every internal
node encountered on the way down is a 1 and every leaf is a 0. We also have a
string of leaf characters that we encountered in order. */
static struct Huff_compressed_tree
huff_compress_tree(
    struct Huff_tree *const tree, CCC_Allocator const *const allocator
) {
    struct Huff_compressed_tree ret = {
        .tree_paths = {
            .bs = flat_bitset_with_capacity(*allocator, tree->num_nodes),
        },
        .arena = string_arena_create(HUFF_START_STRING_ARENA_CAP, allocator),
    };
    check(ret.arena.arena);
    check(flat_bitset_capacity(&ret.tree_paths.bs).count >= tree->num_nodes);
    ret.leaf_string = string_arena_allocate(&ret.arena, 0, allocator);
    size_t cur = tree->root;
    /* To properly emulate a recursive Pre-Order traversal with iteration we
       use the parent field for backtracking and an iterator for caching and
       progression. */
    while (cur) {
        struct Huff_node *const node = huff_node_at(tree, cur);
        if (!node->link[1]) {
            /* A leaf is always pushed because it is only seen once. */
            huff_bitq_push_back(&ret.tree_paths, CCC_FALSE, allocator);
            enum String_arena_result const ch_push = string_arena_push_back(
                &ret.arena, &ret.leaf_string, node->ch, allocator
            );
            check(ch_push == STRING_ARENA_OK);
            cur = node->parent;
        } else if (node->child_index < HUFF_MAX_CHILD_COUNT) {
            /* We only push internal 1 nodes the first time on the way down. We
               still need to access the second child so don't push a bit when we
               are simply progressing to the next child subtree. */
            if (node->child_index == 0) {
                huff_bitq_push_back(&ret.tree_paths, CCC_TRUE, allocator);
            }
            cur = node->link[node->child_index++];
        } else {
            /* Both child subtrees have been explored, so cleanup/backtrack. */
            node->child_index = 0;
            cur = node->parent;
        }
    }
    return ret;
}

/** Writes all encoded information to a file with the help of a header for
later file reconstruction. */
static void
huff_write_to_file(
    SV_Str_view const original_filepath,
    size_t const original_filesize,
    struct Huff_encoding *const header
) {
    struct New_file cccz = open_compressed_file(original_filepath);
    check(cccz.file, (void)fprintf(stderr, "%s", strerror(errno)););
    defer {
        (void)fclose(cccz.file);
    }

    /* When writing bytes we need every bit to make it so use many checks. */
    size_t writ
        = huff_write_bytes(cccz.file, &header->magic, sizeof(header->magic));
    check(writ == sizeof(header->magic));
    writ = huff_write_bytes(
        cccz.file, &header->leaves_minus_one, sizeof(header->leaves_minus_one)
    );
    check(writ == sizeof(header->leaves_minus_one));
    char const *leaf_string = string_arena_at(
        &header->blueprint.arena, &header->blueprint.leaf_string
    );
    writ = huff_write_bytes(
        cccz.file, leaf_string, header->leaves_minus_one + 1
    );
    check(writ == (size_t)(header->leaves_minus_one + 1));
    writ = huff_write_bytes(
        cccz.file, &header->file_bits_count, sizeof(header->file_bits_count)
    );
    check(writ == sizeof(header->file_bits_count));
    huff_write_bitq(cccz.file, &header->blueprint.tree_paths);
    huff_write_bitq(cccz.file, &header->file_bits);

    size_t const cccz_size = file_size(cccz.file);
    printf(
        "Zipped file %s has compression ratio of %.2lf%% (%zu bytes).\n",
        cccz.path.str,
        (100.0 * (double)cccz_size) / (double)(original_filesize),
        cccz_size
    );
}

/** Writes a queue of bits to a file. Some bits may be unused in the last byte
written to the file but this should not be a problem because the header can
tell us the number of tree bits and file text bits we write to the file. The
information written to file is on a per byte basis where every bit of the byte
represents a bit in the bit queue. */
static void
huff_write_bitq(FILE *const cccz, struct Bit_queue *const bq) {
    static_assert(sizeof(uint8_t) == sizeof(CCC_Tribool));
    uint8_t buf = 0;
    uint8_t i = 0;
    while (!huff_bitq_is_empty(bq)) {
        buf |= (uint8_t)(huff_bitq_pop_front(bq) << i);
        ++i;
        if (i >= CHAR_BIT) {
            size_t const byte = huff_write_bytes(cccz, &buf, sizeof(buf));
            check(byte);
            buf = 0;
            i = 0;
        }
    }
    if (i) {
        size_t const byte = huff_write_bytes(cccz, &buf, sizeof(buf));
        check(byte);
    }
}

/*=========================     Huffman Decoding    =========================*/

/** Unzips the requested file. The file must end in the .cccz suffix. This file
is reconstructed and a copy of the original text is written to the output
directory as a new file with the same name. */
static void
huff_decode(SV_Str_view unzip, CCC_Allocator const *const allocator) {
    /* First we verify the compressed file is correct before creating new. */
    struct Huff_encoding he = huff_read_from_file(unzip, allocator);
    defer {
        huff_bitq_clear_and_free(&he.file_bits, allocator);
        huff_bitq_clear_and_free(&he.blueprint.tree_paths, allocator);
        string_arena_free(&he.blueprint.arena, allocator);
    }
    if (!he.file_bits_count) {
        return;
    }
    struct Huff_tree tree = huff_reconstruct_tree(&he.blueprint, allocator);
    defer {
        huff_free_encode_tree(&tree, allocator);
    }
    if (!tree.root) {
        return;
    }
    FILE *const copy_of_original = open_file_copy_in_output(unzip);
    defer {
        (void)fclose(copy_of_original);
    }
    huff_reconstruct_text(copy_of_original, &tree, &he.file_bits);
    printf(
        "Unzipped %s (%zu bytes) to output/.\n",
        SV_begin(unzip),
        file_size(copy_of_original)
    );
}

/** Reconstructs the Huffman encoding queues from the header of the specified
file. Once complete this function returns all information needed to reconstruct
the tree and write out a copy of the original file to the output directory. */
static struct Huff_encoding
huff_read_from_file(
    SV_Str_view const unzip, CCC_Allocator const *const allocator
) {
    CCC_Tribool has_suffix = SV_ends_with(unzip, SV_from(".cccz"));
    check(has_suffix);
    FILE *const cccz = fopen(SV_begin(unzip), "r");
    defer {
        if (cccz) {
            (void)fclose(cccz);
        }
    }
    check(cccz, (void)fprintf(stderr, "%s\n", strerror(errno)););
    printf("Unzip %s (%zu bytes).\n", SV_begin(unzip), file_size(cccz));
    struct Huff_encoding encoding = {
        .file_bits = {
            .bs = flat_bitset_default(),
        },
        .blueprint = {
            .arena = string_arena_create(HUFF_START_STRING_ARENA_CAP, allocator),
            .tree_paths = {
                .bs = flat_bitset_default(),
            },
        },
    };
    size_t read = huff_readbytes(
        cccz, (unsigned char *)&encoding.magic, sizeof(encoding.magic)
    );
    check(read == sizeof(encoding.magic) && encoding.magic == CCCZ_MAGIC);
    read = huff_readbytes(
        cccz, &encoding.leaves_minus_one, sizeof(encoding.leaves_minus_one)
    );
    check(read == sizeof(encoding.leaves_minus_one));
    struct String_arena *const arena = &encoding.blueprint.arena;
    struct String_offset *const leaves = &encoding.blueprint.leaf_string;
    /* Add 2: one for being minus 1 already and one for NULL terminator. */
    *leaves = string_arena_allocate(
        arena, encoding.leaves_minus_one + 2, allocator
    );
    check(!leaves->error);
    read = huff_readbytes(cccz, string_arena_at(arena, leaves), leaves->len);
    check(read == (size_t)(encoding.leaves_minus_one + 1));
    read = huff_readbytes(
        cccz, &encoding.file_bits_count, sizeof(encoding.file_bits_count)
    );
    check(read == sizeof(encoding.file_bits_count));
    /* The pairing method we used while building the tree makes this true. */
    size_t const tree_path_bits = (leaves->len * 2) - 1;
    huff_fill_bitq(
        cccz, &encoding.blueprint.tree_paths, tree_path_bits, allocator
    );
    huff_fill_bitq(
        cccz, &encoding.file_bits, encoding.file_bits_count, allocator
    );
    return encoding;
}

/** Reconstructs a Huffman encoding tree based on the blueprint provided. The
tree is constructed in linear time and constant space additional to the nodes
being allocated. */
static struct Huff_tree
huff_reconstruct_tree(
    struct Huff_compressed_tree *const blueprint,
    CCC_Allocator const *const allocator
) {
    size_t const bq_count = huff_bitq_count(&blueprint->tree_paths);
    struct Huff_tree tree = {
        /* 0 index is NULL so real data can't be there. */
        .tree_storage = {CCC_flat_buffer_from(
            *allocator,
            bq_count,
            (struct Huff_node[]){
                {}, /* nil */
                {}, /* root */
            }
        ),},
        /* By creating the root outside of the main loop we can be sure we
           always have valid prev node. Don't need to check on every loop
           iteration. */
        .root = 1,
        .num_nodes = bq_count,
    };
    check(!flat_buffer_is_empty(&tree.tree_storage.buffer));
    (void)huff_bitq_pop_front(&blueprint->tree_paths);
    size_t parent = tree.root;
    size_t current = 0;
    char const *leaves
        = string_arena_at(&blueprint->arena, &blueprint->leaf_string);
    while (!huff_bitq_is_empty(&blueprint->tree_paths)) {
        CCC_Tribool is_internal_node = CCC_TRUE;
        if (!current) {
            is_internal_node = huff_bitq_pop_front(&blueprint->tree_paths);
            struct Huff_node *const pushed = huff_node_buffer_emplace_back(
                &tree.tree_storage,
                allocator,
                (struct Huff_node){.parent = parent}
            );
            current = huff_node_buffer_index(&tree.tree_storage, pushed);
            /* Get the parent reference after the buffer push in case the
               buffer resized to accommodate push. */
            struct Huff_node *const parent_r = huff_node_at(&tree, parent);
            parent_r->link[parent_r->child_index++] = current;
            if (!is_internal_node) {
                pushed->ch = *leaves;
                ++leaves;
                ++tree.num_leaves;
            }
        }
        struct Huff_node *const node_r = huff_node_at(&tree, current);
        /* An internal node has further child subtrees to build. */
        if (is_internal_node && node_r->child_index < HUFF_MAX_CHILD_COUNT) {
            parent = current;
            current = node_r->link[node_r->child_index];
            continue;
        }
        /* Backtrack. A leaf or internal node with both children built. */
        current = parent;
        parent = huff_parent_index(&tree, parent);
    }
    return tree;
}

/*=========================     LZW Compression     =========================*/

/** Implements LZW encoding using a flat hash map. This approach saves some
RAM usage over a flat table and exercises the hash map. This algorithm also
implements clear codes which flush and reset the hash map. */
static void
lzw_encode(
    SV_Str_view const to_compress, CCC_Allocator const *const allocator
) {
    FILE *const f = fopen(SV_begin(to_compress), "r");
    if (!f) {
        (void)fprintf(stderr, "%s\n", strerror(errno));
        return;
    }
    defer {
        (void)fclose(f);
    }
    size_t const fsize = file_size(f);
    struct Lzw_map prefix_map = {
        CCC_flat_hash_map_with_capacity(
            struct Lzw_key_value,
            key,
            ((CCC_Hasher){.hash = lzw_hash, .compare = lzw_compare}),
            *allocator,
            LZW_STARTING_PREFIX_COUNT
        ),
    };
    struct New_file const compression_location
        = open_compressed_file(to_compress);
    check(compression_location.file);
    printf("Zip %s (%zu bytes).\n", SV_begin(to_compress), fsize);
    defer {
        CCC_flat_hash_map_clear_and_free(
            &prefix_map.map, &(CCC_Destructor){}, allocator
        );
        (void)fclose(compression_location.file);
    }
    {
        uint16_t code = LZW_NEW_CODE_START;
        struct File_char const first_char = peek_first_char(f);
        check(!first_char.error);
        uint16_t prefix = first_char.c;
        CCC_Flat_bitset sliding_code_window = flat_bitset_with_storage(
            0, (CCC_Bit[sizeof(Lzw_code) * CHAR_BIT]){}
        );
        foreach_filechar(f, 1, c, {
            CCC_Entry const e = lzw_try_insert(
                &prefix_map,
                &(struct Lzw_key_value){
                    .key = {
                        .prefix = prefix,
                        .next_char = (unsigned char)*c,
                    },
                    .code_value = code,
                },
                allocator
            );
            if (occupied(&e)) {
                // Make the prefix longer so we don't push to file yet.
                struct Lzw_key_value const *const seen = unwrap(&e);
                prefix = seen->code_value;
            } else {
                // Prefix has reached max unique length so we push to file.
                lzw_push_code(
                    compression_location.file, &sliding_code_window, prefix
                );
                ++code;
                if (code >= LZW_CODE_LIMIT) {
                    lzw_push_code(
                        compression_location.file,
                        &sliding_code_window,
                        LZW_CLEAR_CODE
                    );
                    CCC_flat_hash_map_clear(
                        &prefix_map.map, &(CCC_Destructor){}
                    );
                    code = LZW_NEW_CODE_START;
                }
                prefix = (unsigned char)*c;
            }
        });
        lzw_push_code(compression_location.file, &sliding_code_window, prefix);
        bool wrote = lzw_push_code(
            compression_location.file, &sliding_code_window, LZW_STOP_CODE
        );
        while (!wrote) {
            wrote = lzw_push_code(
                compression_location.file, &sliding_code_window, 0
            );
        }
    }
    size_t const cccz_size = file_size(compression_location.file);
    printf(
        "Zipped file %s has compression ratio of %.2lf%% (%zu bytes).\n",
        SV_begin(compression_location.path),
        (100.0 * (double)cccz_size) / (double)(fsize),
        cccz_size
    );
}

/** Pushes the specified code into the fixed size bit set. If the fixed size
bit set reaches the size of the Lzw_code's bit width it's contents are written
to the file within that types number of bytes and the bit set is cleared. */
static bool
lzw_push_code(
    FILE *const file,
    Flat_bitset *const sliding_code_window,
    Lzw_code const code
) {
    bool wrote_to_file = false;
    for (Lzw_code b = 0; b < LZW_FILE_CODE_BIT_COUNT; ++b) {
        CCC_Result const push_result = flat_bitset_push_back(
            sliding_code_window,
            (CCC_Tribool){(code >> b) & 1},
            &(CCC_Allocator){}
        );
        check(push_result == CCC_RESULT_OK);
        if (flat_bitset_count(sliding_code_window).count
            == (sizeof(Lzw_code) * CHAR_BIT)) {
            check(
                fwrite(
                    flat_bitset_data(sliding_code_window),
                    sizeof(Lzw_code),
                    1,
                    file
                )
                == 1
            );
            flat_bitset_clear(sliding_code_window);
            wrote_to_file = true;
        }
    }
    return wrote_to_file;
}

/*=========================     LZW Decompression   =========================*/

/** Implements LZW decoding using a flat table and bit accumulator. The approach
expects clear and stop codes while it rebuilds strings by walking prefix trees
for characters. */
static void
lzw_decode(SV_Str_view decode, CCC_Allocator const *const) {
    CCC_Tribool has_suffix = SV_ends_with(decode, SV_from(".cccz"));
    check(has_suffix);
    FILE *const cccz = fopen(SV_begin(decode), "r");
    defer {
        if (cccz) {
            (void)fclose(cccz);
        }
    }
    check(cccz, (void)fprintf(stderr, "%s\n", strerror(errno)););
    check(file_size(cccz) % sizeof(Lzw_code) == 0);
    printf("Unzip %s (%zu bytes).\n", SV_begin(decode), file_size(cccz));
    FILE *const copy_of_original = open_file_copy_in_output(decode);
    static struct Lzw_prefix code_table[LZW_CODE_LIMIT];
    lzw_reset_table(code_table);
    Lzw_code next_code = LZW_NEW_CODE_START;
    Lzw_code old_code = 0;
    static unsigned char current_string[LZW_CODE_LIMIT];
    size_t current_string_index = LZW_CODE_LIMIT;
    struct Lzw_code_accumulator *current_code
        = &(struct Lzw_code_accumulator){};
    if (lzw_read_code(cccz, current_code)) {
        old_code = current_code->current_code;
        current_string[--current_string_index] = (unsigned char)old_code;
        [[maybe_unused]] size_t const written = fwrite(
            &current_string[current_string_index], 1, 1, copy_of_original
        );
        check(written == 1);
    }
    while ((current_code = lzw_read_code(cccz, current_code))) {
        if (current_code->current_code == LZW_STOP_CODE) {
            return;
        }
        if (current_code->current_code == LZW_CLEAR_CODE) {
            next_code = LZW_NEW_CODE_START;
            lzw_reset_table(code_table);
            if (!lzw_read_code(cccz, current_code)) {
                return;
            }
            old_code = current_code->current_code;
            current_string_index = LZW_CODE_LIMIT;
            current_string[--current_string_index] = (unsigned char)old_code;
            // Write the literal character following CLEAR_CODE to output
            [[maybe_unused]] size_t const written = fwrite(
                &current_string[current_string_index], 1, 1, copy_of_original
            );
            check(written == 1);
            continue;
        }
        if (current_code->current_code >= next_code) {
            Lzw_code walk = old_code;
            /* We know we will be adding the first character of K to the final
               position of the buffer so leave room for it now. */
            current_string_index = LZW_CODE_LIMIT - 1;
            while (walk != LZW_NULL_PARENT_CODE) {
                current_string[--current_string_index]
                    = code_table[walk].next_char;
                walk = code_table[walk].prefix;
            }
            unsigned char const first_char
                = current_string[current_string_index];
            current_string[LZW_CODE_LIMIT - 1] = first_char;
        } else {
            current_string_index = LZW_CODE_LIMIT;
            Lzw_code walk = current_code->current_code;
            while (walk != LZW_NULL_PARENT_CODE) {
                current_string[--current_string_index]
                    = code_table[walk].next_char;
                walk = code_table[walk].prefix;
            }
        }
        [[maybe_unused]] size_t const written = fwrite(
            &current_string[current_string_index],
            1,
            LZW_CODE_LIMIT - current_string_index,
            copy_of_original
        );
        check(written == LZW_CODE_LIMIT - current_string_index);
        if (next_code < LZW_CODE_LIMIT) {
            code_table[next_code].prefix = old_code;
            code_table[next_code].next_char
                = current_string[current_string_index];
            ++next_code;
        }
        old_code = current_code->current_code;
    }
}

/** Reads 12 bit codes into an accumulator and provides the currently obtained
next code from the file bytes. Every file is expected to have a number of bytes
divisible by the Lzw_code type. */
static inline struct Lzw_code_accumulator *
lzw_read_code(
    FILE *const file, struct Lzw_code_accumulator *const accumulator
) {
    if (accumulator->bit_count < LZW_FILE_CODE_BIT_COUNT) {
        uint16_t block = 0;
        size_t const read_count = fread(&block, sizeof(Lzw_code), 1, file);
        if (read_count == 0) {
            return NULL;
        }
        accumulator->accumulator |= ((uint32_t)block << accumulator->bit_count);
        accumulator->bit_count += (sizeof(Lzw_code) * CHAR_BIT);
    }
    accumulator->current_code
        = (uint16_t)(accumulator->accumulator & LZW_CODE_WIDTH_MASK);
    accumulator->accumulator >>= LZW_FILE_CODE_BIT_COUNT;
    accumulator->bit_count -= LZW_FILE_CODE_BIT_COUNT;
    return accumulator;
}

static inline void
lzw_reset_table(struct Lzw_prefix prefix_table[const]) {
    for (Lzw_code i = 0; i < LZW_STARTING_PREFIX_COUNT; ++i) {
        prefix_table[i] = (struct Lzw_prefix){
            .prefix = LZW_NULL_PARENT_CODE,
            .next_char = (unsigned char)i,
        };
    }
}

/*=========================    File Read and Write  =========================*/

/** Reconstructs the text by following the bit paths specified in the file
text bit queue. Bits are written on a per byte basis to the file and each
bit of the byte corresponds to a bit in the bit queue. */
void
huff_reconstruct_text(
    FILE *const f,
    struct Huff_tree const *const tree,
    struct Bit_queue *const bq
) {
    size_t cur = tree->root;
    while (!huff_bitq_is_empty(bq)) {
        /* All paths started from the root during encoding and chose a direction
           first so popping is OK here. Root 1 node never was pushed to q. */
        CCC_Tribool const bit = huff_bitq_pop_front(bq);
        check(bit != CCC_TRIBOOL_ERROR);
        cur = huff_branch_index(tree, cur, (uint8_t)bit);
        if (!huff_branch_index(tree, cur, 1)) {
            char const c = huff_char_index(tree, cur);
            size_t const byte = huff_write_bytes(f, &c, sizeof(c));
            check(byte);
            cur = tree->root;
        }
    }
}

/** Fills a bit queue from a file given an expected number of bits. The bits
are read in on a per byte basis where every bit of a file byte represents a bit
in the bit queue. Exits if not enough bits are found in the file. */
static void
huff_fill_bitq(
    FILE *const f,
    struct Bit_queue *const bq,
    size_t expected_bits,
    CCC_Allocator const *const allocator
) {
    uint8_t buf = 0;
    uint8_t i = CHAR_BIT;
    while (expected_bits--) {
        if (i == CHAR_BIT) {
            size_t const byte = huff_readbytes(f, &buf, sizeof(buf));
            check(byte);
            i = 0;
        }
        static_assert(sizeof(CCC_Tribool) == sizeof(uint8_t));
        CCC_Tribool const bit = (buf & ((uint8_t)1 << i)) != 0;
        huff_bitq_push_back(bq, bit, allocator);
        ++i;
    }
}

/*=========================      Huffman Helpers    =========================*/

static inline size_t
huff_node_buffer_index(
    struct Huff_node_buffer const *const buffer,
    struct Huff_node const *const node
) {
    return CCC_flat_buffer_index(&buffer->buffer, node).count;
}

static inline struct Huff_pair_node *
huff_pair_node_priority_queue_push(
    struct Huff_pair_node_priority_queue *const queue,
    struct Huff_pair_node const *const pair,
    CCC_Allocator const *const allocator
) {
    return CCC_flat_priority_queue_push(
        &queue->priority_queue, pair, &(struct Huff_pair_node){}, allocator
    );
}

static inline CCC_Result
huff_pair_node_priority_queue_pop(
    struct Huff_pair_node_priority_queue *const queue
) {
    return CCC_flat_priority_queue_pop(
        &queue->priority_queue, &(struct Huff_pair_node){}
    );
}

static inline struct Huff_pair_node *
huff_pair_node_priority_queue_front(
    struct Huff_pair_node_priority_queue const *const queue
) {
    return CCC_flat_priority_queue_front(&queue->priority_queue);
}

static struct Huff_node *
huff_node_at(struct Huff_tree const *const t, size_t const node) {
    return (
        (struct Huff_node *)CCC_flat_buffer_at(&t->tree_storage.buffer, node)
    );
}

static size_t
huff_branch_index(
    struct Huff_tree const *const t, size_t const node, uint8_t const dir
) {
    return ((struct Huff_node *)
                CCC_flat_buffer_at(&t->tree_storage.buffer, node))
        ->link[dir];
}

static size_t
huff_parent_index(struct Huff_tree const *const t, size_t node) {
    return ((struct Huff_node *)
                CCC_flat_buffer_at(&t->tree_storage.buffer, node))
        ->parent;
}

static char
huff_char_index(struct Huff_tree const *const t, size_t const node) {
    return ((struct Huff_node *)
                CCC_flat_buffer_at(&t->tree_storage.buffer, node))
        ->ch;
}

/** Frees all encoding nodes from the tree provided. */
static void
huff_free_encode_tree(
    struct Huff_tree *tree, CCC_Allocator const *const allocator
) {
    CCC_Result const r = CCC_flat_buffer_clear_and_free(
        &tree->tree_storage.buffer, &(CCC_Destructor){}, allocator
    );
    check(r == CCC_RESULT_OK);
    *tree = (struct Huff_tree){};
}

/* NOLINTBEGIN(*misc-no-recursion) */

[[maybe_unused]] static void
huff_print_tree(
    struct Huff_tree const *const tree,
    size_t const node,
    CCC_Allocator const *const allocator
) {
    if (!tree) {
        return;
    }
    huff_print_node(tree, node);
    huff_print_inner_tree(
        tree, huff_branch_index(tree, node, 1), BRANCH, "", allocator
    );
    huff_print_inner_tree(
        tree, huff_branch_index(tree, node, 0), LEAF, "", allocator
    );
}

[[maybe_unused]] static void
huff_print_inner_tree(
    struct Huff_tree const *const tree,
    size_t const node,
    enum Print_branch const branch_type,
    char const *const prefix,
    CCC_Allocator const *const allocator
) {
    if (!node) {
        return;
    }
    printf("%s", prefix);
    printf("%s", branch_type == LEAF ? " └──" : " ├──");

    huff_print_node(tree, node);

    char *str = NULL;
    size_t const string_length = (size_t)snprintf(
        NULL, 0, "%s%s", prefix, branch_type == LEAF ? "     " : " │   "
    );
    if (string_length > 0) {
        str = allocator->allocate((CCC_Allocator_arguments){
            .input = NULL,
            .bytes = string_length + 1,
            .alignment = alignof(char),
            .context = allocator->context,
        });
        (void)snprintf(
            str,
            string_length,
            "%s%s",
            prefix,
            branch_type == LEAF ? "     " : " │   "
        );
    }
    check(str);
    struct Huff_node const *const root = huff_node_at(tree, node);
    if (!root->link[1]) {
        huff_print_inner_tree(tree, root->link[0], LEAF, str, allocator);
    } else if (!root->link[0]) {
        huff_print_inner_tree(tree, root->link[1], LEAF, str, allocator);
    } else {
        huff_print_inner_tree(tree, root->link[1], BRANCH, str, allocator);
        huff_print_inner_tree(tree, root->link[0], LEAF, str, allocator);
    }
    (void)allocator->allocate((CCC_Allocator_arguments){
        .input = str,
        .bytes = 0,
        .alignment = alignof(char),
        .context = allocator->context,
    });
}

[[maybe_unused]] static void
huff_print_node(struct Huff_tree const *const tree, size_t const node) {
    if (huff_is_leaf(tree, node)) {
        switch (huff_char_index(tree, node)) {
            case '\n':
                printf("(\\n)\n");
                break;
            case '\r':
                printf("(\\r)\n");
                break;
            case '\t':
                printf("(\\t)\n");
                break;
            case '\v':
                printf("(\\v)\n");
                break;
            case '\f':
                printf("(\\f)\n");
                break;
            case '\b':
                printf("(\\b)\n");
                break;
            default:
                printf("(%c)\n", huff_char_index(tree, node));
        }
    } else {
        printf("1┐\n");
    }
}

[[maybe_unused]] static bool
huff_is_leaf(struct Huff_tree const *const tree, size_t const node) {
    struct Huff_node const *const root = huff_node_at(tree, node);
    return !root->link[0] && !root->link[1];
}

[[maybe_unused]] static void
huff_print_bitq(struct Bit_queue const *const bq) {
    for (size_t i = 0, col = 1, end = huff_bitq_count(bq); i < end;
         ++i, ++col) {
        huff_bitq_test(bq, i) ? printf("1") : printf("0");
        if (col % 50 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

/* NOLINTEND(*misc-no-recursion) */

/*========================       LZW Helper Code     ========================*/

static uint64_t
lzw_hash(CCC_Key_arguments const key) {
    struct Lzw_prefix const *const prefix = key.key;
    uint64_t code = (uint64_t)((prefix->prefix << 8) | prefix->next_char);
    code = (code ^ (code >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    code = (code ^ (code >> 27)) * UINT64_C(0x94d049bb133111eb);
    code = code ^ (code >> 31);
    return code;
}

static CCC_Order
lzw_compare(CCC_Key_comparator_arguments const key) {
    struct Lzw_prefix const *const known_prefix = key.key_left;
    struct Lzw_prefix const *const sequence = key.type_right;
    uint32_t const left
        = (uint32_t)((known_prefix->prefix << 8) | known_prefix->next_char);
    uint32_t const right
        = (uint32_t)((sequence->prefix << 8) | sequence->next_char);
    return (left > right) - (left < right);
}

static inline CCC_Entry
lzw_try_insert(
    struct Lzw_map *const map,
    struct Lzw_key_value const *const prefix,
    CCC_Allocator const *allocator
) {
    return CCC_flat_hash_map_try_insert(&map->map, prefix, allocator);
}

/*=====================       Bit Queue Helper Code     =====================*/

static CCC_Result
huff_bitq_push_back(
    struct Bit_queue *const bq,
    CCC_Tribool const bit,
    CCC_Allocator const *const allocator
) {
    CCC_Result result = huff_bitq_maybe_resize(bq, 1, allocator);
    if (result != CCC_RESULT_OK) {
        return result;
    }
    if (bq->count == flat_bitset_count(&bq->bs).count) {
        result = push_back(&bq->bs, bit, &(CCC_Allocator){});
        if (result != CCC_RESULT_OK) {
            return result;
        }
    } else {
        CCC_Tribool const was = flat_bitset_set(
            &bq->bs, (bq->front + bq->count) % capacity(&bq->bs).count, bit
        );
        if (was == CCC_TRIBOOL_ERROR) {
            return CCC_RESULT_FAIL;
        }
    }
    ++bq->count;
    return CCC_RESULT_OK;
}

static CCC_Tribool
huff_bitq_pop_back(struct Bit_queue *const bq) {
    if (!bq->count) {
        return CCC_TRIBOOL_ERROR;
    }
    size_t const i = (bq->front + bq->count - 1) % capacity(&bq->bs).count;
    CCC_Tribool const bit = flat_bitset_test(&bq->bs, i);
    check(bit != CCC_TRIBOOL_ERROR);
    --bq->count;
    return bit;
}

static CCC_Tribool
huff_bitq_pop_front(struct Bit_queue *const bq) {
    if (!bq->count) {
        return CCC_TRIBOOL_ERROR;
    }
    CCC_Tribool const bit = flat_bitset_test(&bq->bs, bq->front);
    check(bit != CCC_TRIBOOL_ERROR);
    bq->front = (bq->front + 1) % count(&bq->bs).count;
    --bq->count;
    return bit;
}

static CCC_Tribool
huff_bitq_test(struct Bit_queue const *const bq, size_t const i) {
    if (!bq->count) {
        return CCC_TRIBOOL_ERROR;
    }
    return flat_bitset_test(&bq->bs, (bq->front + i) % capacity(&bq->bs).count);
}

/** We don't support push front so we technically don't need this logic yet
but I felt the code was just waiting for a bug without it so I'm adding it
here. As soon as you add push front, pop front, push back, pop back you need
to compact elements from [0, count) whenever you resize, otherwise modulo
by capacity will break. Elements would wrap before the end of the bit set
capacity otherwise, and the double ended queue invariants don't hold. */
static CCC_Result
huff_bitq_maybe_resize(
    struct Bit_queue *const bq,
    size_t bits_to_add,
    CCC_Allocator const *const allocator
) {
    size_t const old_capacity = flat_bitset_capacity(&bq->bs).count;
    size_t const requirement = bq->count + bits_to_add;
    if (requirement < old_capacity) {
        return CCC_RESULT_OK;
    }
    static_assert(
        (CCC_FLAT_BITSET_BLOCK_BITS & (CCC_FLAT_BITSET_BLOCK_BITS - 1)) == 0,
        "rounding up to next block bits capacity with powers of 2 only"
    );
    assert(requirement);
    size_t const new_capacity
        = ((requirement * 2) + (CCC_FLAT_BITSET_BLOCK_BITS - 1))
        & ~(CCC_FLAT_BITSET_BLOCK_BITS - 1);
    Flat_bitset compact_bits
        = flat_bitset_with_capacity(*allocator, new_capacity, bq->count);
    if (!flat_bitset_capacity(&compact_bits).count) {
        return CCC_RESULT_ALLOCATOR_ERROR;
    }
    if (bq->count) {
        size_t const first_chunk = min(bq->count, old_capacity - bq->front);
        size_t compacting_bit = 0;
        size_t bq_bit = bq->front;
        while (compacting_bit < first_chunk) {
            CCC_Tribool const set = flat_bitset_set(
                &compact_bits, compacting_bit, flat_bitset_test(&bq->bs, bq_bit)
            );
            if (set == CCC_TRIBOOL_ERROR) {
                return CCC_RESULT_FAIL;
            }
            ++compacting_bit;
            ++bq_bit;
        }
        bq_bit = 0;
        while (compacting_bit < bq->count) {
            CCC_Tribool const set = flat_bitset_set(
                &compact_bits, compacting_bit, flat_bitset_test(&bq->bs, bq_bit)
            );
            if (set == CCC_TRIBOOL_ERROR) {
                return CCC_RESULT_FAIL;
            }
            ++compacting_bit;
            ++bq_bit;
        }
    }
    CCC_Result const result = flat_bitset_clear_and_free(&bq->bs, allocator);
    if (result != CCC_RESULT_OK) {
        return result;
    }
    bq->bs = compact_bits;
    bq->front = 0;
    return CCC_RESULT_OK;
}

static CCC_Tribool
huff_bitq_is_empty(struct Bit_queue const *const bq) {
    return !bq->count;
}

static size_t
huff_bitq_count(struct Bit_queue const *const bq) {
    return bq->count;
}

static void
huff_bitq_clear_and_free(
    struct Bit_queue *const bq, CCC_Allocator const *const allocator
) {
    check(bq);
    CCC_Result const r = clear_and_free(&bq->bs, allocator);
    check(r == CCC_RESULT_OK);
}

static inline size_t
min(size_t const a, size_t const b) {
    return a < b ? a : b;
}

/*=====================       File Helper Code     ==+++++===================*/

static inline size_t
file_size(FILE *const f) {
    check(fseek(f, 0L, SEEK_END) >= 0);
    size_t const ret = (size_t)ftell(f);
    check(fseek(f, 0L, SEEK_SET) >= 0);
    return ret;
}

static inline FILE *
open_file_copy_in_output(SV_Str_view compressed_file) {
    /* This means the compressed file was valid so write a new one to output. */
    static char path[FILESYS_MAX_PATH];
    CCC_Tribool has_suf = SV_ends_with(compressed_file, cccz_suffix);
    check(has_suf);
    compressed_file = SV_remove_suffix(compressed_file, SV_len(cccz_suffix));
    size_t const dir_delim = SV_reverse_find(
        compressed_file, SV_len(compressed_file), SV_from("/")
    );
    SV_Str_view const raw_file
        = dir_delim == SV_npos(compressed_file)
            ? compressed_file
            : SV_substr(
                  compressed_file, dir_delim + 1, SV_len(compressed_file)
              );
    size_t const total_bytes = SV_bytes(output_dir) + SV_bytes(raw_file);
    check(total_bytes < FILESYS_MAX_PATH);
    size_t const prefix = SV_fill(SV_bytes(output_dir), path, output_dir);
    (void)SV_fill(SV_bytes(raw_file), path + (prefix - 1), raw_file);
    /* Checks are good and path is set this will be a fresh copy. */
    FILE *const copy_of_original = fopen(path, "w");
    if (!copy_of_original) {
        (void)fprintf(stderr, "%s\n", strerror(errno));
        return NULL;
    }
    return copy_of_original;
}

static inline struct File_char
peek_first_char(FILE *const file) {
    int const first_byte = fgetc(file);
    check(first_byte != EOF);
    check(ungetc(first_byte, file) != EOF);
    return first_byte == EOF
             ? (struct File_char){.error = true}
             : (struct File_char){.c = (unsigned char)first_byte};
}

static struct New_file
open_compressed_file(SV_Str_view const original_filepath) {
    /* We write all new files to output directory so create the new path. */
    static char path_to_cccz[FILESYS_MAX_PATH];
    size_t const dir_delim = SV_reverse_find(
        original_filepath, SV_len(original_filepath), SV_from("/")
    );
    SV_Str_view const raw_file
        = dir_delim == SV_npos(original_filepath)
            ? original_filepath
            : SV_substr(
                  original_filepath, dir_delim + 1, SV_len(original_filepath)
              );
    size_t const total_bytes
        = SV_bytes(output_dir) + SV_bytes(raw_file) + SV_bytes(cccz_suffix);
    check(total_bytes < FILESYS_MAX_PATH);
    size_t const path_bytes
        = SV_fill(SV_bytes(output_dir), path_to_cccz, output_dir);
    size_t const file_bytes = SV_fill(
        SV_bytes(raw_file), path_to_cccz + (path_bytes - 1), raw_file
    );
    (void)SV_fill(
        SV_bytes(cccz_suffix),
        path_to_cccz + (path_bytes - 1) + (file_bytes - 1),
        cccz_suffix
    );

    /* Path is now correct and verified try to create file. */
    FILE *const cccz = fopen(path_to_cccz, "w");
    check(cccz, (void)fprintf(stderr, "%s", strerror(errno)););
    return (struct New_file){
        .path = SV_from_terminated(path_to_cccz),
        .file = cccz,
    };
}

/** Write the specified bytes to the file or exit the program if an error
occurs. Returns the number of bytes written. */
static size_t
huff_write_bytes(
    FILE *const f, void const *const source, size_t const to_write
) {
    size_t count = 0;
    unsigned char const *source_pointer = source;
    while (count < to_write) {
        /* Avoiding a strict aliasing violation. */
        uint8_t byte = 0;
        memcpy(&byte, source_pointer, sizeof(unsigned char));
        int const writ = fputc(byte, f);
        check(!ferror(f));
        if (writ == EOF) {
            return count;
        }
        ++source_pointer;
        ++count;
    }
    return count;
}

/** Reads the specified number of bytes from the file or exits if an error
occurs. The bytes read are returned and may be less than requested if the file
ends. */
static size_t
huff_readbytes(FILE *const f, void *const destination, size_t const to_read) {
    size_t count = 0;
    unsigned char *destination_pointer = destination;
    while (count < to_read) {
        int const read = fgetc(f);
        check(!ferror(f));
        if (read == EOF) {
            return count;
        }
        /* Avoid strict aliasing violation. */
        memcpy(destination_pointer, &read, sizeof(unsigned char));
        ++destination_pointer;
        ++count;
    }
    return count;
}

/*=====================       Container Helper Code     =====================*/

/** The flat hash map documentation mentions we should have good bit diversity
in the high and low byte of our hash but we are only hashing characters which
are their own unique value across all characters we will encounter. So the
hashed value will just be the character repeated at the high and low byte. */
static uint64_t
hash_char(CCC_Key_arguments const to_hash) {
    char const key = *(char const *)to_hash.key;
    return ((uint64_t)key << 55) | (uint64_t)key;
}

static CCC_Order
char_order(CCC_Key_comparator_arguments const order) {
    struct Huff_character_frequency const *const right
        = (struct Huff_character_frequency *)order.type_right;
    char const left = *(char *)order.key_left;
    return (left > right->ch) - (left < right->ch);
}

static CCC_Order
order_freqs(CCC_Comparator_arguments const order) {
    struct Huff_pair_node const *const left
        = (struct Huff_pair_node *)order.type_left;
    struct Huff_pair_node const *const right
        = (struct Huff_pair_node *)order.type_right;
    return (left->frequency > right->frequency)
         - (left->frequency < right->frequency);
}

static CCC_Order
path_memo_order(CCC_Key_comparator_arguments const order) {
    struct Huff_path_memo const *const right
        = (struct Huff_path_memo *)order.type_right;
    char const left = *(char *)order.key_left;
    return (left > right->ch) - (left < right->ch);
}

/*=========================     Help Message      ===========================*/

static void
print_help(void) {
    static char const *const message
        = "Compress and Decompress Files:\n\n\t-c=/file/name - [c]ompress the "
          "file to create a samples/output/name.ccc file\n\t"
          "-d=/samples/output/name.ccc - [d]ecompress the file to "
          "create a samples/output/name file\n\nNote: Compression comes before "
          "decompression.\nThe following command compresses a file and then "
          "decompresses it.\nThe final copy of the original file is in the "
          "output directory.\nSample Command:\n./build/bin/ccczip "
          "-c=README.md -d=samples/output/README.md.ccc\n";
    printf("%s", message);
}
