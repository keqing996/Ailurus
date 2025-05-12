#include <vulkan/vulkan_hash.hpp>
#include "VulkanResource.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	VulkanResource::VulkanResource()
	{
	}

    VulkanResource::~VulkanResource()
    {
        if (!_referencedCommandBuffer.empty())
            Logger::LogError("VulkanResource has non-zero reference count on destruction: " + std::to_string(_referencedCommandBuffer.size()));
    }

	void VulkanResource::AddRef(const VulkanCommandBuffer& pCommandBuffer)
	{
		_referencedCommandBuffer.insert(pCommandBuffer.GetBuffer());
	}

	void VulkanResource::RemoveRef(const VulkanCommandBuffer& pCommandBuffer)
	{
		_referencedCommandBuffer.erase(pCommandBuffer.GetBuffer());
	}

	void VulkanResource::MarkDelete()
	{
		_markDeleted = true;
	}

	bool VulkanResource::IsValid() const
	{
		return !_markDeleted;
	}
} // namespace Ailurus