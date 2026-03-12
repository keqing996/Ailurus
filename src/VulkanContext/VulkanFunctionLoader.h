#pragma once

#include <string>
#include "VulkanPch.h"

namespace Ailurus
{
    class VulkanFunctionLoader
    {
    public:
        static bool LoadVulkanLibrary();
        static const std::string& GetLoadedLibraryPath();
        static vk::detail::DynamicLoader* GetDynamicLoader();

    private:
        static std::string _libraryPath;
        static vk::detail::DynamicLoader* _loader;
    };
}