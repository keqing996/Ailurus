
add_executable          (test_net_dns ./Network/TestDNS.cpp)
target_link_libraries   (test_net_dns piik)

add_executable          (test_net_tcp_client ./Network/TestTcpSocketSend.cpp)
target_link_libraries   (test_net_tcp_client piik)

add_executable          (test_net_tcp_server ./Network/TestTcpSocketReceive.cpp)
target_link_libraries   (test_net_tcp_server piik)

add_executable          (test_util_string ./TestString.cpp)
target_link_libraries   (test_util_string piik)
add_test                (NAME test_string COMMAND test_util_string)

add_executable          (test_console ./TestConsole.cpp)
target_link_libraries   (test_console piik)

add_executable          (test_process ./TestProcess.cpp)
target_link_libraries   (test_process piik)

add_executable          (test_system ./TestSystem.cpp)
target_link_libraries   (test_system piik)

add_executable          (test_command_line ./TestCommandLine.cpp)
target_link_libraries   (test_command_line piik)