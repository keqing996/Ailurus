#include "VulkanFunctionLoader.h"
#include "Platform/VulkanPlatform.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    std::string                 VulkanFunctionLoader::_libraryPath = "";
    vk::detail::DynamicLoader*  VulkanFunctionLoader::_loader = nullptr;

    bool VulkanFunctionLoader::LoadVulkanLibrary()
    {
        if (_loader != nullptr)
        {
            Logger::LogWarn("Vulkan library already loaded from: {}", _libraryPath);
            return true;
        }

        auto vulkanLibPathOpt = VulkanPlatform::GetVulkanLibraryPath();
        if (!vulkanLibPathOpt.has_value())
        {
            Logger::LogError("VulkanContext: No Vulkan library path found for this platform");
            return false;
        }

        const auto& vulkanLibPath = vulkanLibPathOpt.value();
        Logger::LogInfo("VulkanContext: Trying to load Vulkan library from: {}", vulkanLibPath);

        // Try primary path
        vk::detail::DynamicLoader loader(vulkanLibPath.c_str());
        if (loader.success())
        {
            Logger::LogInfo("VulkanContext: Successfully loaded Vulkan library from: {}", vulkanLibPath);
            _libraryPath = vulkanLibPath;
            _loader = new vk::detail::DynamicLoader(std::move(loader));
            return true;
        }

        Logger::LogWarn("VulkanContext: Failed to load primary Vulkan library path: {}", vulkanLibPath);

        // Try fallback paths
        auto fallbackPaths = VulkanPlatform::GetFallbackPaths();
        for (const auto& fallbackPath : fallbackPaths)
        {
            Logger::LogInfo("VulkanContext: Trying fallback path: {}", fallbackPath);
            vk::detail::DynamicLoader fallbackLoader(fallbackPath.c_str());
            if (fallbackLoader.success())
            {
                Logger::LogInfo("VulkanContext: Successfully loaded Vulkan library from fallback: {}", fallbackPath);
                _libraryPath = fallbackPath;
                _loader = new vk::detail::DynamicLoader(std::move(fallbackLoader));
                return true;
            }
        }

        // If all attempts failed
        Logger::LogError("VulkanContext: Failed to load Vulkan library from all attempted paths");
        return false;
    }

    const std::string& VulkanFunctionLoader::GetLoadedLibraryPath()
    {
        return _libraryPath;
    }

    vk::detail::DynamicLoader* VulkanFunctionLoader::GetDynamicLoader()
    {
        return _loader;
    }
}