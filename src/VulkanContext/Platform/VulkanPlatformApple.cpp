#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_APPLE

#include "VulkanPlatform.h"
#include <mach-o/dyld.h>
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    std::optional<std::string> VulkanPlatform::GetVulkanLibraryPath()
    {
        // macOS: Detect if we're running from an app bundle
		char executablePath[1024];
		uint32_t size = sizeof(executablePath);
		if (_NSGetExecutablePath(executablePath, &size) == 0)
		{
			std::string execPath(executablePath);
			
			// Check if we're in an app bundle (path contains .app/Contents/MacOS)
			if (execPath.find(".app/Contents/MacOS") != std::string::npos)
			{
				Logger::LogInfo("VulkanLoadLibrary: Running in macOS app bundle");
				return "@executable_path/../Frameworks/libvulkan.1.dylib";
			}
			else
			{
				Logger::LogInfo("VulkanLoadLibrary: Running as Unix executable on macOS");
				// For development builds or command-line tools
				return "libvulkan.1.dylib";
			}
		}
		
        return std::nullopt;
    }
    
    std::vector<std::string> VulkanPlatform::GetFallbackPaths()
    {
        return {
            "libvulkan.1.dylib",
            "libvulkan.dylib",
            "/usr/local/lib/libvulkan.1.dylib",
            "/opt/homebrew/lib/libvulkan.1.dylib"  // Apple Silicon Homebrew
        };
    }

    const std::vector<const char*>& VulkanPlatform::GetRequiredInstanceExtensions()
    {
        // The VK_KHR_portability_subset extension is required for portability, because
		// Metal does not fully support all Vulkan features.
        static const std::vector<const char*> extensions = {
            VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        };
        return extensions;
    }

    vk::InstanceCreateFlags VulkanPlatform::GetInstanceCreateFlags()
    {
        return vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    }

    const std::vector<const char*>& VulkanPlatform::GetRequiredDeviceExtensions()
    {
        static const std::vector<const char*> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            "VK_KHR_portability_subset",
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        return extensions;
    }
}

#endif