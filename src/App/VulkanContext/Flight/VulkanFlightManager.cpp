#include "Ailurus/Utility/Logger.h"
#include "VulkanFlightManager.h"
#include "VulkanContext/VulkanContext.h"
#include "VulkanContext/SwapChain/VulkanSwapChain.h"
#include "VulkanContext/Resource/VulkanResourceManager.h"
#include "VulkanContext/Descriptor/VulkanDescriptorAllocator.h"

namespace Ailurus
{
	VulkanFlightManager::VulkanFlightManager(uint32_t parallelFrames)
		: _parallelFrame(parallelFrames)
	{
		// Create frame context
		_frameContexts.clear();
		for (auto i = 0; i < _parallelFrame; i++)
		{
			_frameContexts.push_back(std::make_unique<FrameContext>());
			_frameContexts[i]->EnsureFrameInitialized();
		}
	}

	VulkanFlightManager::~VulkanFlightManager()
	{
		WaitAllFrameFinish();
	}

    bool VulkanFlightManager::WaitCurrentFrameFinish()
	{
		return GetFrameContext()->WaitFinish();
	}

	bool VulkanFlightManager::WaitOneFlight()
	{
		if (_onAirFrames.empty())
			return true;

		auto onAirFrame = std::move(_onAirFrames.front());
		_onAirFrames.pop_front();

		const auto waitFence = VulkanContext::GetDevice().waitForFences(onAirFrame.allFinishFence,
			true, std::numeric_limits<uint64_t>::max());
		if (waitFence != vk::Result::eSuccess)
		{
			Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
			_onAirFrames.push_front(std::move(onAirFrame));
			return false;
		}

		auto vkResMgr = VulkanContext::GetResourceManager();

		// Free finished command buffer
		onAirFrame.pRenderingCommandBuffer.reset();

		// Free used semaphores
		vkResMgr->FreeSemaphore(onAirFrame.waitSemaphore);
		vkResMgr->FreeSemaphore(onAirFrame.signalSemaphore);

		// Reset fence and free
		VulkanContext::GetDevice().resetFences(onAirFrame.allFinishFence);
		vkResMgr->FreeFence(onAirFrame.allFinishFence);

		// Reset and free descriptor pool
		onAirFrame.pFrameDescriptorAllocator->ResetPool();
		vkResMgr->FreeDescriptorAllocator(std::move(onAirFrame.pFrameDescriptorAllocator));

		// Trigger resource collect
		vkResMgr->GarbageCollect();

		return true;
	}

    void VulkanFlightManager::WaitAllFlight()
	{
		while (!_onAirFrames.empty())
		{
			if (!WaitOneFlight())
			{
				Logger::LogError("Fail to wait flight.");
				break;
			}
		}
	}

	bool VulkanFlightManager::TakeOffFlight(uint32_t imageIndex, vk::Semaphore imageReadySemaphore, bool* needRebuild)
	{
		auto renderFinishSemaphore = GetFrameContext()->SubmitCommandBuffer(imageReadySemaphore);

		// Present
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(renderFinishSemaphore)
			.setSwapchains(VulkanContext::GetSwapChain()->GetSwapChain())
			.setImageIndices(imageIndex);

		switch (const auto present = VulkanContext::GetPresentQueue().presentKHR(presentInfo))
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
		_currentParallelFrameIndex = (_currentParallelFrameIndex + 1) % _parallelFrame;

		return true;
	}

	FrameContext* VulkanFlightManager::GetFrameContext()
	{
		return _frameContexts[_currentParallelFrameIndex].get();
	}
} // namespace Ailurus