#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Ailurus/Application/AssetsSystem/Asset.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "Ailurus/Application/AssetsSystem/AssetRef.h"
#include "Ailurus/Application/AssetsSystem/Texture/Texture.h"

namespace Ailurus
{
	class Material : public TypedAsset<AssetType::Material> 
	{
		struct MaterialRenderPassInfo
		{
			StageShaderArray shaders;
			std::unique_ptr<UniformSet> pUniformSet;
			std::unordered_map<std::string, AssetRef<Texture>> textures; // uniform var name -> texture
		};

	public:
		bool HasRenderPass(RenderPassType pass) const;
		auto GetPassShaderArray(RenderPassType pass) const -> const StageShaderArray*;
		auto GetUniformSet(RenderPassType pass) const -> const UniformSet*;
		auto GetTextures(RenderPassType pass) const -> const std::unordered_map<std::string, AssetRef<Texture>>*;

	private:
		friend class AssetsSystem;
		explicit Material(uint64_t assetId);
		void SetPassShaderAndUniform(RenderPassType pass, const std::vector<const Shader*>& shaders,
			std::unique_ptr<UniformSet>&& pUniformSet);
		void SetPassTexture(RenderPassType pass, const std::string& uniformVarName, const AssetRef<Texture>& texture);

	private:
		std::unordered_map<RenderPassType, MaterialRenderPassInfo> _renderPassInfoMap;
	};
} // namespace Ailurus