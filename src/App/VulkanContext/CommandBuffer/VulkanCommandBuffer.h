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
	class VulkanPipeline;
	class VulkanVertexBuffer;
	class VulkanIndexBuffer;

	class VulkanCommandBuffer : public NonCopyable
	{
	public:
		/// @brief Construct a command buffer
		/// @param isPrimary True for primary command buffer, false for secondary
		explicit VulkanCommandBuffer(bool isPrimary);
		virtual ~VulkanCommandBuffer();

	public:
		/// @brief Get the underlying Vulkan command buffer handle
		/// @return Reference to the Vulkan command buffer
		const vk::CommandBuffer& GetBuffer() const;
		
		/// @brief Check if the command buffer is currently recording
		/// @return True if recording, false otherwise
		bool IsRecording() const;
		
		/// @brief Clear all resource references tracked by this command buffer
		void ClearResourceReferences();

		/// @brief Begin recording commands into this command buffer
		void Begin();
		
		/// @brief End recording commands
		void End();
		
		/// @brief Copy data from source buffer to destination buffer
		/// @param pSrcBuffer Source buffer to copy from
		/// @param pDstBuffer Destination buffer to copy to
		/// @param size Size in bytes to copy
		void CopyBuffer(VulkanDataBuffer* pSrcBuffer, VulkanDataBuffer* pDstBuffer, vk::DeviceSize size);
		
		/// @brief Insert a buffer memory barrier for synchronization
		/// @param pBuffer Buffer to apply barrier to
		/// @param srcAccessMask Source access mask
		/// @param dstAccessMask Destination access mask
		/// @param srcStageMask Source pipeline stage mask
		/// @param dstStageMask Destination pipeline stage mask
		void BufferMemoryBarrier(VulkanDataBuffer* pBuffer, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask);
		
		/// @brief Insert an image memory barrier for layout transition and synchronization
		/// @param image Image to apply barrier to
		/// @param oldLayout Current image layout
		/// @param newLayout Target image layout
		/// @param srcAccessMask Source access mask
		/// @param dstAccessMask Destination access mask
		/// @param srcStageMask Source pipeline stage mask
		/// @param dstStageMask Destination pipeline stage mask
		void ImageMemoryBarrier(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask);
		
		/// @brief Begin dynamic rendering (VK_KHR_dynamic_rendering)
		/// @param colorImageView Color attachment image view (MSAA or swapchain image)
		/// @param depthImageView Depth attachment image view (can be null if useDepth is false)
		/// @param resolveImageView Resolve target image view for MSAA (can be null if no MSAA)
		/// @param extent Rendering area extent
		/// @param clearColor If true, clear color attachment; if false, load existing content
		/// @param useDepth If true, use depth attachment; if false, render without depth
		void BeginRendering(vk::ImageView colorImageView, vk::ImageView depthImageView, vk::ImageView resolveImageView, vk::Extent2D extent, bool clearColor = true, bool useDepth = true);
		
		/// @brief End dynamic rendering
		void EndRendering();
		
		/// @brief Set viewport and scissor to match swap chain extent
		void SetViewportAndScissor();
		
		/// @brief Bind a graphics pipeline
		/// @param pPipeline Pipeline to bind
		void BindPipeline(const VulkanPipeline* pPipeline);
		
		/// @brief Bind a vertex buffer
		/// @param pVertexBuffer Vertex buffer to bind
		void BindVertexBuffer(const VulkanVertexBuffer* pVertexBuffer);
		
		/// @brief Bind an index buffer
		/// @param pIndexBuffer Index buffer to bind
		void BindIndexBuffer(const VulkanIndexBuffer* pIndexBuffer);
		
		/// @brief Bind descriptor sets
		/// @param layout Pipeline layout
		/// @param descriptorSets Array of descriptor sets to bind
		void BindDescriptorSet(vk::PipelineLayout layout, const std::vector<vk::DescriptorSet>& descriptorSets);
		
		/// @brief Execute an indexed draw call
		/// @param indexCount Number of indices to draw
		void DrawIndexed(uint32_t indexCount);
		
		/// @brief Execute a non-indexed draw call
		/// @param vertexCount Number of vertices to draw
		void DrawNonIndexed(uint32_t vertexCount);
		
		/// @brief Push model matrix constant to shader
		/// @param pPipeline Pipeline containing push constant layout
		/// @param mvpMatrix Model matrix to push
		void PushConstantModelMatrix(const VulkanPipeline* pPipeline, const Matrix4x4f& mvpMatrix);
		
		/// @brief Execute a secondary command buffer
		/// @param pSecondaryCommandBuffer Secondary command buffer to execute
		void ExecuteSecondaryCommandBuffer(const VulkanCommandBuffer* pSecondaryCommandBuffer);

	protected:
		vk::CommandBuffer _buffer;
		bool _isPrimary;
		bool _isRecording;
		std::unordered_set<VulkanResource*> _referencedResources;
	};
} // namespace Ailurus