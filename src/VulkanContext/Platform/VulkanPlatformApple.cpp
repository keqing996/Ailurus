#include "Ailurus/PlatformDefine.h"

#if AILURUS_PLATFORM_APPLE

#include "VulkanPlatform.h"
#include <mach-o/dyld.h>
#include <filesystem>
#include <vector>
#include "Ailurus/OS/System.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    namespace
    {
        std::optional<std::filesystem::path> GetBundleContentsPath()
        {
            std::vector<char> executablePath(1024, '\0');
            uint32_t size = static_cast<uint32_t>(executablePath.size());
            if (_NSGetExecutablePath(executablePath.data(), &size) != 0)
            {
                executablePath.resize(size);
                if (_NSGetExecutablePath(executablePath.data(), &size) != 0)
                    return std::nullopt;
            }

            std::filesystem::path execPath = std::filesystem::path(executablePath.data()).lexically_normal();
            if (execPath.string().find(".app/Contents/MacOS") == std::string::npos)
                return std::nullopt;

            auto macOSDir = execPath.parent_path();
            auto contentsDir = macOSDir.parent_path();
            if (macOSDir.filename() != "MacOS" || contentsDir.filename() != "Contents")
                return std::nullopt;

            return contentsDir;
        }

        void ConfigureBundledDriverEnvironment(const std::filesystem::path& bundleContentsPath)
        {
            auto bundledIcdPath = bundleContentsPath / "Resources" / "vulkan" / "icd.d" / "MoltenVK_icd.json";
            if (!std::filesystem::exists(bundledIcdPath))
            {
                Logger::LogWarn("VulkanLoadLibrary: Bundled MoltenVK ICD manifest not found: {}", bundledIcdPath.string());
                return;
            }

            System::SetEnvironmentVariable("VK_DRIVER_FILES", bundledIcdPath.string());
            System::SetEnvironmentVariable("VK_ICD_FILENAMES", bundledIcdPath.string());
            System::SetEnvironmentVariable("VK_ADD_DRIVER_FILES", "");
            Logger::LogInfo("VulkanLoadLibrary: Forced bundled MoltenVK ICD: {}", bundledIcdPath.string());
        }
    }

    std::optional<std::string> VulkanPlatform::GetVulkanLibraryPath()
    {
        if (auto bundleContentsPath = GetBundleContentsPath(); bundleContentsPath.has_value())
        {
            ConfigureBundledDriverEnvironment(*bundleContentsPath);
            Logger::LogInfo("VulkanLoadLibrary: Running in macOS app bundle");
            return "@executable_path/../Frameworks/libvulkan.1.dylib";
        }

        Logger::LogInfo("VulkanLoadLibrary: Running as Unix executable on macOS");
        // For development builds or command-line tools
        return "libvulkan.1.dylib";
		
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