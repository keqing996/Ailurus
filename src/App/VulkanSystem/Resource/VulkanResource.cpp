#include "VulkanResource.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    VulkanResource::~VulkanResource()
    {
        if (_refCount > 0)
            Logger::LogError("VulkanResource has non-zero reference count on destruction: " + std::to_string(_refCount));
    }

	void VulkanResource::AddRef()
	{
		_refCount++;
	}

	void VulkanResource::Release()
	{
		_refCount--;
	}

	void VulkanResource::MarkDelete()
	{
		_markDeleted = true;
	}

	bool VulkanResource::IsValid() const
	{
		return !_markDeleted;
	}

	bool VulkanResource::GetRefCount() const
	{
		return _refCount;
	}
} // namespace Ailurus