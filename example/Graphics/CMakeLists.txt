find_package            (Python3 REQUIRED COMPONENTS Interpreter)
if(NOT DEFINED AILURUS_VULKAN_SDK)
        if(DEFINED ENV{VULKAN_SDK})
                set(AILURUS_VULKAN_SDK "$ENV{VULKAN_SDK}")
        else()
                message(FATAL_ERROR "Vulkan SDK path not provided. Set AILURUS_VULKAN_SDK or environment variable VULKAN_SDK.")
        endif()
endif()

file                    (REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader"          AILURUS_SHADER_SOURCE_DIR)
file                    (REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"       AILURUS_SHADER_BINARY_DIR)
file                    (GLOB_RECURSE  AILURUS_RENDER_EXAMPLE_SRC ./*.cpp)
add_executable          (ailurus_example_render ${AILURUS_RENDER_EXAMPLE_SRC})
target_link_libraries   (ailurus_example_render PUBLIC ailurus-app)
add_custom_command      (TARGET ailurus_example_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/shader_gen.py" "${AILURUS_VULKAN_SDK}" "${AILURUS_SHADER_SOURCE_DIR}" "${AILURUS_SHADER_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Assets Copy Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/assets_copy.py" "${CMAKE_CURRENT_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:ailurus_example_render>/Assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
)