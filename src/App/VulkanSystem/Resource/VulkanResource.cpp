#include "VulkanResource.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"

namespace Ailurus
{
	VulkanResource::VulkanResource()
	{

	}

    VulkanResource::~VulkanResource()
    {
        if (!_referencedFrames.empty())
            Logger::LogError("VulkanResource has non-zero reference count on destruction: " + std::to_string(_referencedFrames.size()));
    }

	void VulkanResource::AddRef()
	{
		auto thisFrame = Application::Get<TimeSystem>()->FrameCount();
		_referencedFrames.insert(thisFrame);
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