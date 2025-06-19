#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialUniformAccess.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include "VulkanSystem/Buffer/VulkanUniformBuffer.h"

namespace Ailurus
{
	MaterialInstance::RenderPassUniformBufferOffsetInfo::RenderPassUniformBufferOffsetInfo(uint32_t offset, uint32_t size)
		: offset(offset)
		, size(size)
	{
	}

	MaterialInstance::MaterialInstance(uint64_t assetId, const AssetRef<Material>& targetMaterial)
		: TypedAsset(assetId)
		, _targetMaterial(targetMaterial)
	{
		const auto* pMaterial = targetMaterial.Get();

		uint32_t offset = 0;
		for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
		{
			auto pass = static_cast<RenderPassType>(i);
			if (!pMaterial->HasRenderPass(pass))
				continue;

			const auto* pUniformSet = pMaterial->GetUniformSet(pass);
			const auto renderPassUniformBufferSize = pUniformSet->GetUniformBufferSize();

			// Record current
			_renderPassUniformBufferOffsetMap[pass] = { offset, renderPassUniformBufferSize };
			offset += renderPassUniformBufferSize;
		}

		_pUniformBuffer = std::make_unique<VulkanUniformBuffer>(offset);
	}

	MaterialInstance::~MaterialInstance()
	{
	}

	Material* MaterialInstance::GetTargetMaterial() const
	{
		return _targetMaterial.Get();
	}

	void MaterialInstance::SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		MaterialUniformAccess entry{ pass, bindingId, access };
		SetUniformValue(entry, value);
	}

	void MaterialInstance::SetUniformValue(const MaterialUniformAccess& entry, const UniformValue& value)
	{
		// Check uniform buffer initialization
		if (_pUniformBuffer == nullptr)
		{
			Logger::LogError("Uniform buffer not initialized for UniformSet");
			return;
		}

		// Get render pass offset and range
		auto renderPassInfoOpt = GetRenderPassUniformBufferOffset(entry.pass);
		if (!renderPassInfoOpt.has_value())
		{
			Logger::LogError("Render pass uniform buffer offset not found for pass: {}", EnumReflection<RenderPassType>::ToString(entry.pass));
			return;
		}

		auto [renderPassOffset, renderPassSize] = renderPassInfoOpt.value();

		// Get uniform set
		const auto* pUniformSet = _targetMaterial.Get()->GetUniformSet(entry.pass);
		if (pUniformSet == nullptr)
		{
			Logger::LogError("Uniform set not found for render pass: {}", EnumReflection<RenderPassType>::ToString(entry.pass));
			return;
		}

		// Get binding point offset
		const auto bindingOffset = pUniformSet->GetBindingPointOffsetInUniformBuffer(entry.bindingId);

		// Get access offset
		const auto* pBindingPoint = pUniformSet->GetBindingPoint(entry.bindingId);
		const auto accessOffset = pBindingPoint->GetAccessOffset(entry.access);
		if (!accessOffset.has_value())
		{
			Logger::LogError("Access '{}' not found in binding point with ID: {}", entry.access, entry.bindingId);
			return;
		}

		// Check if access offset is valid
		if (bindingOffset + *accessOffset >= renderPassSize)
		{
			Logger::LogError("Access offset out of range for binding ID: {}, access: {}, render pass size: {}, binding offset: {}, access offset: {}",
				entry.bindingId, entry.access, renderPassSize, bindingOffset, *accessOffset);
			return;
		}

		// Write value to uniform buffer
		uint32_t offset = renderPassOffset + bindingOffset + *accessOffset;
		auto pBeginPos = _pUniformBuffer->GetWriteBeginPos() + offset;
		
		auto visitor = [pBeginPos](auto&& arg) 
		{
			using T = std::decay_t<decltype(arg)>;
			std::memcpy(static_cast<void*>(pBeginPos), &arg, sizeof(T));
		};

		std::visit(visitor, value);

		// Record uniform value
		_uniformValueMap[entry] = value;
	}

	VulkanUniformBuffer* MaterialInstance::GetUniformBuffer() const
	{
		return _pUniformBuffer.get();
	}

	auto MaterialInstance::GetRenderPassUniformBufferOffset(RenderPassType pass) const -> std::optional<RenderPassUniformBufferOffsetInfo>
	{
		const auto itr = _renderPassUniformBufferOffsetMap.find(pass);
		if (itr != _renderPassUniformBufferOffsetMap.end())
			return itr->second;

		return std::nullopt;
	}

	void MaterialInstance::UpdateAllUniformValuesToUniformBuffer()
	{
		for (const auto& [entry, value] : _uniformValueMap)
			SetUniformValue(entry, value);
	}
} // namespace Ailurus