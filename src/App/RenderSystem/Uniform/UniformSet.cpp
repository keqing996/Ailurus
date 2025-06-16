#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Utility/Logger.h>
#include <VulkanSystem/Buffer/VulkanUniformBuffer.h>
#include <VulkanSystem/Descriptor/VulkanDescriptorSetLayout.h>

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

	UniformBindingPoint* UniformSet::GetBindingPoint(uint32_t bindingPoint)
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

	void UniformSet::UpdateUniformValue(uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		if (_pUniformBuffer == nullptr)
		{
			Logger::LogError("Uniform buffer not initialized for UniformSet with ID: {}", _setId);
			return;
		}

		auto* bindingPoint = GetBindingPoint(bindingId);
		if (bindingPoint == nullptr)
		{
			Logger::LogError("Uniform binding point not found for binding ID: {}", bindingId);
			return;
		}

		auto accessOffset = bindingPoint->GetAccessOffset(access);
		if (!accessOffset.has_value())
		{
			Logger::LogError("Access '{}' not found in binding point with ID: {}", access, bindingId);
			return;
		}

		auto itr = _bindingPointOffsetInUniformBufferMap.find(bindingId);
		if (itr == _bindingPointOffsetInUniformBufferMap.end())
		{
			Logger::LogError("Binding point offset not found for binding ID: {}", bindingId);
			return;
		}

		uint32_t bindingPointOffset = itr->second;
		uint32_t offset = bindingPointOffset + *accessOffset;
		auto pBeginPos = _pUniformBuffer->GetWriteBeginPos();

		auto visitor = [pBeginPos]<typename T>(T&& arg)
		{
			using T = std::decay_t<T>;
			std::memcpy(static_cast<void*>(pBeginPos), &arg, sizeof(T));
		};
		
		std::visit(visitor, value);
	}

	VulkanDescriptorSetLayout* UniformSet::GetDescriptorSetLayout() const
	{
		return _pDescriptorSetLayout.get();
	}
} // namespace Ailurus