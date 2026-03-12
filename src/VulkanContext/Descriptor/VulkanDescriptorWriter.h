#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <Ailurus/Utility/NonCopyable.h>

namespace Ailurus
{
	class VulkanDescriptorWriter : public NonCopyable
	{
	public:
		VulkanDescriptorWriter() = default;
		~VulkanDescriptorWriter() = default;

	public:
		/// @brief Write a uniform buffer binding
		/// @param binding Binding point in the descriptor set
		/// @param buffer Vulkan buffer handle
		/// @param offset Offset into the buffer
		/// @param range Size of the buffer region
		auto WriteBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range) -> VulkanDescriptorWriter&;

		/// @brief Write a dynamic uniform buffer binding
		/// @param binding Binding point in the descriptor set
		/// @param buffer Vulkan buffer handle
		/// @param offset Offset into the buffer
		/// @param range Size of the buffer region
		auto WriteDynamicBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range) -> VulkanDescriptorWriter&;

		/// @brief Write a combined image sampler binding
		/// @param binding Binding point in the descriptor set
		/// @param imageView Image view to bind
		/// @param sampler Sampler to use
		/// @param imageLayout Layout of the image
		auto WriteImage(uint32_t binding, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal) -> VulkanDescriptorWriter&;

		/// @brief Write a storage buffer binding
		/// @param binding Binding point in the descriptor set
		/// @param buffer Vulkan buffer handle
		/// @param offset Offset into the buffer
		/// @param range Size of the buffer region
		auto WriteStorageBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range) -> VulkanDescriptorWriter&;

		/// @brief Clear all pending writes
		auto Clear() -> void;

		/// @brief Update a descriptor set with all pending writes
		/// @param descriptorSet The descriptor set to update
		auto UpdateSet(vk::DescriptorSet descriptorSet) -> void;

		/// @brief Get the number of pending writes
		auto GetWriteCount() const -> size_t { return _writes.size(); }

	private:
		std::vector<vk::WriteDescriptorSet> _writes;
		std::vector<vk::DescriptorBufferInfo> _bufferInfos;
		std::vector<vk::DescriptorImageInfo> _imageInfos;
	};

} // namespace Ailurus
