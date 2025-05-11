#include "VulkanSystem.h"
#include <optional>
#include "Ailurus/Application/TimeSystem/TimeSystem.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "VulkanHelper.h"
#include "VulkanSystem/FrameContext/FrameContext.h"

namespace Ailurus
{
	bool VulkanSystem::WaitNextFrame(bool* needRebuild)
	{
		FrameContext* pFrameContext = GetFrameContext();

		// Wait fence

	}

	bool VulkanSystem::SubmitThisFrame(bool* needRebuild)
	{
		// Acquire swap chain next image
		const vk::Semaphore imageReadySemaphore = AllocateSemaphore();

		auto acquireImage = _vkDevice.acquireNextImageKHR(_vkSwapChain,
			std::numeric_limits<uint64_t>::max(), imageReadySemaphore);

		switch (acquireImage.result)
		{
			case vk::Result::eErrorOutOfDateKHR:
				*needRebuild = true;
				return false;
			case vk::Result::eSuboptimalKHR:
				*needRebuild = true;
				break;
			case vk::Result::eSuccess:
				break;
			default:
				Logger::LogError("Fail to acquire next image, result = {}", static_cast<int>(acquireImage.result));
				_semaphorePool.Free(imageReadySemaphore, true);
				return false;
		}

		auto currentSwapChainImageIndex = acquireImage.value;

		// Submit command buffers
		FrameContext* pFrameContext = GetFrameContext();
		const vk::Fence renderFinishFence = AllocateFence();
		vk::Semaphore waitSemaphore = nullptr;
		for (auto i = 0; i < pFrameContext->waitingSubmittedCmdBuffers.size(); i++)
		{
			const auto& cmdBufferInfo = pFrameContext->waitingSubmittedCmdBuffers[i];
			vk::SubmitInfo submitInfo;

			// Target buffer
			submitInfo.setCommandBuffers(cmdBufferInfo.buffer);

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
			if (i == pFrameContext->waitingSubmittedCmdBuffers.size() - 1)
				_vkGraphicQueue.submit(submitInfo, renderFinishFence);
			else
				_vkGraphicQueue.submit(submitInfo);

			// Update last signaled semaphore to next wait semaphore
			waitSemaphore = cmdBufferInfo.signalSemaphore;
		}

		// Collect resource
		pFrameContext->renderingFrameContext = RenderingFrameContext{};
		pFrameContext->renderingFrameContext->renderingFrameCount = Application::Get<TimeSystem>()->FrameCount();
		pFrameContext->renderingFrameContext->allFinishFence = renderFinishFence;
		// todo add image ready sem
		for (auto cmdBufferInfo : pFrameContext->waitingSubmittedCmdBuffers)
		{

		}

		// Present
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(waitSemaphore)
			.setSwapchains(_vkSwapChain)
			.setImageIndices(currentSwapChainImageIndex);

		switch (const auto present = _vkPresentQueue.presentKHR(presentInfo))
		{
			case vk::Result::eErrorOutOfDateKHR:
				*needRebuild = true;
				return false;
			case vk::Result::eSuboptimalKHR:
				*needRebuild = true;
				break;
			case vk::Result::eSuccess:
				break;
			default:
				Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
				return false;
		}

		// Update flight index
		_currentParallelFrameIndex = (_currentParallelFrameIndex + 1) % PARALLEL_FRAME;

		return true;
	}
} // namespace Ailurus