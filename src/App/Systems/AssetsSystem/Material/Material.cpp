#include <cstdint>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/AssetsSystem/Texture/Texture.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformSet.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	Material::Material(uint64_t assetId)
		: TypedAsset(assetId)
	{
	}

	bool Material::HasRenderPass(RenderPassType pass) const
	{
		return _renderPassInfoMap.contains(pass);
	}

	const StageShaderArray* Material::GetPassShaderArray(RenderPassType pass) const
	{
		const auto itr = _renderPassInfoMap.find(pass);
		if (itr != _renderPassInfoMap.end())
			return &itr->second.shaders;
		return nullptr;
	}

	const UniformSet* Material::GetUniformSet(RenderPassType pass) const
	{
		const auto itr = _renderPassInfoMap.find(pass);
		if (itr != _renderPassInfoMap.end())
			return itr->second.pUniformSet.get();
		return nullptr;
	}

	void Material::SetPassShaderAndUniform(RenderPassType pass, const std::vector<const Shader*>& shaders,
		std::unique_ptr<UniformSet>&& pUniformSet)
	{
		for (const auto* pShader : shaders)
		{
			if (pShader != nullptr)
				_renderPassInfoMap[pass].shaders[pShader->GetStage()] = pShader;
		}

		_renderPassInfoMap[pass].pUniformSet = std::move(pUniformSet);
	}

	void Material::SetPassTexture(RenderPassType pass, const std::string& uniformVarName, const AssetRef<Texture>& texture)
	{
		_renderPassInfoMap[pass].textures.emplace(uniformVarName, texture);
	}

	auto Material::GetTextures(RenderPassType pass) const -> const std::unordered_map<std::string, AssetRef<Texture>>*
	{
		const auto itr = _renderPassInfoMap.find(pass);
		if (itr != _renderPassInfoMap.end())
			return &itr->second.textures;
		return nullptr;
	}
} // namespace Ailurus
