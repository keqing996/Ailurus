#pragma once

#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Math/Matrix4x4.hpp>

namespace Ailurus
{
	class VulkanCommandBuffer;
	class VulkanResource;
	class VulkanDataBuffer;
	class VulkanRenderPass;
	class VulkanPipeline;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;
	class VulkanFrameBuffer;

	class VulkanCommandBuffer : public NonCopyable
	{
	public:
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

	public:
		const vk::CommandBuffer& GetBuffer() const;

		void Begin();
		void End();
		void CopyBuffer(VulkanDataBuffer* pSrcBuffer, VulkanDataBuffer* pDstBuffer, vk::DeviceSize size);
		void BufferMemoryBarrier(VulkanDataBuffer* pBuffer, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask);
		void BeginRenderPass(VulkanRenderPass* pRenderPass, VulkanFrameBuffer* pTargetFrameBuffer);
		void EndRenderPass();
		void SetViewportAndScissor();
		void BindPipeline(const VulkanPipeline* pPipeline);
		void BindVertexBuffer(const VulkanVertexBuffer* pVertexBuffer);
		void BindIndexBuffer(const VulkanIndexBuffer* pIndexBuffer);
		void BindDescriptorSet(vk::PipelineLayout layout, const std::vector<vk::DescriptorSet>& descriptorSets);
		void DrawIndexed(uint32_t indexCount);
		void DrawNonIndexed(uint32_t vertexCount);
		void PushConstantModelMaterix(const VulkanPipeline* pPipeline, const Matrix4x4f& mvpMatrix);

	private:
		vk::CommandBuffer _buffer;
		std::unordered_set<VulkanResource*> _referencedResources;
	};
} // namespace Ailurus