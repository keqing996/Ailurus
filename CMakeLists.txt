cmake_minimum_required(VERSION 3.22)

project (AILURUS_INFRASTRUCTURE)
set (CMAKE_CXX_STANDARD 20)
set (AILURUS_TARGET_NAME ailurus)

# Set output path
set (LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/artifacts/${CMAKE_CXX_COMPILER_ID}/${CMAKE_BUILD_TYPE}/)

# Source files
file (GLOB_RECURSE AILURUS_SRC ./src/*.cpp)

if (WIN32)
    set (AILURUS_IMGUI_SRC
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_win32.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_dx11.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_opengl3.cpp
            # ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_vulkan.cpp
    )
else ()
    set (AILURUS_IMGUI_SRC )
endif ()

# Library
add_library                 (${AILURUS_TARGET_NAME} STATIC ${AILURUS_SRC} ${AILURUS_IMGUI_SRC})
target_include_directories  (${AILURUS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_include_directories  (${AILURUS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/)
target_include_directories  (${AILURUS_TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/) # for imgui source

if (WIN32)
    target_link_libraries   (${AILURUS_TARGET_NAME} wsock32 ws2_32) # for socket
    target_link_libraries   (${AILURUS_TARGET_NAME} opengl32) # for windows opengl
    target_link_libraries   (${AILURUS_TARGET_NAME} dwmapi) # to fix some 'DwmXXX' link errors under mingw
endif ()

# Test code
option (AILURUS_ENABLE_TEST OFF)

if (AILURUS_ENABLE_TEST)
    # todo
endif ()

# Example code
option (AILURUS_ENABLE_EXAMPLE OFF)

if (AILURUS_ENABLE_EXAMPLE)
    add_executable          (ailurus_example_cmd_parse example/ExampleCmdLine.cpp)
    target_link_libraries   (ailurus_example_cmd_parse PUBLIC ${AILURUS_TARGET_NAME})

    add_executable          (ailurus_example_network_dns example/Network/ExampleDNS.cpp)
    target_link_libraries   (ailurus_example_network_dns PUBLIC ${AILURUS_TARGET_NAME})

    add_executable          (ailurus_example_network_tcp_send example/Network/ExampleTcpSocketSend.cpp)
    target_link_libraries   (ailurus_example_network_tcp_send PUBLIC ${AILURUS_TARGET_NAME})

    add_executable          (ailurus_example_network_tcp_recv example/Network/ExampleTcpSocketReceive.cpp)
    target_link_libraries   (ailurus_example_network_tcp_recv PUBLIC ${AILURUS_TARGET_NAME})

    if (WIN32)
        add_executable          (ailurus_example_window_create example/Window/ExampleWindowCreate.cpp)
        target_link_libraries   (ailurus_example_window_create PUBLIC ${AILURUS_TARGET_NAME})
        set_target_properties   (ailurus_example_window_create PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

        add_executable          (ailurus_example_window_msg_recv example/Window/ExampleMessageReceived.cpp)
        target_link_libraries   (ailurus_example_window_msg_recv PUBLIC ${AILURUS_TARGET_NAME})

        add_executable          (ailurus_example_window_callback example/Window/ExampleWindowCallback.cpp)
        target_link_libraries   (ailurus_example_window_callback PUBLIC ${AILURUS_TARGET_NAME})

        add_executable          (ailurus_example_input_callback example/Window/ExampleInputCallback.cpp)
        target_link_libraries   (ailurus_example_input_callback PUBLIC ${AILURUS_TARGET_NAME})

        add_executable          (ailurus_example_imgui_opengl example/Window/ExampleImGuiOpenGL.cpp)
        target_link_libraries   (ailurus_example_imgui_opengl PUBLIC ${AILURUS_TARGET_NAME})
    endif ()
endif ()