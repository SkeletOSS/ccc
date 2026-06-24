add_library(${PROJECT_NAME})
add_library(${namespace}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# File sets simplify install and allow isolation of only container files.
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/types.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/flat_bitset.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/flat_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/flat_hash_map.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/flat_double_ended_queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/flat_priority_queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/singly_linked_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/doubly_linked_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/tree_map.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../source/array_tree_map.c
    PUBLIC 
        FILE_SET public_headers
            TYPE HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}
            FILES 
              private/private_flat_priority_queue.h
              private/private_singly_linked_list.h
              private/private_doubly_linked_list.h
              private/private_tree_map.h
              private/private_array_tree_map.h
              private/private_traits.h
              private/private_flat_bitset.h
              private/private_flat_buffer.h
              private/private_flat_double_ended_queue.h
              private/private_flat_hash_map.h
              configuration.h
              types.h
              flat_buffer.h
              flat_bitset.h
              flat_hash_map.h
              flat_double_ended_queue.h
              flat_priority_queue.h
              tree_map.h
              array_tree_map.h
              singly_linked_list.h
              doubly_linked_list.h
              traits.h
              sort.h
)

if (CCC_BUILD_SPECIALIZED)
    message(STATUS "CCC Building with specialized modules enabled.")
    target_sources(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../source/specialized/adaptive_map.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../source/specialized/array_adaptive_map.c
            ${CMAKE_CURRENT_SOURCE_DIR}/../source/specialized/priority_queue.c
        PUBLIC 
            FILE_SET public_headers
                FILES 
                  specialized/private/private_adaptive_map.h
                  specialized/private/private_array_adaptive_map.h
                  specialized/private/private_priority_queue.h
                  specialized/adaptive_map.h
                  specialized/array_adaptive_map.h
                  specialized/priority_queue.h
    )
endif()

option(CCC_FLAT_HASH_MAP_PORTABLE "Fallback to portable implementation of flat hash map to improve cross platform compatibility while decreasing optimizations" OFF)
set(CCC_USER_CONFIGURATION ""  CACHE STRING "Freestanding environment with user defined configuration providing necessary functions")
if (CCC_USER_CONFIGURATION)
    target_compile_definitions(${PROJECT_NAME} 
        PUBLIC 
            CCC_USER_CONFIGURATION="${CCC_USER_CONFIGURATION}"
    )
endif()
if (CCC_FLAT_HASH_MAP_PORTABLE)
    target_compile_definitions(${PROJECT_NAME} 
        PUBLIC 
            CCC_FLAT_HASH_MAP_PORTABLE
    )
endif()
if (CCC_BUILD_SPECIALIZED)
    target_compile_definitions(${PROJECT_NAME} PUBLIC CCC_SPECIALIZED_ENABLED)
endif()
target_compile_features(${PROJECT_NAME} PUBLIC c_std_23)

# set properties for the target. VERSION set the library version to the project
# version * SOVERSION set the compatibility  version for the library to the
# major number of the version
# note that ${public_headers} should be in quotes
set_target_properties(${PROJECT_NAME} 
    PROPERTIES 
        RELEASE_POSTFIX "_release"
        DEBUG_POSTFIX "_debug"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        C_VISIBILITY_PRESET "default"
        VISIBILITY_INLINES_HIDDEN TRUE
)

include(GenerateExportHeader)

# this command generates a header file in the CMAKE_CURRENT_BINARY_DIR which
# sets the visibility attributes according to the compiler settings
generate_export_header(${PROJECT_NAME}
    EXPORT_FILE_NAME export/export_${PROJECT_NAME}.h
)

# Add CMAKE_CURRENT_BINARY_DIR to the include path so the generated header can
# be found
target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/export>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
