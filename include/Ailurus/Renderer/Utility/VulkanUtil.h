#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    class VulkanUtil
    {
    public:
        VulkanUtil() = delete;

    public:
        static bool VerboseLog;


        static VKAPI_ATTR VkBool32 VKAPI_CALL
        DebugReportExtCallback(
            VkDebugReportFlagsEXT,
            VkDebugReportObjectTypeEXT,
            std::uint64_t,
            std::size_t,
            std::int32_t,
            const char*,
            const char* pMessage,
            void*);
    };
}
