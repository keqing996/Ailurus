#include "FrameContext.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	FrameContext::FrameContext()
        : commandBuffer()
        , renderingFrame(0)
	{
        auto device = Application::Get<VulkanSystem>()->GetDevice();

        vk::SemaphoreCreateInfo semaphoreInfo;
        imageReadySemaphore = device.createSemaphore(semaphoreInfo);
        renderFinishSemaphore = device.createSemaphore(semaphoreInfo);

        vk::FenceCreateInfo fenceInfo;
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
        fence = device.createFence(fenceInfo);
	}

	FrameContext::~FrameContext()
	{
        auto device = Application::Get<VulkanSystem>()->GetDevice();

        device.destroyFence(fence);
        
        device.destroySemaphore(imageReadySemaphore);
        device.destroySemaphore(renderFinishSemaphore);
	}
} // namespace Ailurus