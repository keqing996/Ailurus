#include "VulkanResourceManager.h"
#include "VulkanBuffer.h"

namespace Ailurus
{
	template <>
	VulkanDeviceBuffer* VulkanResourceManager::CreateResource<VulkanDeviceBuffer>()
    {

    }
}