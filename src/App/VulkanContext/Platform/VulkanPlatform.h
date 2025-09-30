#pragma once

#include <string>
#include <optional>
#include <vector>
#include "VulkanContext/VulkanPch.h"

namespace Ailurus
{
    class VulkanPlatform
    {
    public:
        /**
         * @brief Get the platform-appropriate Vulkan library path
         * 
         * @return std::optional<std::string> Path to Vulkan library, or std::nullopt if not found
         */
        static std::optional<std::string> GetVulkanLibraryPath();
        
        /**
         * @brief Get fallback library paths for the current platform
         * 
         * @return std::vector<std::string> List of fallback paths to try
         */
        static std::vector<std::string> GetFallbackPaths();

        /**
         * @brief Get platform-specific required Vulkan instance extensions
         * 
         * Returns a list of Vulkan instance extensions that are required
         * for the current platform.
         * 
         * @return const std::vector<const char*>& Reference to vector of extension names
         */
        static const std::vector<const char*>& GetRequiredInstanceExtensions();

        /**
         * @brief Get platform-specific Vulkan instance creation flags
         * 
         * Returns flags that should be used when creating a Vulkan instance
         * on the current platform (e.g., portability enumeration on macOS).
         * 
         * @return vk::InstanceCreateFlags Platform-specific creation flags
         */
        static vk::InstanceCreateFlags GetInstanceCreateFlags();

        /**
         * @brief Get platform-specific required Vulkan device extensions
         * 
         * Returns a list of Vulkan device extensions that are required
         * for the current platform (e.g., swapchain extension for presentation).
         * 
         * @return const std::vector<const char*>& Reference to vector of extension names
         */
        static const std::vector<const char*>& GetRequiredDeviceExtensions();
    };
}