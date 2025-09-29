#pragma once

#include <string>
#include <optional>
#include <vector>

namespace Ailurus
{
    /**
     * @brief Platform-specific Vulkan library loading utilities
     * 
     * This class provides platform-specific methods to locate and load
     * the appropriate Vulkan library for different operating systems:
     * 
     * - macOS: Handles app bundle vs. command-line execution
     * - Linux: Searches common installation directories
     * - Windows: Uses standard Windows library loading
     * - Android: Handles Android-specific library locations and API requirements
     */
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
    };
}