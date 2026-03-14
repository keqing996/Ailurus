cmake_minimum_required(VERSION 3.22)

project (AILURUS_INFRASTRUCTURE)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_COLOR_DIAGNOSTICS ON)

add_subdirectory(submodule)
add_subdirectory(thirdparty)

# Source files
file (GLOB_RECURSE AILURUS_SRC ./src/*.cpp)

# Ailurus Library
add_library                 (ailurus STATIC ${AILURUS_SRC})
target_include_directories  (ailurus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories  (ailurus PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_link_libraries       (ailurus PUBLIC nlohmann_json assimp stb_image spdlog glm)
target_link_libraries       (ailurus PRIVATE SDL3-static vulkan_custom websockets)

if (WIN32)
    target_link_libraries   (ailurus PRIVATE dwmapi) # to fix some 'DwmXXX' link errors under mingw
endif ()

target_precompile_headers   (ailurus PRIVATE "src/VulkanContext/VulkanPch.h")

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