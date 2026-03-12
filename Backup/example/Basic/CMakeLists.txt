add_executable          (ailurus_example_cmd_parse ExampleCmdLine.cpp)
target_link_libraries   (ailurus_example_cmd_parse PUBLIC ailurus-core)

add_executable          (ailurus_example_process ExampleProcess.cpp)
target_link_libraries   (ailurus_example_process PUBLIC ailurus-core)

add_executable          (ailurus_example_system ExampleSystem.cpp)
target_link_libraries   (ailurus_example_system PUBLIC ailurus-core)

if (WIN32)
    # Console
    add_executable          (ailurus_example_console ExampleConsole.cpp)
    target_link_libraries   (ailurus_example_console PUBLIC ailurus-app)
endif ()