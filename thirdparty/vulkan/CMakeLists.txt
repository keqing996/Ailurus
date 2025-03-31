
find_package(Vulkan REQUIRED)

add_library                 (vulkan_hpp INTERFACE )
target_link_libraries       (vulkan_hpp INTERFACE Vulkan::Vulkan)
target_compile_definitions  (vulkan_hpp INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
