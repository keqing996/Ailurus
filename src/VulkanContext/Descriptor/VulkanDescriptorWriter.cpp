#include "VulkanDescriptorWriter.h"
#include "VulkanContext/VulkanContext.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VulkanDescriptorWriter& VulkanDescriptorWriter::WriteBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
	{
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.setBuffer(buffer)
			.setOffset(offset)
			.setRange(range);

		_bufferInfos.push_back(bufferInfo);

		vk::WriteDescriptorSet write;
		write.setDstBinding(binding)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setBufferInfo(_bufferInfos.back());

		_writes.push_back(write);

		return *this;
	}

	VulkanDescriptorWriter& VulkanDescriptorWriter::WriteDynamicBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
	{
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.setBuffer(buffer)
			.setOffset(offset)
			.setRange(range);

		_bufferInfos.push_back(bufferInfo);

		vk::WriteDescriptorSet write;
		write.setDstBinding(binding)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDescriptorCount(1)
			.setBufferInfo(_bufferInfos.back());

		_writes.push_back(write);

		return *this;
	}

	VulkanDescriptorWriter& VulkanDescriptorWriter::WriteImage(uint32_t binding, vk::ImageView imageView, vk::Sampler sampler, vk::ImageLayout imageLayout)
	{
		vk::DescriptorImageInfo imageInfo;
		imageInfo.setImageView(imageView)
			.setSampler(sampler)
			.setImageLayout(imageLayout);

		_imageInfos.push_back(imageInfo);

		vk::WriteDescriptorSet write;
		write.setDstBinding(binding)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setImageInfo(_imageInfos.back());

		_writes.push_back(write);

		return *this;
	}

	VulkanDescriptorWriter& VulkanDescriptorWriter::WriteStorageBuffer(uint32_t binding, vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
	{
		vk::DescriptorBufferInfo bufferInfo;
		bufferInfo.setBuffer(buffer)
			.setOffset(offset)
			.setRange(range);

		_bufferInfos.push_back(bufferInfo);

		vk::WriteDescriptorSet write;
		write.setDstBinding(binding)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eStorageBuffer)
			.setDescriptorCount(1)
			.setBufferInfo(_bufferInfos.back());

		_writes.push_back(write);

		return *this;
	}

	void VulkanDescriptorWriter::Clear()
	{
		_writes.clear();
		_bufferInfos.clear();
		_imageInfos.clear();
	}

	void VulkanDescriptorWriter::UpdateSet(vk::DescriptorSet descriptorSet)
	{
		if (_writes.empty())
			return;

		// Set the descriptor set for all writes
		for (auto& write : _writes)
			write.setDstSet(descriptorSet);

		try
		{
			VulkanContext::GetDevice().updateDescriptorSets(_writes, nullptr);
		}
		catch (const vk::SystemError& e)
		{
			Logger::LogError("Failed to update descriptor set: {}", e.what());
		}

		// Clear after update
		Clear();
	}

} // namespace Ailurus
