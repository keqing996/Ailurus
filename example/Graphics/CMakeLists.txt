
add_custom_target       (ailurus_example_render_shader_compile_job ALL
        COMMENT "[Render] Shader Compile Job"
        COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/shader_gen.py" "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader" "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"
)

add_custom_target       (ailurus_example_render_assets_copy_job ALL
        COMMENT "[Render] Assets Copy Job"
        COMMAND python "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/assets_copy.py" "${CMAKE_CURRENT_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:ailurus_example_render>/Assets"
)

file                    (GLOB_RECURSE  AILURUS_RENDER_EXAMPLE_SRC ./*.cpp)
add_executable          (ailurus_example_render ${AILURUS_RENDER_EXAMPLE_SRC})
target_link_libraries   (ailurus_example_render PUBLIC ailurus-app)
add_dependencies        (ailurus_example_render_assets_copy_job ailurus_example_render_shader_compile_job)
add_dependencies        (ailurus_example_render ailurus_example_render_assets_copy_job)