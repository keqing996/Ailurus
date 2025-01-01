cmake_minimum_required(VERSION 3.22)

project (AILURUS_INFRASTRUCTURE)
set (CMAKE_CXX_STANDARD 20)

# Set output path
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/artifacts/${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/)

# Source files
file (GLOB_RECURSE AILURUS_SRC ./src/*.cpp)

# Library
add_library                 (ailurus STATIC ${AILURUS_SRC})
target_include_directories  (ailurus PUBLIC ./include/)

if (WIN32)
    target_link_libraries   (ailurus wsock32 ws2_32)
endif ()

# Test code
option (AILURUS_ENABLE_TEST OFF)

if (AILURUS_ENABLE_TEST)
    add_subdirectory(test)
endif ()