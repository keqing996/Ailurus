#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Utility/Logger.h>
#include <VulkanSystem/Buffer/VulkanUniformBuffer.h>

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

		// Get binding point offset
		const auto bindingOffset = _pTargetUniformSet->GetBindingPointOffsetInUniformBuffer(entry.bindingId);

		// Get access offset
		const auto* pBindingPoint = _pTargetUniformSet->GetBindingPoint(entry.bindingId);
		const auto accessOffset = pBindingPoint->GetAccessOffset(entry.access);
		if (!accessOffset.has_value())
		{
			Logger::LogError("Access '{}' not found in binding point with ID: {}", entry.access, entry.bindingId);
			return;
		}

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

	auto UniformSetMemory::TransitionDataToGpu() const -> void
	{
		if (_pUniformBuffer)
			_pUniformBuffer->TransitionDataToGpu();
	}

	auto UniformSetMemory::GetUniformValueMap() const -> const UniformValueMap&
	{
		return _uniformValueMap;
	}
} // namespace Ailurus