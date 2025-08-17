add_executable          (ailurus_example_window_create ./ExampleWindowCreate.cpp)
target_link_libraries   (ailurus_example_window_create PUBLIC ailurus-app)

add_executable          (ailurus_example_window_imgui ./ExampleWindowImGui.cpp ../../submodule/imgui/imgui_demo.cpp)
target_link_libraries   (ailurus_example_window_imgui PUBLIC ailurus-app)

add_executable          (ailurus_example_window_callback ./ExampleWindowCallback.cpp)
target_link_libraries   (ailurus_example_window_callback PUBLIC ailurus-app)

add_executable          (ailurus_example_input_callback ./ExampleInputCallback.cpp)
target_link_libraries   (ailurus_example_input_callback PUBLIC ailurus-app)