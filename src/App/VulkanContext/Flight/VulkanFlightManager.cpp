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

		// Recycle recording resources
		if (_pRecordingCommandBuffer != nullptr)
		{
			_pRecordingCommandBuffer.reset();
		}

		// Reset and free descriptor pool
		if (_pAllocatingDescriptorPool != nullptr)
		{
			_pAllocatingDescriptorPool->ResetPool();
			VulkanContext::GetResourceManager()->FreeDescriptorAllocator(
				std::move(_pAllocatingDescriptorPool));
		}
	}

	void VulkanFlightManager::EnsurePreparation()
	{
		if (_pRecordingCommandBuffer == nullptr)
		{
			_pRecordingCommandBuffer = std::make_unique<VulkanCommandBuffer>();
			_pRecordingCommandBuffer->Begin();
		}

		if (_pAllocatingDescriptorPool == nullptr)
		{
			_pAllocatingDescriptorPool = std::move(VulkanContext::GetResourceManager()->AllocateDescriptorAllocator());
		}
	}

	bool VulkanFlightManager::WaitOneFlight(uint32_t index)
	{
		auto& context = _frameContext[index];

		// Not in flight
		if (!context.onAirInfo.has_value())
			return true;

		const auto waitFence = VulkanContext::GetDevice().waitForFences(context.renderFinishFence,
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
		vkResMgr->FreeSemaphore(onAirFrame.imageReadySemaphore);
		vkResMgr->FreeSemaphore(onAirFrame.renderFinishSemaphore);

		// Reset fence and free
		VulkanContext::GetDevice().resetFences(onAirFrame.renderFinishFence);
		vkResMgr->FreeFence(onAirFrame.renderFinishFence);

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
		EnsurePreparation();

		// Record end
		_pRecordingCommandBuffer->End();

		// Create a fence and semaphore
		auto vkResMgr = VulkanContext::GetResourceManager();
		const vk::Fence renderFinishFence = vkResMgr->AllocateFence();
		const vk::Semaphore renderFinishSemaphore = vkResMgr->AllocateSemaphore();

		// Do submit
		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(_pRecordingCommandBuffer->GetBuffer())
			.setSignalSemaphores(renderFinishSemaphore)
			.setWaitSemaphores(imageReadySemaphore);

		// Wait stages
		std::vector<vk::PipelineStageFlags> waitStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitDstStageMask(waitStages);

		// Submit
		VulkanContext::GetGraphicQueue().submit(submitInfo, renderFinishFence);
		_onAirFrames.push_back(OnAirFrame {
			.frameCount = Application::Get<TimeSystem>()->FrameCount(),
			.pRenderingCommandBuffer = std::move(_pRecordingCommandBuffer),
			.pFrameDescriptorAllocator = std::move(_pAllocatingDescriptorPool),
			.imageReadySemaphore = imageReadySemaphore,
			.renderFinishSemaphore = renderFinishSemaphore,
			.renderFinishFence = renderFinishFence,
		});

		// New frame resource
		EnsurePreparation();

		// Present
		vk::PresentInfoKHR presentInfo;
		presentInfo.setWaitSemaphores(renderFinishSemaphore)
			.setSwapchains(VulkanContext::GetSwapChain()->GetSwapChain())
			.setImageIndices(imageIndex);

		switch (const auto present = VulkanContext::GetPresentQueue().presentKHR(presentInfo))
		{
			case vk::Result::eSuccess:
				return true;
			case vk::Result::eSuboptimalKHR:
				*needRebuild = true;
				return true;
			case vk::Result::eErrorOutOfDateKHR:
				Logger::LogError("Fail to present, error out of date");
				*needRebuild = true;
				return false;
			default:
				Logger::LogError("Fail to present, result = {}", static_cast<int>(present));
				return false;
		}
	}

	VulkanCommandBuffer* VulkanFlightManager::GetRecordingCommandBuffer()
	{
		EnsurePreparation();
		return _pRecordingCommandBuffer.get();
	}

	VulkanDescriptorAllocator* VulkanFlightManager::GetAllocatingDescriptorPool()
	{
		EnsurePreparation();
		return _pAllocatingDescriptorPool.get();
	}

	uint32_t VulkanFlightManager::GetParallelFramesCount() const
	{
		return _parallelFrame;
	}
} // namespace Ailurus