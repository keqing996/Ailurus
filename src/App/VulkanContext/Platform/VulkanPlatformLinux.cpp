#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_LINUX

#include "VulkanPlatform.h"
#include <unistd.h>
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    std::optional<std::string> VulkanPlatform::GetVulkanLibraryPath()
    {
        // Linux: Check common installation paths and local directories
		std::vector<std::string> searchPaths = {
			"./libvulkan.so.1",              // Local directory (development)
			"../lib/libvulkan.so.1",         // Relative lib directory
			"libvulkan.so.1",                // System library
			"/usr/lib/x86_64-linux-gnu/libvulkan.so.1", // Ubuntu/Debian
			"/usr/lib64/libvulkan.so.1",     // CentOS/RHEL/Fedora
			"/usr/local/lib/libvulkan.so.1"  // Local installation
		};
		
		// Check which path exists
		for (const auto& path : searchPaths)
		{
			if (access(path.c_str(), F_OK) == 0)
			{
				Logger::LogInfo("VulkanLoadLibrary: Found Vulkan library at: {}", path);
				return path;
			}
		}

		Logger::LogInfo("VulkanLoadLibrary: Using system default Vulkan library");
		return "libvulkan.so.1";
    }
    
    std::vector<std::string> VulkanPlatform::GetFallbackPaths()
    {
        return {
            "libvulkan.so.1",
            "libvulkan.so",
            "/usr/lib/libvulkan.so.1",
            "/usr/lib/x86_64-linux-gnu/libvulkan.so.1",
            "/usr/lib64/libvulkan.so.1",
            "/usr/local/lib/libvulkan.so.1"
        };
    }

	const std::vector<const char*>& VulkanPlatform::GetRequiredInstanceExtensions()
    {
        static const std::vector<const char*> extensions = { };
        return extensions;
    }

	vk::InstanceCreateFlags VulkanPlatform::GetInstanceCreateFlags()
    {
        return {};
    }

	const std::vector<const char*>& VulkanPlatform::GetRequiredDeviceExtensions()
    {
        static const std::vector<const char*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        
        return extensions;
    }
}

#endif