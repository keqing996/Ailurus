cmake_minimum_required(VERSION 3.22)

project (AILURUS_INFRASTRUCTURE)
set (CMAKE_CXX_STANDARD 20)

# SDL
set (SDL_STATIC ON) # static link sdl
set (SDL_SHARED OFF)
add_subdirectory(submodule/sdl)

# spdlog
add_subdirectory(submodule/spdlog)

# Vulkan
find_package(Vulkan REQUIRED)

# Source files
file (GLOB_RECURSE AILURUS_SRC ./src/*.cpp)

set (AILURUS_IMGUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/backends/imgui_impl_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/backends/imgui_impl_sdl3.cpp
)

# Core Library
add_library                 (ailurus-core STATIC ${AILURUS_SRC} ${AILURUS_IMGUI_SRC})
target_include_directories  (ailurus-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories  (ailurus-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/)
target_include_directories  (ailurus-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/) # for imgui source
target_link_libraries       (ailurus-core PUBLIC SDL3-static Vulkan::Vulkan spdlog)

if (WIN32)
    target_link_libraries   (ailurus-core PRIVATE wsock32 ws2_32) # for socket
    target_link_libraries   (ailurus-core PRIVATE dwmapi) # to fix some 'DwmXXX' link errors under mingw
endif ()

# Interface
set (AILURUS_TARGET_NAME ailurus)
add_library (${AILURUS_TARGET_NAME} INTERFACE)
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # To fix Mingw link order problem, "undefined reference to WinMain".
    target_link_libraries (${AILURUS_TARGET_NAME} INTERFACE mingw32 ailurus-core)
else ()
    target_link_libraries (${AILURUS_TARGET_NAME} INTERFACE ailurus-core)
endif()

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