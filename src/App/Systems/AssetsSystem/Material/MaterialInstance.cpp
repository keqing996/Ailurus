#include <Ailurus/Utility/EnumReflection.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSetMemory.h>
#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>

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

		for (auto i = 0; i < EnumReflection<RenderPassType>::Size(); i++)
		{
			auto pass = static_cast<RenderPassType>(i);
			if (!pMaterial->HasRenderPass(pass))
				continue;

			const auto* pUniformSet = pMaterial->GetUniformSet(pass);
			if (pUniformSet == nullptr)
				continue; // Pass has no material uniform set (e.g., shadow pass)

			// Record current
			_renderPassUniformBufferMap.insert({ 
				pass, 
				std::make_unique<UniformSetMemory>(pUniformSet) });
		}
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
		UniformAccess entry{ bindingId, access };
		SetUniformValue(pass, entry, value);
	}

	void MaterialInstance::SetUniformValue(RenderPassType pass, const UniformAccess& entry, const UniformValue& value)
	{
		auto itr = _renderPassUniformBufferMap.find(pass);
		if (itr == _renderPassUniformBufferMap.end())
		{
			Logger::LogError("Render pass {} not found in material instance", EnumReflection<RenderPassType>::ToString(pass));
			return;
		}

		auto& pUniformSetMemory = itr->second;
		pUniformSetMemory->SetUniformValue(entry, value);
	}

	UniformSetMemory* MaterialInstance::GetUniformSetMemory(RenderPassType pass) const
	{
		auto itr = _renderPassUniformBufferMap.find(pass);
		if (itr != _renderPassUniformBufferMap.end())
			return itr->second.get();

		return nullptr;
	}

	auto MaterialInstance::GetTextures(RenderPassType pass) const -> const std::unordered_map<std::string, AssetRef<Texture>>*
	{
		const auto* pMaterial = _targetMaterial.Get();
		if (pMaterial == nullptr)
			return nullptr;

		return pMaterial->GetTextures(pass);
	}
} // namespace Ailurus