
find_package(Vulkan REQUIRED)

message(STATUS "Vulkan found:")
message(STATUS "  Vulkan_FOUND: ${Vulkan_FOUND}")
message(STATUS "  Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "  Vulkan_LIBRARIES: ${Vulkan_LIBRARIES}")
message(STATUS "  Vulkan_VERSION: ${Vulkan_VERSION}")
message(STATUS "  AILURUS_VULKAN_STATIC: ${AILURUS_VULKAN_STATIC}")

add_library                 (vulkan_custom INTERFACE )
target_link_libraries       (vulkan_custom INTERFACE Vulkan::Vulkan)
target_compile_definitions  (vulkan_custom INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)