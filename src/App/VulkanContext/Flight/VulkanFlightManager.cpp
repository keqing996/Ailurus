#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "VulkanFlightManager.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"

namespace Ailurus
{
	VulkanFlightManager::VulkanFlightManager(uint32_t parallelFrames)
		: _parallelFrame(parallelFrames)
	{
		for (auto i = 0; i < parallelFrames; ++i)
		{
			FrameContext frameContext;
			frameContext.onAirInfo = std::nullopt; // Not in flight
			frameContext.pRenderingCommandBuffer = std::make_unique<VulkanCommandBuffer>();
			frameContext.pFrameDescriptorAllocator = std::make_unique<VulkanDescriptorAllocator>();
			frameContext.imageReadySemaphore = std::make_unique<VulkanSemaphore>();
			frameContext.renderFinishSemaphore = std::make_unique<VulkanSemaphore>();
			frameContext.renderFinishFence = std::make_unique<VulkanFence>(false);
			_frameContext.push_back(std::move(frameContext));
		}
	}

	VulkanFlightManager::~VulkanFlightManager()
	{
		WaitAllFlight();
	}

	void VulkanFlightManager::WaitCurrentFlightReady()
	{
		WaitOneFlight(_currentFlightIndex);
	}

	bool VulkanFlightManager::WaitOneFlight(uint32_t index)
	{
		auto& context = _frameContext[index];

		// Not in flight
		if (!context.onAirInfo.has_value())
			return true;

		// Wait render finish fence
		const auto waitFence = VulkanContext::GetDevice().waitForFences(context.renderFinishFence->GetFence(),
			true, std::numeric_limits<uint64_t>::max());
		if (waitFence != vk::Result::eSuccess)
		{
			Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
			return false;
		}

		// Reset frame resource
		context.pFrameDescriptorAllocator->ResetPool();

		// Reset on air info
		context.onAirInfo = std::nullopt;

		return true;
	}

	void VulkanFlightManager::WaitAllFlight()
	{
		for (auto i = 0; i < _frameContext.size(); i++)
			WaitOneFlight(i);
	}

	bool VulkanFlightManager::TakeOffFlight(uint32_t imageIndex, bool* needRebuild)
	{
		auto& context = _frameContext[_currentFlightIndex];

		// Record end
		context.pRenderingCommandBuffer->End();

		// Do submit
		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(context.pRenderingCommandBuffer->GetBuffer())
			.setSignalSemaphores(context.renderFinishSemaphore->GetSemaphore())
			.setWaitSemaphores(context.imageReadySemaphore->GetSemaphore());

		// Wait stages
		std::vector<vk::PipelineStageFlags> waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitDstStageMask(waitStages);

		// Submit
		VulkanContext::GetGraphicQueue().submit(submitInfo, context.renderFinishFence->GetFence());

		// Set on air info
		context.onAirInfo = OnAirInfo{
			.frameCount = Application::Get<TimeSystem>()->FrameCount()
		};

		// Present
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(context.renderFinishSemaphore->GetSemaphore())
			.setSwapchains(VulkanContext::GetSwapChain()->GetSwapChain())
			.setImageIndices(imageIndex);

		bool presentResult = false;
		switch (const auto present = VulkanContext::GetPresentQueue().presentKHR(presentInfo))
		{
			case vk::Result::eSuccess:
				presentResult = true;
				break;
			case vk::Result::eSuboptimalKHR:
				*needRebuild = true;
				presentResult = true;
				break;
			case vk::Result::eErrorOutOfDateKHR:
				Logger::LogError("Fail to present, error out of date");
				*needRebuild = true;
				presentResult = false;
				break;
			default:
				Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
				presentResult = false;
				break;
		}

		// Update frame index
		_currentFlightIndex = (_currentFlightIndex + 1) % _parallelFrame;

		return presentResult;
	}

	auto VulkanFlightManager::GetFrameContext() -> FrameContext&
	{
		return _frameContext[_currentFlightIndex];
	}

	uint32_t VulkanFlightManager::GetParallelFramesCount() const
	{
		return _parallelFrame;
	}
} // namespace Ailurus