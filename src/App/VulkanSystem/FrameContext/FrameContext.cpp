#include "FrameContext.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	bool FrameContext::WaitFinish()
	{
		if (!renderingFrameContext.has_value())
			return true;

		const auto device = Application::Get<VulkanSystem>()->GetDevice();
		const auto waitFence = device.waitForFences(renderingFrameContext->allFinishFence,
			true, std::numeric_limits<uint64_t>::max());
		if (waitFence != vk::Result::eSuccess)
		{
			Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
			return false;
		}

		// Set the finished frame count
		lastRenderFinishedFrame = renderingFrameContext->renderingFrameCount;

		// Free finished command buffer
		for (const auto cmdBuf : renderingFrameContext->renderingBuffers)
			Application::Get<VulkanSystem>()->FreeCommandBuffer(cmdBuf);

		// Free used semaphores
		for (const auto semaphore : renderingFrameContext->usingSemaphores)
			Application::Get<VulkanSystem>()->FreeSemaphore(semaphore);

		// Reset fence and free
		device.resetFences(renderingFrameContext->allFinishFence);
		Application::Get<VulkanSystem>()->FreeFence(renderingFrameContext->allFinishFence);

		// Clear render context
		renderingFrameContext = std::nullopt;

		return true;
	}

	void FrameContext::AddCommandBuffer(vk::CommandBuffer buffer, vk::Semaphore waitSemaphore, std::vector<vk::PipelineStageFlags> waitStages)
	{
		waitingSubmittedCmdBuffers.emplace_back(RecordedCommandBufferInfo{
			waitSemaphore == nullptr ? std::nullopt : waitSemaphore,
			buffer,
			Application::Get<VulkanSystem>()->AllocateSemaphore(),
			waitStages
		});
	}
} // namespace Ailurus