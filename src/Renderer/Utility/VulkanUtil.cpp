#include <string_view>
#include "Ailurus/Renderer/Utility/VulkanUtil.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    bool VulkanUtil::VerboseLog = true;

    VkBool32 VulkanUtil::DebugReportExtCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, std::uint64_t,
                                                std::size_t, std::int32_t, const char*, const char* pMessage, void*)
    {
        Logger::LogError(pMessage);
        return VK_FALSE;
    }
}
