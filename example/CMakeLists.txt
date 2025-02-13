
# Command line
add_executable          (ailurus_example_cmd_parse ExampleCmdLine.cpp)
target_link_libraries   (ailurus_example_cmd_parse PUBLIC ${AILURUS_TARGET_NAME})

# Network
add_executable          (ailurus_example_network_dns Network/ExampleDNS.cpp)
target_link_libraries   (ailurus_example_network_dns PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_network_simple_tcp_sender Network/ExampleSimpleTcpSender.cpp)
target_link_libraries   (ailurus_example_network_simple_tcp_sender PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_network_single_tcp_echo_sever Network/ExampleTcpSingleEchoSever.cpp)
target_link_libraries   (ailurus_example_network_single_tcp_echo_sever PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_network_multi_tcp_echo_sever Network/ExampleTcpMultiEchoSever.cpp)
target_link_libraries   (ailurus_example_network_multi_tcp_echo_sever PUBLIC ${AILURUS_TARGET_NAME})

# System
add_executable          (ailurus_example_system ExampleSystem.cpp)
target_link_libraries   (ailurus_example_system PUBLIC ${AILURUS_TARGET_NAME})

# Native window
add_executable          (ailurus_example_window_create Window/ExampleWindowCreate.cpp)
target_link_libraries   (ailurus_example_window_create PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_window_imgui Window/ExampleWindowImGui.cpp ../submodule/imgui/imgui_demo.cpp)
target_link_libraries   (ailurus_example_window_imgui PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_window_msg_recv Window/ExampleMessageReceived.cpp)
target_link_libraries   (ailurus_example_window_msg_recv PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_window_callback Window/ExampleWindowCallback.cpp)
target_link_libraries   (ailurus_example_window_callback PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_input_callback Window/ExampleInputCallback.cpp)
target_link_libraries   (ailurus_example_input_callback PUBLIC ${AILURUS_TARGET_NAME})

add_executable          (ailurus_example_window_triangle Window/ExampleWindowVulkanTriangle.cpp)
target_link_libraries   (ailurus_example_window_triangle PUBLIC ${AILURUS_TARGET_NAME})
add_custom_command(
        TARGET ailurus_example_window_triangle POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/Window/Shader/triangle.vert.spv"
            "$<TARGET_FILE_DIR:ailurus_example_window_triangle>"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/Window/Shader/triangle.frag.spv"
            "$<TARGET_FILE_DIR:ailurus_example_window_triangle>"
)

add_executable          (ailurus_example_imgui Window/ExampleWindowImGui.cpp ../submodule/imgui/imgui_demo.cpp)
target_link_libraries   (ailurus_example_imgui PUBLIC ${AILURUS_TARGET_NAME})

if (WIN32)
    # Console
    add_executable          (ailurus_example_console ExampleConsole.cpp)
    target_link_libraries   (ailurus_example_console PUBLIC ${AILURUS_TARGET_NAME})
endif ()