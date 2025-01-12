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
    enable_testing()

    add_executable          (ailurus_test_math test/TestVector.cpp)
    target_link_libraries   (ailurus_test_math PUBLIC ${AILURUS_TARGET_NAME})
    add_test                (NAME ailurus_test_math COMMAND ailurus_test_math)


    add_executable          (ailurus_test_string test/TestString.cpp)
    target_link_libraries   (ailurus_test_string PUBLIC ${AILURUS_TARGET_NAME})
    add_test                (NAME ailurus_test_string COMMAND ailurus_test_string)

    add_executable          (ailurus_test_lock_free_queue test/TestLockFreeQueue.cpp)
    target_link_libraries   (ailurus_test_lock_free_queue PUBLIC ${AILURUS_TARGET_NAME})
    add_test                (NAME ailurus_test_lock_free_queue COMMAND ailurus_test_lock_free_queue)

    add_executable          (ailurus_test_thread_pool test/TestThreadPool.cpp)
    target_link_libraries   (ailurus_test_thread_pool PUBLIC ${AILURUS_TARGET_NAME})
    add_test                (NAME ailurus_test_thread_pool COMMAND ailurus_test_thread_pool)
endif ()

# Example code
option (AILURUS_ENABLE_EXAMPLE OFF)

if (AILURUS_ENABLE_EXAMPLE)
    # Command line
    add_executable          (ailurus_example_cmd_parse example/ExampleCmdLine.cpp)
    target_link_libraries   (ailurus_example_cmd_parse PUBLIC ${AILURUS_TARGET_NAME})

    # Network
    add_executable          (ailurus_example_network_dns example/Network/ExampleDNS.cpp)
    target_link_libraries   (ailurus_example_network_dns PUBLIC ${AILURUS_TARGET_NAME})

    add_executable          (ailurus_example_network_simple_tcp_sender example/Network/ExampleSimpleTcpSender.cpp)
    target_link_libraries   (ailurus_example_network_simple_tcp_sender PUBLIC ${AILURUS_TARGET_NAME})

    add_executable          (ailurus_example_network_single_tcp_echo_sever example/Network/ExampleTcpSingleEchoSever.cpp)
    target_link_libraries   (ailurus_example_network_single_tcp_echo_sever PUBLIC ${AILURUS_TARGET_NAME})

    add_executable          (ailurus_example_network_multi_tcp_echo_sever example/Network/ExampleTcpMultiEchoSever.cpp)
    target_link_libraries   (ailurus_example_network_multi_tcp_echo_sever PUBLIC ${AILURUS_TARGET_NAME})

    # System
    add_executable          (ailurus_example_system example/ExampleSystem.cpp)
    target_link_libraries   (ailurus_example_system PUBLIC ${AILURUS_TARGET_NAME})

    if (WIN32)
        # Native window
        add_executable          (ailurus_example_window_create example/Window/ExampleWindowCreate.cpp)
        target_link_libraries   (ailurus_example_window_create PUBLIC ${AILURUS_TARGET_NAME})
        set_target_properties   (ailurus_example_window_create PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

        add_executable          (ailurus_example_window_msg_recv example/Window/ExampleMessageReceived.cpp)
        target_link_libraries   (ailurus_example_window_msg_recv PUBLIC ${AILURUS_TARGET_NAME})

        add_executable          (ailurus_example_window_callback example/Window/ExampleWindowCallback.cpp)
        target_link_libraries   (ailurus_example_window_callback PUBLIC ${AILURUS_TARGET_NAME})

        add_executable          (ailurus_example_input_callback example/Window/ExampleInputCallback.cpp)
        target_link_libraries   (ailurus_example_input_callback PUBLIC ${AILURUS_TARGET_NAME})

        add_executable          (ailurus_example_imgui_opengl example/Window/ExampleImGuiOpenGL.cpp thirdparty/imgui/imgui_demo.cpp)
        target_link_libraries   (ailurus_example_imgui_opengl PUBLIC ${AILURUS_TARGET_NAME})

        # Console
        add_executable          (ailurus_example_console example/ExampleConsole.cpp)
        target_link_libraries   (ailurus_example_console PUBLIC ${AILURUS_TARGET_NAME})
    endif ()
endif ()