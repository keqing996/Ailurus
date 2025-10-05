find_package            (Python3 REQUIRED COMPONENTS Interpreter)
find_package            (Vulkan REQUIRED)
file                    (GLOB_RECURSE  AILURUS_RENDER_EXAMPLE_SRC ./*.cpp)

# Platform detection
if(APPLE)
    # macOS Bundle App configuration
    add_executable          (ailurus_example_render MACOSX_BUNDLE ${AILURUS_RENDER_EXAMPLE_SRC})
    
    # Bundle properties
    set_target_properties(ailurus_example_render PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "AilurusRenderExample"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.ailurus.renderexample"
    )
    
    # Set @rpath for Vulkan libraries
    set_target_properties(ailurus_example_render PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
    
else()
    # Windows/Linux executable
    add_executable          (ailurus_example_render ${AILURUS_RENDER_EXAMPLE_SRC})
endif()

target_link_libraries   (ailurus_example_render PUBLIC ailurus-app)

# Platform-specific post-build commands
if(APPLE)
    # macOS Bundle post-build
    add_custom_command      (TARGET ailurus_example_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/shader_gen.py" --vulkan_sdk "$ENV{VULKAN_SDK}" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader" --dst_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Assets Copy Job to Bundle"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/assets_copy.py" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets" --dst_directory "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/Assets"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Copy Vulkan libraries to Bundle"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks"
        # Copy Vulkan libraries from VULKAN_SDK
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VULKAN_SDK}/lib/libvulkan.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VULKAN_SDK}/lib/libvulkan.1.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{VULKAN_SDK}/lib/libMoltenVK.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks/"
        # ICD file
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Copy Vulkan ICD files for standalone distribution"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/vulkan/icd.d"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "$ENV{VULKAN_SDK}/share/vulkan/icd.d" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/vulkan/icd.d"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/fix_icd_paths.py" --icd_dir "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/vulkan/icd.d"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
else()
    # Windows/Linux post-build
    add_custom_command      (TARGET ailurus_example_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/shader_gen.py" --vulkan_sdk "$ENV{VULKAN_SDK}" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader" --dst_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Assets Copy Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/assets_copy.py" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets" --dst_directory "$<TARGET_FILE_DIR:ailurus_example_render>/Assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
endif()