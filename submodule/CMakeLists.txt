
# spdlog
add_subdirectory(spdlog)

# glm
add_subdirectory(glm)
target_compile_definitions(glm PUBLIC GLM_FORCE_LEFT_HANDED)

# sdl
set (SDL_STATIC ON) # static link sdl
set (SDL_SHARED OFF)
add_subdirectory(sdl)

# imgui
find_package(Vulkan REQUIRED)

add_library(imgui STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_vulkan.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_sdl3.cpp
)

target_include_directories  (imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/)
target_link_libraries       (imgui PUBLIC SDL3-static Vulkan::Headers)
