#include "FrameContext.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "Ailurus/Utility/Logger.h"
#include <optional>

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
		renderingFrameContext->renderingBuffers.clear();

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

	void FrameContext::AddCommandBuffer(std::unique_ptr<VulkanCommandBuffer>&& pBuffer)
	{
		waitingSubmittedCmdBuffers.emplace_back(RecordedCommandBufferInfo{
			std::nullopt,
			std::move(pBuffer),
			Application::Get<VulkanSystem>()->AllocateSemaphore(),
			{} });
	}

	void FrameContext::AddCommandBuffer(std::unique_ptr<VulkanCommandBuffer>&& pBuffer, vk::Semaphore waitSemaphore)
	{
		waitingSubmittedCmdBuffers.emplace_back(RecordedCommandBufferInfo{
			waitSemaphore,
			std::move(pBuffer),
			Application::Get<VulkanSystem>()->AllocateSemaphore(),
			{} });
	}

	void FrameContext::AddCommandBuffer(std::unique_ptr<VulkanCommandBuffer>&& pBuffer, vk::Semaphore waitSemaphore, std::vector<vk::PipelineStageFlags> waitStages)
	{
		waitingSubmittedCmdBuffers.emplace_back(RecordedCommandBufferInfo{
			waitSemaphore,
			std::move(pBuffer),
			Application::Get<VulkanSystem>()->AllocateSemaphore(),
			waitStages });
	}

	vk::Semaphore FrameContext::SubmitCommandBuffer(vk::Semaphore imageReadySemaphore)
	{
		const vk::Fence renderFinishFence = Application::Get<VulkanSystem>()->AllocateFence();

		// Do submit
		vk::Semaphore waitSemaphore = imageReadySemaphore;
		for (auto i = 0; i < waitingSubmittedCmdBuffers.size(); i++)
		{
			const auto& cmdBufferInfo = waitingSubmittedCmdBuffers[i];
			vk::SubmitInfo submitInfo;

			// Target buffer
			submitInfo.setCommandBuffers(cmdBufferInfo.pCommandBuffer->GetBuffer());

			// Signal semaphore
			submitInfo.setSignalSemaphores(cmdBufferInfo.signalSemaphore);

			// Wait stages
			submitInfo.setWaitDstStageMask(cmdBufferInfo.waitStages);

			// Wait semaphore
			std::vector<vk::Semaphore> waitSemaphores;
			if (waitSemaphore != nullptr)
				waitSemaphores.push_back(waitSemaphore);
			if (cmdBufferInfo.waitSemaphore.has_value())
				waitSemaphores.push_back(*cmdBufferInfo.waitSemaphore);

			submitInfo.setWaitSemaphores(waitSemaphores);

			// If last command buffer, set the finish fence
			if (i == waitingSubmittedCmdBuffers.size() - 1)
				Application::Get<VulkanSystem>()->GetGraphicQueue().submit(submitInfo, renderFinishFence);
			else
				Application::Get<VulkanSystem>()->GetGraphicQueue().submit(submitInfo);

			// Update last signaled semaphore to next wait semaphore
			waitSemaphore = cmdBufferInfo.signalSemaphore;
		}

		// Collect resources
		renderingFrameContext = RenderingFrameContext{};
		{
			// Record rendering frame count
			renderingFrameContext->renderingFrameCount = Application::Get<TimeSystem>()->FrameCount();

			// Record render finish fence
			renderingFrameContext->allFinishFence = renderFinishFence;

			// Record all used semaphores & buffers
			renderingFrameContext->usingSemaphores.insert(imageReadySemaphore);
			for (auto& cmdBufferInfo : waitingSubmittedCmdBuffers)
			{
				renderingFrameContext->renderingBuffers.push_back(std::move(cmdBufferInfo.pCommandBuffer));
				renderingFrameContext->usingSemaphores.insert(cmdBufferInfo.signalSemaphore);
				if (cmdBufferInfo.waitSemaphore.has_value())
					renderingFrameContext->usingSemaphores.insert(*cmdBufferInfo.waitSemaphore);
			}
		}

		// Clean up
		waitingSubmittedCmdBuffers.clear();

		return waitSemaphore;
	}
} // namespace Ailurus