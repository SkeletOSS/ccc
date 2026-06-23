# This version gives us file sets which are great for library install.
cmake_minimum_required(VERSION 3.23)
project("ccc" 
  VERSION 0.80.3
  LANGUAGES C
  DESCRIPTION "The C Container Collection (ccc). A collection of containers written in C."
)
# We will set standard on a per target basis for CCC in the ccc/CMakeLists.txt file.
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(BUILD_SHARED_LIBS "Building ccc as a shared library is available, but OFF by default." OFF)

# Required to help clarify and cleanup install in ccc/ and cmake/ cmake files.
set(namespace "ccc")

# Autodetect the specialized headers directory
if(EXISTS "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/specialized" AND EXISTS "${PROJECT_SOURCE_DIR}/source/specialized")
    set(CCC_HAS_SPECIALIZED_SOURCE TRUE)
else()
    set(CCC_HAS_SPECIALIZED_SOURCE FALSE)
endif()

option(CCC_BUILD_SPECIALIZED "Enable specialized containers" ${CCC_HAS_SPECIALIZED_SOURCE})
# If a user manually forces specialized verify the files exist
if(CCC_BUILD_SPECIALIZED AND NOT CCC_HAS_SPECIALIZED_SOURCE)
    message(FATAL_ERROR "CCC_BUILD_SPECIALIZED is set to ON, but the 'specialized/' directory is not found. Users should obtain a specialized release from the releases page. Developers may obtain a specialized containers from the releases page or project repository.")
endif()

# For releases we only produce this and the source. No test or sample bloat.
add_subdirectory("${PROJECT_SOURCE_DIR}/${PROJECT_NAME}")

# If we are in our own repository build environment, not a release, then we
# will bring in the str_view dependency for our samples and testing.
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "This project is a top-level one")
    if (EXISTS "${PROJECT_SOURCE_DIR}/utility")
        add_subdirectory("${PROJECT_SOURCE_DIR}/utility" EXCLUDE_FROM_ALL)
    endif()
    if (EXISTS "${PROJECT_SOURCE_DIR}/samples")
        add_subdirectory("${PROJECT_SOURCE_DIR}/samples" EXCLUDE_FROM_ALL)
    endif()
    if (EXISTS "${PROJECT_SOURCE_DIR}/tests")
        add_subdirectory("${PROJECT_SOURCE_DIR}/tests" EXCLUDE_FROM_ALL)
    endif()
    if (EXISTS "${PROJECT_SOURCE_DIR}/samples" AND EXISTS "${PROJECT_SOURCE_DIR}/tests" AND EXISTS "${PROJECT_SOURCE_DIR}/utility")
        include(tools/scanners.cmake)
    endif()
    if (EXISTS "${PROJECT_SOURCE_DIR}/tests" OR EXISTS "${PROJECT_SOURCE_DIR}/samples" OR EXISTS "${PROJECT_SOURCE_DIR}/utility")
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
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
