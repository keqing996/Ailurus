#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Utility/Logger.h>
#include <VulkanContext/DataBuffer/VulkanUniformBuffer.h>
#include <VulkanContext/Descriptor/VulkanDescriptorSetLayout.h>

namespace Ailurus
{
	UniformSet::UniformSet(uint32_t setId)
		: _setId(setId)
	{
	}

	UniformSet::UniformSet(UniformSetUsage usage)
		: _setId(static_cast<uint32_t>(usage))
	{
	}

	UniformSet::~UniformSet() = default;

	void UniformSet::AddBindingPoint(std::unique_ptr<UniformBindingPoint>&& pBindingPoint)
	{
		const auto id = pBindingPoint->GetBindingPoint();
		if (_bindingPoints.contains(id))
		{
			Logger::LogError("Duplicate binding point id: {} ", id);
			return;
		}

		_bindingPoints[id] = std::move(pBindingPoint);
	}

	void UniformSet::InitUniformBufferInfo()
	{
		uint32_t offset = 0;
		for (const auto& [bindingId, pBindingPoint] : _bindingPoints)
		{
			_bindingPointOffsetInUniformBufferMap[bindingId] = offset;
			offset += pBindingPoint->GetTotalSize();
		}

		_uniformBufferSize = offset;
	}

	void UniformSet::InitDescriptorSetLayout()
	{
		_pDescriptorSetLayout = std::make_unique<VulkanDescriptorSetLayout>(this);
	}

	const UniformSet::BindingPointMap& UniformSet::GetAllBindingPoints() const
	{
		return _bindingPoints;
	}

	const UniformBindingPoint* UniformSet::GetBindingPoint(uint32_t bindingPoint) const
	{
		const auto itr = _bindingPoints.find(bindingPoint);
		if (itr == _bindingPoints.end())
			return nullptr;

		return itr->second.get();
	}

	uint32_t UniformSet::GetSetId() const
	{
		return _setId;
	}

	VulkanDescriptorSetLayout* UniformSet::GetDescriptorSetLayout() const
	{
		return _pDescriptorSetLayout.get();
	}

	auto UniformSet::GetUniformBufferSize() const -> uint32_t
	{
		return _uniformBufferSize;
	}

	auto UniformSet::GetBindingPointOffsetInUniformBuffer(uint32_t bindingPoint) const -> uint32_t
	{
		const auto itr = _bindingPointOffsetInUniformBufferMap.find(bindingPoint);
		if (itr != _bindingPointOffsetInUniformBufferMap.end())
			return itr->second;
		return 0;
	}
} // namespace Ailurus