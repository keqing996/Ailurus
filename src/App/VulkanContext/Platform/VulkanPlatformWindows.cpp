#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_WINDOWS

#include "VulkanPlatform.h"

namespace Ailurus
{
    std::optional<std::string> VulkanPlatform::GetVulkanLibraryPath()
    {
        return "vulkan-1.dll"; // System PATH or local directory
    }
    
    std::vector<std::string> VulkanPlatform::GetFallbackPaths()
    {
        return {
            "vulkan-1.dll",
            "./vulkan-1.dll",
            "C:\\Windows\\System32\\vulkan-1.dll",
            "C:\\Program Files\\Vulkan\\Bin\\vulkan-1.dll"
        };
    }

}

#endif