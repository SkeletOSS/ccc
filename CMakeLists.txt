# This version ensures we have access to file sets.
cmake_minimum_required(VERSION 3.23)
project("ccc" 
    VERSION 0.80.4
    LANGUAGES C
    DESCRIPTION "The C Container Collection offers allocator agnostic data structures for C programmers."
    HOMEPAGE_URL "https://github.com/SkeletOSS/ccc"
)
# We will set standard on a per target basis for CCC in the ccc/CMakeLists.txt file.
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(BUILD_SHARED_LIBS "Building ccc as a shared library is available, but OFF by default." OFF)
# Required to help clarify and cleanup install in ccc/ and cmake/ cmake files.
set(namespace "ccc")
# Autodetect the specialized headers directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/specialized" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/specialized")
    set(CCC_HAS_SPECIALIZED_SOURCE TRUE)
else()
    set(CCC_HAS_SPECIALIZED_SOURCE FALSE)
endif()

option(CCC_BUILD_SPECIALIZED "Enable specialized containers" ${CCC_HAS_SPECIALIZED_SOURCE})
# If a user manually forces specialized verify the files exist
if(CCC_BUILD_SPECIALIZED AND NOT CCC_HAS_SPECIALIZED_SOURCE)
    message(FATAL_ERROR "CCC_BUILD_SPECIALIZED is set to ON, but the 'specialized/' directory is not found. Users should obtain a specialized release from the releases page. Developers may obtain a specialized containers from the releases page or project repository.")
endif()

###############################################################################
################### CCC Library Target For Consumers  #########################
# Even though we have directories for tests, samples, and utilities, these will
# all be stripped out of releases. So we treat this project as if it's sole
# purpose is to distribute only the core ccc::ccc target and create the library
# target at the root with relative pathing such that the project will compile
# on a user's system regardless of being fetched via FetchContent or 
# find_package. This way they also get to include our headers with the ccc 
# prefix, such as `#include "ccc/flat_buffer.h"`. Other directories will be 
# conditionally detected.
###############################################################################

add_library(${PROJECT_NAME})
add_library(${namespace}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/types.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/flat_bitset.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/flat_buffer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/flat_hash_map.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/flat_double_ended_queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/flat_priority_queue.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/singly_linked_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/doubly_linked_list.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/tree_map.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/array_tree_map.c
    PUBLIC 
        FILE_SET ${PROJECT_NAME}_public_headers
            TYPE HEADERS
            BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
            FILES 
              ccc/private/private_flat_priority_queue.h
              ccc/private/private_singly_linked_list.h
              ccc/private/private_doubly_linked_list.h
              ccc/private/private_tree_map.h
              ccc/private/private_array_tree_map.h
              ccc/private/private_traits.h
              ccc/private/private_flat_bitset.h
              ccc/private/private_flat_buffer.h
              ccc/private/private_flat_double_ended_queue.h
              ccc/private/private_flat_hash_map.h
              ccc/configuration.h
              ccc/types.h
              ccc/flat_buffer.h
              ccc/flat_bitset.h
              ccc/flat_hash_map.h
              ccc/flat_double_ended_queue.h
              ccc/flat_priority_queue.h
              ccc/tree_map.h
              ccc/array_tree_map.h
              ccc/singly_linked_list.h
              ccc/doubly_linked_list.h
              ccc/traits.h
              ccc/sort.h
)
if (CCC_BUILD_SPECIALIZED)
    target_sources(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/source/specialized/adaptive_map.c
            ${CMAKE_CURRENT_SOURCE_DIR}/source/specialized/array_adaptive_map.c
            ${CMAKE_CURRENT_SOURCE_DIR}/source/specialized/priority_queue.c
        PUBLIC 
            FILE_SET ${PROJECT_NAME}_public_headers
                FILES 
                  ccc/specialized/private/private_adaptive_map.h
                  ccc/specialized/private/private_array_adaptive_map.h
                  ccc/specialized/private/private_priority_queue.h
                  ccc/specialized/adaptive_map.h
                  ccc/specialized/array_adaptive_map.h
                  ccc/specialized/priority_queue.h
    )
endif()
option(CCC_FLAT_HASH_MAP_PORTABLE "Fallback to portable implementation of flat hash map to improve cross platform compatibility while decreasing optimizations" OFF)
set(CCC_USER_CONFIGURATION ""  CACHE STRING "Freestanding environment with user defined configuration providing necessary functions")
if (CCC_USER_CONFIGURATION)
    target_compile_definitions(${PROJECT_NAME} 
        PUBLIC 
        CCC_USER_CONFIGURATION_HEADER="${CCC_USER_CONFIGURATION}"
    )
endif()
if (CCC_FLAT_HASH_MAP_PORTABLE)
    target_compile_definitions(${PROJECT_NAME} 
        PUBLIC 
            CCC_FLAT_HASH_MAP_PORTABLE_ENABLED
    )
endif()
if (CCC_BUILD_SPECIALIZED)
    target_compile_definitions(${PROJECT_NAME} PUBLIC CCC_SPECIALIZED_ENABLED)
endif()
target_compile_features(${PROJECT_NAME} PUBLIC c_std_23)
# Now provide developers and users with as much clarity as possible in case they don't read install directions.
include(FeatureSummary)
add_feature_info(CCC_BUILD_SPECIALIZED 
    CCC_BUILD_SPECIALIZED "Build containers with unique space and runtime properties found in the specialized/ directories."
)
add_feature_info(CCC_FLAT_HASH_MAP_PORTABLE
    CCC_FLAT_HASH_MAP_PORTABLE "Compile the CCC_Flat_hash_map with Single Register Multiple Data fallback logic for compatibility with a wider range of platforms."
)
add_feature_info(CCC_USER_CONFIGURATION
    CCC_USER_CONFIGURATION "Build CCC as part of a freestanding target with user-provided implementations of C standard library functions. User provided header: '${CCC_USER_CONFIGURATION}'"
)
feature_summary(WHAT ALL
    DESCRIPTION "C Container Collection Feature Summary (v${PROJECT_VERSION}):"
)
# set properties for the target. VERSION set the library version to the project
# version * SOVERSION set the compatibility  version for the library to the
# major number of the version
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

###############################################################################
################## CCC Auxiliary Targets For Developers #######################
# These targets will only be built if their directories are found. These 
# directories are stripped out of the releases and the main.zip that GitHub 
# creates for the repository. Therefore, they can only be obtained by cloning
# the repository as a contributing developer.
###############################################################################

# If we are in our own repository build environment, not a release, then we
# will bring in the str_view dependency for our samples and testing.
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "This project is a top-level one")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/utility")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/utility" EXCLUDE_FROM_ALL)
    endif()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/samples" EXCLUDE_FROM_ALL)
    endif()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests" EXCLUDE_FROM_ALL)
    endif()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/utility")
        include(tools/scanners.cmake)
    endif()
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/samples" OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/utility")
        include(FetchContent)
        FetchContent_Declare(
          str_view
          URL https://github.com/skeletoss/str_view/releases/download/v0.7.6/str_view-v0.7.6.zip
          URL_HASH SHA256=4ab4e21f1deb16b15f83542489ffc3b136a8e6cbe37d61686abc4dc1d103f3f0
          DOWNLOAD_EXTRACT_TIMESTAMP FALSE
        )
        FetchContent_MakeAvailable(str_view)
        target_compile_options(str_view PRIVATE 
            $<$<C_COMPILER_ID:MSVC>:/W0>
            $<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>:-w>
        )
    endif()
else()
    message(STATUS "This project has a top-level one called [${CMAKE_PROJECT_NAME}]")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug/bin)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
include(cmake/Coverage.cmake)
# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
