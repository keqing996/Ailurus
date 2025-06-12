#include <cstdint>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	Material::Material(uint64_t assetId)
		: Asset(assetId)
	{
	}

	void Material::SetPassShader(RenderPassType pass, const Shader* pShader)
	{
		if (pShader == nullptr)
			return;

		_renderPassShaderMap[pass][pShader->GetStage()] = pShader;
	}

	void Material::UpdateUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		auto itr = _renderPassUniformMap.find(pass);
		if (itr == _renderPassUniformMap.end())
		{
			Logger::LogError("Material render pass not found: {}", EnumReflection<RenderPassType>::ToString(pass));
			return;
		}

		auto& pUniform = itr->second;
		pUniform->UpdateUniformValue(bindingId, access, value);
	}

	void Material::SetUniformSet(RenderPassType pass, std::unique_ptr<UniformSet>&& pUniformSet)
	{
		if (pUniformSet == nullptr)
			return;

		_renderPassUniformMap[pass] = std::move(pUniformSet);
	}
} // namespace Ailurus
