#include <vulkan/vulkan_hash.hpp>
#include "VulkanResource.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"

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

	bool VulkanResource::IsMarkDeleted() const
	{
		return _markDeleted;
	}

	size_t VulkanResource::GetRefCount() const
	{
		return _referencedCommandBuffer.size();
	}
} // namespace Ailurus