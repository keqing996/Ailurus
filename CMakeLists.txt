cmake_minimum_required(VERSION 3.22)

project (AILURUS_INFRASTRUCTURE)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_COLOR_DIAGNOSTICS ON)

add_subdirectory(submodule)
add_subdirectory(thirdparty)

# Source files
file (GLOB_RECURSE AILURUS_CORE_SRC ./src/Core/*.cpp)
file (GLOB_RECURSE AILURUS_APP_SRC ./src/App/*.cpp)

# Core Library
add_library                 (ailurus-core STATIC ${AILURUS_CORE_SRC})
target_include_directories  (ailurus-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Core/)
target_link_libraries       (ailurus-core PRIVATE stb_image spdlog)

if (WIN32)
    target_link_libraries   (ailurus-core PRIVATE wsock32 ws2_32) # for socket
    target_link_libraries   (ailurus-core PRIVATE dwmapi) # to fix some 'DwmXXX' link errors under mingw
endif ()

# App Library
add_library                 (ailurus-app STATIC ${AILURUS_APP_SRC})
target_include_directories  (ailurus-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/App/)
target_include_directories  (ailurus-app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/App/)
target_link_libraries       (ailurus-app PUBLIC imgui ailurus-core)
target_link_libraries       (ailurus-app PRIVATE SDL3-static vulkan_hpp)

# Test code
option (AILURUS_ENABLE_TEST OFF)

if (AILURUS_ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

# Example code
option (AILURUS_ENABLE_EXAMPLE OFF)

if (AILURUS_ENABLE_EXAMPLE)
    add_subdirectory(example)
endif ()