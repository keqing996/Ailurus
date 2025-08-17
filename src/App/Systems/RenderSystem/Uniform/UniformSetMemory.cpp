#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <VulkanContext/VulkanContext.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Resource/VulkanBuffer.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSet.h>

namespace Ailurus
{
    UniformSetMemory::UniformSetMemory(const UniformSet* pTargetUniformSet)
		: _pTargetUniformSet(pTargetUniformSet)
	{
		auto uniformBufferSize = pTargetUniformSet->GetUniformBufferSize();
		_pUniformBuffer = std::make_unique<VulkanUniformBuffer>(uniformBufferSize);
	}

	UniformSetMemory::~UniformSetMemory()
	{
	}

	void UniformSetMemory::SetUniformValue(uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		UniformAccess entry{ bindingId, access };
		SetUniformValue(entry, value);
	}

	void UniformSetMemory::SetUniformValue(const UniformAccess& entry, const UniformValue& value)
	{
		// Check uniform buffer initialization
		if (_pUniformBuffer == nullptr)
		{
			Logger::LogError("Uniform buffer not initialized for UniformSet");
			return;
		}

		// Get access offset
		const auto* pBindingPoint = _pTargetUniformSet->GetBindingPoint(entry.bindingId);
    	if (pBindingPoint == nullptr)
    	{
    		Logger::LogError("Uniform does not have binding point {}", entry.bindingId);
    		return;
    	}

		const auto accessOffset = pBindingPoint->GetAccessOffset(entry.access);
		if (!accessOffset.has_value())
		{
			Logger::LogError("Access '{}' not found in binding point with ID: {}", entry.access, entry.bindingId);
			return;
		}

    	// Get binding point offset
    	const auto bindingOffset = _pTargetUniformSet->GetBindingPointOffsetInUniformBuffer(entry.bindingId);

		// Check if access offset is valid
		uint32_t offset = bindingOffset + *accessOffset;
		if (offset >= _pUniformBuffer->GetBufferSize())
		{
			Logger::LogError("Access offset out of range for binding ID: {}, access: {}, uniform buffer size: {}, binding offset: {}, access offset: {}",
				entry.bindingId, entry.access, _pUniformBuffer->GetBufferSize(), bindingOffset, *accessOffset);
			return;
		}

		// Write value to uniform buffer
		_pUniformBuffer->WriteData(offset, value);
		
		// Record uniform value
		_uniformValueMap[entry] = value;
	}

	void UniformSetMemory::UpdateToDescriptorSet(VulkanCommandBuffer* pCommandBuffer, const VulkanDescriptorSet& descriptorSet) const
	{
		TransitionDataToGpu(pCommandBuffer);

		std::vector<vk::WriteDescriptorSet> writeDescriptorSets;

		auto& allBindingPoints = _pTargetUniformSet->GetAllBindingPoints();
		for (auto& [bindingId, pBindingPoint] : allBindingPoints)
		{
			auto offset = _pTargetUniformSet->GetBindingPointOffsetInUniformBuffer(bindingId);
			auto range = pBindingPoint->GetTotalSize();

			vk::DescriptorBufferInfo descriptorBufferInfo;
			descriptorBufferInfo.setBuffer(_pUniformBuffer->GetThisFrameDeviceBuffer()->buffer)
				.setOffset(offset)
				.setRange(range);

			vk::WriteDescriptorSet writeDescriptorSet;
			writeDescriptorSet.setDstSet(descriptorSet.descriptorSet)
				.setDstBinding(bindingId)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setBufferInfo(descriptorBufferInfo);

			writeDescriptorSets.push_back(writeDescriptorSet);
		}

		VulkanContext::GetDevice().updateDescriptorSets(writeDescriptorSets, {});
	}

	auto UniformSetMemory::TransitionDataToGpu(VulkanCommandBuffer* pCommandBuffer) const -> void
	{
		if (_pUniformBuffer)
			_pUniformBuffer->TransitionDataToGpu(pCommandBuffer);
	}

	auto UniformSetMemory::GetUniformValueMap() const -> const UniformValueMap&
	{
		return _uniformValueMap;
	}
} // namespace Ailurus