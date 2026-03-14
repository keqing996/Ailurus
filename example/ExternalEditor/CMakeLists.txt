find_package            (Python3 REQUIRED COMPONENTS Interpreter)
find_package            (Vulkan REQUIRED)
file                    (GLOB_RECURSE  AILURUS_EXTERNAL_EDITOR_SRC ./*.cpp)

set                     (AILURUS_EXTERNAL_EDITOR_TARGET ailurus_example_external_editor)
set                     (AILURUS_EXTERNAL_EDITOR_GRAPHICS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../Graphics)
set                     (AILURUS_EXTERNAL_EDITOR_ASSET_DIR ${AILURUS_EXTERNAL_EDITOR_GRAPHICS_DIR}/Assets)
set                     (AILURUS_EXTERNAL_EDITOR_SCRIPT_DIR ${AILURUS_EXTERNAL_EDITOR_GRAPHICS_DIR}/Scripts)

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
    message             (FATAL_ERROR "Unable to locate glslc for the ExternalEditor example. Install Vulkan SDK tools or ensure glslc is discoverable.")
endif()

set                     (AILURUS_VULKAN_ICD_DIR "")
if (AILURUS_VULKAN_ROOT AND EXISTS "${AILURUS_VULKAN_ROOT}/share/vulkan/icd.d")
    set                 (AILURUS_VULKAN_ICD_DIR "${AILURUS_VULKAN_ROOT}/share/vulkan/icd.d")
endif()

if(APPLE)
    add_executable      (${AILURUS_EXTERNAL_EDITOR_TARGET} MACOSX_BUNDLE ${AILURUS_EXTERNAL_EDITOR_SRC})
    set_target_properties(${AILURUS_EXTERNAL_EDITOR_TARGET} PROPERTIES
        BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "AilurusExternalEditorExample"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.ailurus.externaleditor"
        INSTALL_RPATH "@executable_path/../Frameworks"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
else()
    add_executable      (${AILURUS_EXTERNAL_EDITOR_TARGET} ${AILURUS_EXTERNAL_EDITOR_SRC})
endif()

target_link_libraries   (${AILURUS_EXTERNAL_EDITOR_TARGET} PUBLIC ailurus)

if(APPLE)
    set                 (AILURUS_EXTERNAL_EDITOR_POST_BUILD_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E echo "[ExternalEditor] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${AILURUS_EXTERNAL_EDITOR_SCRIPT_DIR}/shader_gen.py" --glslc "${AILURUS_VULKAN_GLSLC}" --src_directory "${AILURUS_EXTERNAL_EDITOR_ASSET_DIR}/Shader" --dst_directory "${AILURUS_EXTERNAL_EDITOR_ASSET_DIR}/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[ExternalEditor] Assets Copy Job to Bundle"
        COMMAND ${Python3_EXECUTABLE} "${AILURUS_EXTERNAL_EDITOR_SCRIPT_DIR}/assets_copy.py" --src_directory "${AILURUS_EXTERNAL_EDITOR_ASSET_DIR}" --dst_directory "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Resources/Assets"
        COMMAND ${CMAKE_COMMAND} -E echo "[ExternalEditor] Copy Vulkan libraries to Bundle"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Frameworks"
    )

    if (AILURUS_VULKAN_LIBRARY_DIR AND EXISTS "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.dylib")
        list            (APPEND AILURUS_EXTERNAL_EDITOR_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Frameworks/"
        )
    endif()

    if (AILURUS_VULKAN_LIBRARY_DIR AND EXISTS "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.1.dylib")
        list            (APPEND AILURUS_EXTERNAL_EDITOR_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AILURUS_VULKAN_LIBRARY_DIR}/libvulkan.1.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Frameworks/"
        )
    endif()

    if (AILURUS_VULKAN_LIBRARY_DIR AND EXISTS "${AILURUS_VULKAN_LIBRARY_DIR}/libMoltenVK.dylib")
        list            (APPEND AILURUS_EXTERNAL_EDITOR_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${AILURUS_VULKAN_LIBRARY_DIR}/libMoltenVK.dylib" "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Frameworks/"
        )
    endif()

    if (AILURUS_VULKAN_ICD_DIR)
        list            (APPEND AILURUS_EXTERNAL_EDITOR_POST_BUILD_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E echo "[ExternalEditor] Copy Vulkan ICD files for standalone distribution"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Resources/vulkan/icd.d"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${AILURUS_VULKAN_ICD_DIR}" "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Resources/vulkan/icd.d"
            COMMAND ${Python3_EXECUTABLE} "${AILURUS_EXTERNAL_EDITOR_SCRIPT_DIR}/fix_icd_paths.py" --icd_dir "$<TARGET_BUNDLE_CONTENT_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Resources/vulkan/icd.d"
        )
    endif()

    add_custom_command  (TARGET ${AILURUS_EXTERNAL_EDITOR_TARGET} POST_BUILD
        ${AILURUS_EXTERNAL_EDITOR_POST_BUILD_COMMANDS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
else()
    add_custom_command  (TARGET ${AILURUS_EXTERNAL_EDITOR_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "[ExternalEditor] Shader Compile Job"
        COMMAND ${Python3_EXECUTABLE} "${AILURUS_EXTERNAL_EDITOR_SCRIPT_DIR}/shader_gen.py" --glslc "${AILURUS_VULKAN_GLSLC}" --src_directory "${AILURUS_EXTERNAL_EDITOR_ASSET_DIR}/Shader" --dst_directory "${AILURUS_EXTERNAL_EDITOR_ASSET_DIR}/ShaderBin"
        COMMAND ${CMAKE_COMMAND} -E echo "[ExternalEditor] Assets Copy Job"
        COMMAND ${Python3_EXECUTABLE} "${AILURUS_EXTERNAL_EDITOR_SCRIPT_DIR}/assets_copy.py" --src_directory "${AILURUS_EXTERNAL_EDITOR_ASSET_DIR}" --dst_directory "$<TARGET_FILE_DIR:${AILURUS_EXTERNAL_EDITOR_TARGET}>/Assets"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
endif()