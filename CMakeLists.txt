cmake_minimum_required(VERSION 3.22)

project (AILURUS_INFRASTRUCTURE)
set (CMAKE_CXX_STANDARD 20)
set (AILURUS_TARGET_NAME ailurus)

# SDL
set (SDL_STATIC ON) # static link sdl
set (SDL_SHARED OFF)
add_subdirectory(submodule/sdl)

# spdlog
add_subdirectory(submodule/spdlog)

# Vulkan
find_package(Vulkan REQUIRED)

# Set output path
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/artifacts/${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/)

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

# Library
add_library                 (${AILURUS_TARGET_NAME} STATIC ${AILURUS_SRC} ${AILURUS_IMGUI_SRC})
target_include_directories  (${AILURUS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories  (${AILURUS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/)
target_include_directories  (${AILURUS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/submodule/imgui/) # for imgui source
target_link_libraries       (${AILURUS_TARGET_NAME} PUBLIC SDL3-static Vulkan::Vulkan spdlog)

if (WIN32)
    target_link_libraries   (${AILURUS_TARGET_NAME} PRIVATE wsock32 ws2_32) # for socket
    target_link_libraries   (${AILURUS_TARGET_NAME} PRIVATE dwmapi) # to fix some 'DwmXXX' link errors under mingw
endif ()

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