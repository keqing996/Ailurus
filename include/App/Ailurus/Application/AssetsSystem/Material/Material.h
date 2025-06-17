#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Ailurus/Application/AssetsSystem/Asset.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"

namespace Ailurus
{
	class Material : public Asset
	{
		struct MaterialRenderPassInfo
		{
			StageShaderArray shaders;
			std::unique_ptr<UniformSet> pUniformSet;
		};

	public:
		bool HasRenderPass(RenderPassType pass) const;
		auto GetPassShaderArray(RenderPassType pass) const -> const StageShaderArray*;
		auto GetUniformSet(RenderPassType pass) const -> const UniformSet*;

	private:
		friend class AssetsSystem;
		explicit Material(uint64_t assetId);
		void SetPassShaderAndUniform(RenderPassType pass, const std::vector<const Shader*>& shaders,
			std::unique_ptr<UniformSet>&& pUniformSet);

	private:
		std::unordered_map<RenderPassType, MaterialRenderPassInfo> _renderPassInfoMap;
	};
} // namespace Ailurus