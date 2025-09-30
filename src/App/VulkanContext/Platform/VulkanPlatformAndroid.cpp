#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_ANDROID

#include "VulkanPlatform.h"
#include "Ailurus/Utility/Logger.h"
#include <android/api-level.h>
#include <dlfcn.h>
#include <unistd.h>

namespace Ailurus
{
    std::optional<std::string> VulkanPlatform::GetVulkanLibraryPath()
    {
        // Android Vulkan library path handling
        
        // Android API level check - Vulkan is available from API 24+
        int apiLevel = android_get_device_api_level();
        if (apiLevel < 24)
        {
            Logger::LogError("VulkanLoadLibrary: Vulkan requires Android API level 24+, current: {}", apiLevel);
            return std::nullopt;
        }
        
        // Android Vulkan library search paths in priority order
        std::vector<std::string> searchPaths = {
            // System library (most common)
            "libvulkan.so",
            
            // Full system paths for different Android architectures
            "/system/lib64/libvulkan.so",     // 64-bit ARM/x86_64
            "/system/lib/libvulkan.so",       // 32-bit ARM/x86
            "/vendor/lib64/libvulkan.so",     // Vendor-specific 64-bit
            "/vendor/lib/libvulkan.so",       // Vendor-specific 32-bit
            
            // Alternative system paths (some OEMs)
            "/system/lib64/egl/libvulkan.so",
            "/system/lib/egl/libvulkan.so",
            
            // APK-bundled library (for apps that include Vulkan)
            "./libvulkan.so",
            "../lib/libvulkan.so"
        };
        
        // Try to load each path to verify it exists and is accessible
        for (const auto& path : searchPaths)
        {
            // Use dlopen to test if the library can be loaded
            void* handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_NOLOAD);
            if (handle != nullptr)
            {
                dlclose(handle);
                Logger::LogInfo("VulkanLoadLibrary: Found Vulkan library at: {}", path);
                return path;
            }
            
            // Alternative check using access for file existence
            if (access(path.c_str(), F_OK) == 0)
            {
                Logger::LogInfo("VulkanLoadLibrary: Found accessible Vulkan library at: {}", path);
                return path;
            }
        }
        
        // Fallback: try system default
        Logger::LogInfo("VulkanLoadLibrary: Using Android system default Vulkan library");
        return "libvulkan.so";
    }
    
    std::vector<std::string> VulkanPlatform::GetFallbackPaths()
    {
        return {
            "libvulkan.so",
            "/system/lib64/libvulkan.so",
            "/system/lib/libvulkan.so",
            "/vendor/lib64/libvulkan.so",
            "/vendor/lib/libvulkan.so"
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