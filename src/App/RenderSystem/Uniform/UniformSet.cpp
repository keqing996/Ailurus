#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <VulkanSystem/Buffer/VulkanUniformBuffer.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	UniformSet::UniformSet(uint32_t setId)
		: _setId(setId)
	{
	}

	UniformSet::~UniformSet() = default;

	void UniformSet::AddBindingPoint(std::unique_ptr<UniformBindingPoint>&& pBindingPoint)
	{
		if (_pUniformBuffer != nullptr)
		{
			Logger::LogError("UniformSet already has a uniform buffer, cannot add more binding points.");
			return;
		}

		const auto id = pBindingPoint->GetBindingPoint();
		if (_bindingPoints.contains(id))
		{
			Logger::LogError("Duplicate binding point id: {} ", id);
			return;
		}

		_bindingPoints[id] = std::move(pBindingPoint);
	}

	void UniformSet::InitUniformBuffer()
	{
		if (_pUniformBuffer != nullptr)
		{
			Logger::LogError("UniformSet already has a uniform buffer initialized.");
			return;
		}

		uint32_t offset = 0;
		for (const auto& [bindingId, pBindingPoint] : _bindingPoints)
		{
			_bindingPointOffsetInUniformBufferMap[bindingId] = offset;
			offset += pBindingPoint->GetTotalSize();
		}

		_pUniformBuffer = std::make_unique<VulkanUniformBuffer>(offset);
	}

	const UniformBindingPoint* UniformSet::GetBindingPoint(uint32_t bindingPoint) const
	{
		auto itr = _bindingPoints.find(bindingPoint);
		if (itr != _bindingPoints.end())
			return itr->second.get();

		return nullptr;
	}

	uint32_t UniformSet::GetSetId() const
	{
		return _setId;
	}
} // namespace Ailurus