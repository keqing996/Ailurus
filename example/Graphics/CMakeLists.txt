find_package            (Python3 REQUIRED COMPONENTS Interpreter)
file                    (GLOB_RECURSE  AILURUS_RENDER_EXAMPLE_SRC ./*.cpp)
add_executable          (ailurus_example_render ${AILURUS_RENDER_EXAMPLE_SRC})
target_link_libraries   (ailurus_example_render PUBLIC ailurus-app)
add_custom_command      (TARGET ailurus_example_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/shader_gen.py" "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader" "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Assets Copy Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/assets_copy.py" "${CMAKE_CURRENT_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:ailurus_example_render>/Assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
)