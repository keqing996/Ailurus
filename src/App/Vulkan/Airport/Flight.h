#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class RenderPass;

	struct Flight
	{
	public:
		uint32_t imageIndex;
		vk::CommandBuffer commandBuffer;
		vk::Semaphore imageReadySemaphore;
		vk::Semaphore renderFinishSemaphore;
		vk::Fence fence;

	public:
		void Begin();
		void End();

		void BeginRenderPass(RenderPass* pRenderPass);
		void EndRenderPass();

		void DrawObject(const RenderObject* pRenderObject);

	private:
		RenderPass* _pCurrentRenderPass = nullptr;
	};
} // namespace Ailurus