# Master Testing Target & Global Infrastructure
# Custom meta-target to build all tests selectively via 'cmake --build . --target tests'
add_custom_target(tests)

# Shared test assertion verification utility
add_library(checkers STATIC checkers.c checkers.h)
target_link_libraries(checkers PRIVATE ccc::ccc)
add_dependencies(tests checkers)

# Main test runner driver binary
add_executable(run_tests run_tests.c)
target_link_libraries(run_tests PRIVATE checkers str_view::str_view)
add_dependencies(tests run_tests)

# Test Generation 
# Usage: ccc_add_test(<category_folder> <test_name> [extra_libs...])
function(ccc_add_test CATEGORY TEST_NAME)
    # Define the executable using the nested folder structure
    add_executable(${TEST_NAME} "${CATEGORY}/${TEST_NAME}.c")
    
    # Apply standard required testing link matrix + any extra optional args (${ARGN})
    target_link_libraries(${TEST_NAME}
        PRIVATE
            ${ARGN}  # Captures extra variations like 'random' or custom utilities
            std_allocator
            stack_allocator
            checkers
            ccc::ccc
    )

    # Isolate test binary binaries into the designated test directory
    set_target_properties(${TEST_NAME} 
        PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests"
    )
    
    # Tie the executable directly to the master build target
    add_dependencies(tests ${TEST_NAME})
endfunction()

# Container Tests

# Bit Set 
ccc_add_test(flat_bitset test_flat_bitset_construct str_view::str_view)
ccc_add_test(flat_bitset test_flat_bitset_insert str_view::str_view)
ccc_add_test(flat_bitset test_flat_bitset_erase str_view::str_view)
ccc_add_test(flat_bitset test_flat_bitset_test_and_set str_view::str_view)

# Flat Buffer 
add_library(flat_buffer_utility STATIC flat_buffer/flat_buffer_utility.h flat_buffer/flat_buffer_utility.c)
target_link_libraries(flat_buffer_utility PRIVATE ccc checkers random)
add_dependencies(tests flat_buffer_utility)

ccc_add_test(flat_buffer test_flat_buffer_construct flat_buffer_utility random)
ccc_add_test(flat_buffer test_flat_buffer_insert flat_buffer_utility random)
ccc_add_test(flat_buffer test_flat_buffer_erase flat_buffer_utility random)
ccc_add_test(flat_buffer test_flat_buffer_iterator flat_buffer_utility random)
ccc_add_test(flat_buffer test_flat_buffer_error flat_buffer_utility random)

# Flat Priority Queue 
add_library(flat_priority_queue_utility STATIC flat_priority_queue/flat_priority_queue_utility.h flat_priority_queue/flat_priority_queue_utility.c)
target_link_libraries(flat_priority_queue_utility PRIVATE ccc std_allocator checkers)
add_dependencies(tests flat_priority_queue_utility)

ccc_add_test(flat_priority_queue test_flat_priority_queue_construct flat_priority_queue_utility)
ccc_add_test(flat_priority_queue test_flat_priority_queue_insert flat_priority_queue_utility)
ccc_add_test(flat_priority_queue test_flat_priority_queue_erase flat_priority_queue_utility)
ccc_add_test(flat_priority_queue test_flat_priority_queue_update flat_priority_queue_utility)

# Tree Map 
add_library(tree_map_utility STATIC tree_map/tree_map_utility.h tree_map/tree_map_utility.c)
target_link_libraries(tree_map_utility PRIVATE ccc checkers)
add_dependencies(tests tree_map_utility)

ccc_add_test(tree_map test_tree_map_construct tree_map_utility)
ccc_add_test(tree_map test_tree_map_insert tree_map_utility)
ccc_add_test(tree_map test_tree_map_erase tree_map_utility)
ccc_add_test(tree_map test_tree_map_iterator tree_map_utility)
ccc_add_test(tree_map test_tree_map_entry tree_map_utility)
ccc_add_test(tree_map test_tree_map_lru tree_map_utility)

# Array Tree Map
add_library(array_tree_map_utility STATIC array_tree_map/array_tree_map_utility.h array_tree_map/array_tree_map_utility.c)
target_link_libraries(array_tree_map_utility PRIVATE ccc checkers)
add_dependencies(tests array_tree_map_utility)

ccc_add_test(array_tree_map test_array_tree_map_construct array_tree_map_utility random)
ccc_add_test(array_tree_map test_array_tree_map_insert array_tree_map_utility random)
ccc_add_test(array_tree_map test_array_tree_map_erase array_tree_map_utility random)
ccc_add_test(array_tree_map test_array_tree_map_iterator array_tree_map_utility random)
ccc_add_test(array_tree_map test_array_tree_map_handle array_tree_map_utility random)
ccc_add_test(array_tree_map test_array_tree_map_lru array_tree_map_utility random)

# Flat Hash Map 
add_library(flat_hash_map_utility STATIC flat_hash_map/flat_hash_map_utility.h flat_hash_map/flat_hash_map_utility.c)
target_link_libraries(flat_hash_map_utility PRIVATE ccc)
add_dependencies(tests flat_hash_map_utility)

ccc_add_test(flat_hash_map test_flat_hash_map_construct flat_hash_map_utility random)
ccc_add_test(flat_hash_map test_flat_hash_map_insert flat_hash_map_utility random)
ccc_add_test(flat_hash_map test_flat_hash_map_erase flat_hash_map_utility random)
ccc_add_test(flat_hash_map test_flat_hash_map_lru flat_hash_map_utility random)
ccc_add_test(flat_hash_map test_flat_hash_map_entry flat_hash_map_utility random)
ccc_add_test(flat_hash_map test_flat_hash_map_iterator flat_hash_map_utility random)
ccc_add_test(flat_hash_map test_flat_hash_map_error flat_hash_map_utility random)

# Doubly Linked List 
add_library(doubly_linked_list_utility STATIC doubly_linked_list/doubly_linked_list_utility.h doubly_linked_list/doubly_linked_list_utility.c)
target_link_libraries(doubly_linked_list_utility PRIVATE ccc)
add_dependencies(tests doubly_linked_list_utility)

ccc_add_test(doubly_linked_list test_doubly_linked_list_construct doubly_linked_list_utility)
ccc_add_test(doubly_linked_list test_doubly_linked_list_insert doubly_linked_list_utility)
ccc_add_test(doubly_linked_list test_doubly_linked_list_iterate doubly_linked_list_utility)
ccc_add_test(doubly_linked_list test_doubly_linked_list_erase doubly_linked_list_utility)

# Singly Linked List 
add_library(singly_linked_list_utility STATIC singly_linked_list/singly_linked_list_utility.h singly_linked_list/singly_linked_list_utility.c)
target_link_libraries(singly_linked_list_utility PRIVATE ccc)
add_dependencies(tests singly_linked_list_utility)

ccc_add_test(singly_linked_list test_singly_linked_list_construct singly_linked_list_utility)
ccc_add_test(singly_linked_list test_singly_linked_list_insert singly_linked_list_utility)
ccc_add_test(singly_linked_list test_singly_linked_list_iterate singly_linked_list_utility)
ccc_add_test(singly_linked_list test_singly_linked_list_erase singly_linked_list_utility)

# Flat Double Ended Queue 
add_library(flat_double_ended_queue_utility STATIC flat_double_ended_queue/flat_double_ended_queue_utility.h flat_double_ended_queue/flat_double_ended_queue_utility.c)
target_link_libraries(flat_double_ended_queue_utility PRIVATE ccc)
add_dependencies(tests flat_double_ended_queue_utility)

ccc_add_test(flat_double_ended_queue test_flat_double_ended_queue_construct flat_double_ended_queue_utility)
ccc_add_test(flat_double_ended_queue test_flat_double_ended_queue_insert flat_double_ended_queue_utility)
ccc_add_test(flat_double_ended_queue test_flat_double_ended_queue_iterate flat_double_ended_queue_utility)
ccc_add_test(flat_double_ended_queue test_flat_double_ended_queue_erase flat_double_ended_queue_utility)

# Specialized Container Tests 
if (CCC_BUILD_SPECIALIZED)
    # Adaptive Map 
    add_library(adaptive_map_utility STATIC specialized/adaptive_map/adaptive_map_utility.h specialized/adaptive_map/adaptive_map_utility.c)
    target_link_libraries(adaptive_map_utility PRIVATE ccc checkers)
    add_dependencies(tests adaptive_map_utility)

    ccc_add_test(specialized/adaptive_map test_adaptive_map_construct adaptive_map_utility)
    ccc_add_test(specialized/adaptive_map test_adaptive_map_insert adaptive_map_utility)
    ccc_add_test(specialized/adaptive_map test_adaptive_map_erase adaptive_map_utility)
    ccc_add_test(specialized/adaptive_map test_adaptive_map_iterator adaptive_map_utility)
    ccc_add_test(specialized/adaptive_map test_adaptive_map_entry adaptive_map_utility)
    ccc_add_test(specialized/adaptive_map test_adaptive_map_lru adaptive_map_utility)

    # Array Adaptive Map 
    add_library(array_adaptive_map_utility STATIC specialized/array_adaptive_map/array_adaptive_map_utility.h specialized/array_adaptive_map/array_adaptive_map_utility.c)
    target_link_libraries(array_adaptive_map_utility PRIVATE ccc checkers)
    add_dependencies(tests array_adaptive_map_utility)

    ccc_add_test(specialized/array_adaptive_map test_array_adaptive_map_construct array_adaptive_map_utility)
    ccc_add_test(specialized/array_adaptive_map test_array_adaptive_map_insert array_adaptive_map_utility)
    ccc_add_test(specialized/array_adaptive_map test_array_adaptive_map_erase array_adaptive_map_utility)
    ccc_add_test(specialized/array_adaptive_map test_array_adaptive_map_iterator array_adaptive_map_utility)
    ccc_add_test(specialized/array_adaptive_map test_array_adaptive_map_handle array_adaptive_map_utility)
    ccc_add_test(specialized/array_adaptive_map test_array_adaptive_map_lru array_adaptive_map_utility)

    # Pair Priority Queue 
    add_library(priority_queue_utility STATIC specialized/priority_queue/priority_queue_utility.h specialized/priority_queue/priority_queue_utility.c)
    target_link_libraries(priority_queue_utility PRIVATE ccc checkers stack_allocator)
    add_dependencies(tests priority_queue_utility)

    ccc_add_test(specialized/priority_queue test_priority_queue_construct priority_queue_utility)
    ccc_add_test(specialized/priority_queue test_priority_queue_insert priority_queue_utility)
    ccc_add_test(specialized/priority_queue test_priority_queue_erase priority_queue_utility)
    ccc_add_test(specialized/priority_queue test_priority_queue_update priority_queue_utility)
    ccc_add_test(specialized/priority_queue test_priority_queue_error priority_queue_utility)
endif()
