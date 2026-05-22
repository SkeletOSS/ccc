# This version gives us file sets which are great for library install.
cmake_minimum_required(VERSION 3.23)
project("ccc" 
  VERSION 0.79.0
  LANGUAGES C
  DESCRIPTION "The C Container Collection (ccc). A collection of containers written in C."
)
# Specify that C23 should be used
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(BUILD_SHARED_LIBS "Building ccc as a shared library is available, but OFF by default." OFF)

# Required to help clarify and cleanup install in ccc/ and cmake/ cmake files.
set(namespace "ccc")

# For releases we only produce this and the source. No test or sample bloat.
include_directories("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}")
add_subdirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}")

# Excluding from all simplifies cmake presets until cmake figures out how
# to tame the combinatorial explosion of buildPresets
if (EXISTS "${PROJECT_SOURCE_DIR}/samples")
    include_directories("${PROJECT_SOURCE_DIR}/samples")
    add_subdirectory("${PROJECT_SOURCE_DIR}/samples" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/tests")
    include_directories("${PROJECT_SOURCE_DIR}/tests")
    add_subdirectory("${PROJECT_SOURCE_DIR}/tests" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/utility")
    include_directories("${PROJECT_SOURCE_DIR}/utility")
    add_subdirectory("${PROJECT_SOURCE_DIR}/utility" EXCLUDE_FROM_ALL)
endif()
if (EXISTS "${PROJECT_SOURCE_DIR}/tests" AND EXISTS "${PROJECT_SOURCE_DIR}/samples" AND EXISTS "${PROJECT_SOURCE_DIR}/utility")
    include(tools/scanners.cmake)
endif()

# If we are in our own repository build environment, not a release, then we
# will bring in the str_view dependency for our samples and testing.
if (EXISTS "${PROJECT_SOURCE_DIR}/tests" OR EXISTS "${PROJECT_SOURCE_DIR}/samples" OR EXISTS "${PROJECT_SOURCE_DIR}/utility")
    include(FetchContent)
    FetchContent_Declare(
      str_view
      URL https://github.com/skeletoss/str_view/releases/download/v0.7.6/str_view-v0.7.6.zip
      URL_HASH SHA256=4ab4e21f1deb16b15f83542489ffc3b136a8e6cbe37d61686abc4dc1d103f3f0
      DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    )
    FetchContent_MakeAvailable(str_view)
    target_compile_options(str_view PRIVATE "-w")
endif()

if (NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "This project has a top-level one called [${CMAKE_PROJECT_NAME}]")
else()
    message(STATUS "This project is a top-level one")
endif()


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug/bin)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

set(CCC_COVERAGE_TOOL "" CACHE STRING "Code coverage tool to use for coverage report")
# Code coverage is easy on gcc but clang requires some extra steps. We can 
# indicate via our presets that we are using a code coverage preset and we can
# also manually specify the code coverage tool if we have one installed. This
# is helpful for gcc, for example, because I have a newer gcov I like to use
# for the gcc-XX.X compiler; this is not the default one installed.
if (CCC_ENABLE_COVERAGE)

    if (CCC_COVERAGE_TOOL)
        set(COVERAGE_TOOL ${CCC_COVERAGE_TOOL})
        message(STATUS "Using code coverage tool: ${CCC_COVERAGE_TOOL}")
    endif()

    # Clang had very tricky behavior especially on mac to successfully generate
    # lcov compatible reports. The command line would not accept clang's 
    # provided 'llvm-cov gcov' string so a custom script that executes that 
    # command needs to be generated. We will just place it in the build files.
    if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
        if (COVERAGE_TOOL)
            set(LLVM_COV_EXECUTABLE ${COVERAGE_TOOL})
        else()
            find_program(LLVM_COV_EXECUTABLE llvm-cov)
            if (NOT LLVM_COV_EXECUTABLE)
                message(FATAL_ERROR "llvm-cov not found")
            endif()
        endif()

        set(LLVM_GCOV_WRAPPER "${CMAKE_BINARY_DIR}/llvm-gcov")

        file(WRITE ${LLVM_GCOV_WRAPPER}
"#!/usr/bin/env bash
exec \"${LLVM_COV_EXECUTABLE}\" gcov \"$@\"
")

        file(CHMOD ${LLVM_GCOV_WRAPPER}
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE)

        set(COVERAGE_TOOL ${LLVM_GCOV_WRAPPER})
    endif()

    if (NOT COVERAGE_TOOL)
        message(FATAL_ERROR "no code coverage tool selected")
    endif()

    # Developer coverage reports include all test files to confirm which test
    # cod is ran. This should be for the local developer machine.
    add_custom_target(coverage-developer
        COMMAND lcov --ignore-errors inconsistent 
                     --gcov-tool ${COVERAGE_TOOL}
                     --capture
                     --directory ${CMAKE_BINARY_DIR}
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND lcov --ignore-errors inconsistent
                     --extract ${CMAKE_SOURCE_DIR}/docs/coverage.info 
                     '${CMAKE_SOURCE_DIR}/tests/*' 
                     '${CMAKE_SOURCE_DIR}/source/*'
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND lcov --remove ${CMAKE_SOURCE_DIR}/docs/coverage.info 
                     '${CMAKE_SOURCE_DIR}/tests/run_tests.c' 
                     '${CMAKE_SOURCE_DIR}/tests/checkers.c'
                     -o ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND genhtml ${CMAKE_SOURCE_DIR}/docs/coverage.info
                        --output-directory ${CMAKE_SOURCE_DIR}/docs/coverage
                        --prefix ${CMAKE_SOURCE_DIR}
                        --title "CCC Test Suite Coverage Report"
    )
    
    # The published report on github pages should not include tests as I don't
    # want the C Container Collection to be seen as including tests to inflate
    # coverage numbers. We want to build trust and show we are working hard on
    # good library not just padding imaginary numbers.
    add_custom_target(coverage-publish
        COMMAND lcov --ignore-errors inconsistent 
                     --gcov-tool ${COVERAGE_TOOL}
                     --capture
                     --directory ${CMAKE_BINARY_DIR}
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND lcov --ignore-errors inconsistent
                     --extract ${CMAKE_SOURCE_DIR}/docs/coverage.info 
                     '${CMAKE_SOURCE_DIR}/source/*'
                     --output-file ${CMAKE_SOURCE_DIR}/docs/coverage.info

        COMMAND genhtml ${CMAKE_SOURCE_DIR}/docs/coverage.info
                        --output-directory ${CMAKE_SOURCE_DIR}/docs/coverage
                        --prefix ${CMAKE_SOURCE_DIR}
                        --flat
                        --title "CCC Test Suite Coverage Report"
    )
endif()


# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
