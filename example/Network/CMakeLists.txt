add_executable          (ailurus_example_network_dns ./ExampleDNS.cpp)
target_link_libraries   (ailurus_example_network_dns PUBLIC ailurus-core)

add_executable          (ailurus_example_network_simple_tcp_sender ./ExampleSimpleTcpSender.cpp)
target_link_libraries   (ailurus_example_network_simple_tcp_sender PUBLIC ailurus-core)

add_executable          (ailurus_example_network_single_tcp_echo_sever ./ExampleTcpSingleEchoSever.cpp)
target_link_libraries   (ailurus_example_network_single_tcp_echo_sever PUBLIC ailurus-core)

add_executable          (ailurus_example_network_multi_tcp_echo_sever ./ExampleTcpMultiEchoSever.cpp)
target_link_libraries   (ailurus_example_network_multi_tcp_echo_sever PUBLIC ailurus-core)