find_package            (Python3 REQUIRED COMPONENTS Interpreter)
find_package            (Vulkan REQUIRED)
file                    (GLOB_RECURSE  AILURUS_RENDER_EXAMPLE_SRC ./*.cpp)

set                     (AILURUS_VULKAN_GLSLC "")
if (DEFINED Vulkan_GLSLC_EXECUTABLE AND EXISTS "${Vulkan_GLSLC_EXECUTABLE}")
    set                 (AILURUS_VULKAN_GLSLC "${Vulkan_GLSLC_EXECUTABLE}")
else()
    find_program        (AILURUS_VULKAN_GLSLC NAMES glslc HINTS "$ENV{VULKAN_SDK}/bin")
endif()

set                     (AILURUS_VULKAN_LIBRARY_FILE "")
foreach (candidate ${Vulkan_LIBRARIES})
    if (EXISTS "${candidate}")
        set             (AILURUS_VULKAN_LIBRARY_FILE "${candidate}")
        break()
    endif()
endforeach()

set                     (AILURUS_VULKAN_LIBRARY_DIR "")
set                     (AILURUS_VULKAN_ROOT "")
if (AILURUS_VULKAN_LIBRARY_FILE)
    get_filename_component(AILURUS_VULKAN_LIBRARY_DIR "${AILURUS_VULKAN_LIBRARY_FILE}" DIRECTORY)
    get_filename_component(AILURUS_VULKAN_ROOT "${AILURUS_VULKAN_LIBRARY_DIR}" DIRECTORY)
endif()

if (NOT AILURUS_VULKAN_GLSLC AND AILURUS_VULKAN_ROOT AND EXISTS "${AILURUS_VULKAN_ROOT}/bin/glslc")
    set                 (AILURUS_VULKAN_GLSLC "${AILURUS_VULKAN_ROOT}/bin/glslc")
endif()

if (NOT AILURUS_VULKAN_GLSLC)
    message             (FATAL_ERROR "Unable to locate glslc for the Graphics example. Install Vulkan SDK tools or ensure glslc is discoverable.")
endif()

set                     (AILURUS_VULKAN_ICD_DIR "")
if (AILURUS_VULKAN_ROOT AND EXISTS "${AILURUS_VULKAN_ROOT}/share/vulkan/icd.d")
    set                 (AILURUS_VULKAN_ICD_DIR "${AILURUS_VULKAN_ROOT}/share/vulkan/icd.d")
endif()

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

target_link_libraries   (ailurus_example_render PUBLIC ailurus)

# Platform-specific post-build commands
if(APPLE)
    set                 (AILURUS_RENDER_POST_BUILD_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/shader_gen.py" --glslc "${AILURUS_VULKAN_GLSLC}" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader" --dst_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Assets Copy Job to Bundle"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/assets_copy.py" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets" --dst_directory "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/Assets"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Copy Vulkan libraries to Bundle"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks"
    )

    if (AILURUS_VULKAN_LIBRARY_DIR AND EXISTS "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.dylib")
        list            (APPEND AILURUS_RENDER_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks/"
        )
    endif()

    if (AILURUS_VULKAN_LIBRARY_DIR AND EXISTS "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.1.dylib")
        list            (APPEND AILURUS_RENDER_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.1.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks/"
        )
    endif()

    if (AILURUS_VULKAN_LIBRARY_DIR AND EXISTS "${AILURUS_VULKAN_LIBRARY_DIR}/libMoltenVK.dylib")
        list            (APPEND AILURUS_RENDER_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AILURUS_VULKAN_LIBRARY_DIR}/libMoltenVK.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Frameworks/"
        )
    endif()

    if (AILURUS_VULKAN_ICD_DIR)
        list            (APPEND AILURUS_RENDER_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E echo "[Render] Copy Vulkan ICD files for standalone distribution"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/vulkan/icd.d"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${AILURUS_VULKAN_ICD_DIR}" "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/vulkan/icd.d"
            COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/fix_icd_paths.py" --icd_dir "$<TARGET_BUNDLE_CONTENT_DIR:ailurus_example_render>/Resources/vulkan/icd.d"
        )
    endif()

    # macOS Bundle post-build
    add_custom_command      (TARGET ailurus_example_render POST_BUILD
        ${AILURUS_RENDER_POST_BUILD_COMMANDS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
else()
    # Windows/Linux post-build
    add_custom_command      (TARGET ailurus_example_render POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/shader_gen.py" --glslc "${AILURUS_VULKAN_GLSLC}" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shader" --dst_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[Render] Assets Copy Job"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/Scripts/assets_copy.py" --src_directory "${CMAKE_CURRENT_SOURCE_DIR}/Assets" --dst_directory "$<TARGET_FILE_DIR:ailurus_example_render>/Assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
endif()