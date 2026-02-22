# This version gives us file sets which are great for library install.
cmake_minimum_required(VERSION 3.23)
project("ccc" 
  VERSION 0.63.0
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
      URL https://github.com/skeletoss/str_view/releases/download/v0.7.1/str_view-v0.7.1.zip
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


# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
