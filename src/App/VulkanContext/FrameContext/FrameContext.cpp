#include <optional>
#include "FrameContext.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanSystem/Resource/VulkanResourceManager.h"
#include "VulkanSystem/CommandBuffer/VulkanCommandBuffer.h"
#include "VulkanSystem/Descriptor/VulkanDescriptorPool.h"

namespace Ailurus
{
	FrameContext::~FrameContext()
	{
		// Recycle fling resources
		WaitFinish();

		// Recycle recording resources
		if (_pRecordingCommandBuffer != nullptr)
		{
			_pRecordingCommandBuffer.reset();
		}

		// Reset and free descriptor pool
		if (_pAllocatingDescriptorPool != nullptr)
		{
			_pAllocatingDescriptorPool->ResetPool();
			Application::Get<VulkanSystem>()->FreeDescriptorPool(std::move(_pAllocatingDescriptorPool));
		}
	}

	void FrameContext::EnsureFrameInitialized()
	{
		if (_pRecordingCommandBuffer == nullptr)
		{
			_pRecordingCommandBuffer = std::make_unique<VulkanCommandBuffer>();
			_pRecordingCommandBuffer->Begin();
		}

		if (_pAllocatingDescriptorPool == nullptr)
		{
			_pAllocatingDescriptorPool = std::move(Application::Get<VulkanSystem>()->AllocateDescriptorPool());
		}
	}

	bool FrameContext::WaitFinish()
	{
		if (!_renderingFrameContext.has_value())
			return true;

		const auto device = Application::Get<VulkanSystem>()->GetDevice();
		const auto waitFence = device.waitForFences(_renderingFrameContext->allFinishFence,
			true, std::numeric_limits<uint64_t>::max());
		if (waitFence != vk::Result::eSuccess)
		{
			Logger::LogError("Fail to wait fences, result = {}", static_cast<int>(waitFence));
			return false;
		}

		// Free finished command buffer
		_renderingFrameContext->pRenderingCommandBuffer.reset();

		// Free used semaphores
		Application::Get<VulkanSystem>()->FreeSemaphore(_renderingFrameContext->waitSemaphore);
		Application::Get<VulkanSystem>()->FreeSemaphore(_renderingFrameContext->signalSemaphore);

		// Reset fence and free
		device.resetFences(_renderingFrameContext->allFinishFence);
		Application::Get<VulkanSystem>()->FreeFence(_renderingFrameContext->allFinishFence);

		// Reset and free descriptor pool
		_renderingFrameContext->pFrameDescriptorPool->ResetPool();
		Application::Get<VulkanSystem>()->FreeDescriptorPool(std::move(_renderingFrameContext->pFrameDescriptorPool));

		// Clear render context
		_renderingFrameContext = std::nullopt;

		// Trigger resource collect
		Application::Get<VulkanSystem>()->GetResourceManager()->GarbageCollect();

		return true;
	}

	VulkanCommandBuffer* FrameContext::GetRecordingCommandBuffer() const
	{
		return _pRecordingCommandBuffer.get();
	}

	VulkanDescriptorPool* FrameContext::GetAllocatingDescriptorPool() const
	{
		return _pAllocatingDescriptorPool.get();
	}

	vk::Semaphore FrameContext::SubmitCommandBuffer(vk::Semaphore imageReadySemaphore)
	{
		// Record end
		_pRecordingCommandBuffer->End();

		// Create a fence and semaphore
		const vk::Fence renderFinishFence = Application::Get<VulkanSystem>()->AllocateFence();
		const vk::Semaphore signalSemaphore = Application::Get<VulkanSystem>()->AllocateSemaphore();

		// Do submit
		vk::SubmitInfo submitInfo;
		submitInfo.setCommandBuffers(_pRecordingCommandBuffer->GetBuffer())
			.setSignalSemaphores(signalSemaphore)
			.setWaitSemaphores(imageReadySemaphore);

		// Wait stages
		std::vector<vk::PipelineStageFlags> waitStages { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitDstStageMask(waitStages);

		// Submit
		Application::Get<VulkanSystem>()->GetGraphicQueue().submit(submitInfo, renderFinishFence);

		// Collect resources
		_renderingFrameContext = RenderingFrameContext{};
		{
			_renderingFrameContext->renderingFrameCount = Application::Get<TimeSystem>()->FrameCount();
			_renderingFrameContext->pRenderingCommandBuffer = std::move(_pRecordingCommandBuffer);
			_renderingFrameContext->pFrameDescriptorPool = std::move(_pAllocatingDescriptorPool);
			_renderingFrameContext->waitSemaphore = imageReadySemaphore;
			_renderingFrameContext->signalSemaphore = signalSemaphore;
			_renderingFrameContext->allFinishFence = renderFinishFence;
		}

		// New frame resource
		EnsureFrameInitialized();

		return signalSemaphore;
	}
} // namespace Ailurus