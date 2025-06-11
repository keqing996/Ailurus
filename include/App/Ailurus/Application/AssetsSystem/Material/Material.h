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
		struct PerPass 
		{
			StageShaderArray stageShaders;
			UniformSet uniformSet;
		};

	public:
		void SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value);
		
	private:
		PerPass* GetRenderPassParameters(RenderPassType pass);

	private:
		friend class AssetsSystem;
		explicit Material(uint64_t assetId);

	private:
		std::unordered_map<RenderPassType, PerPass> _renderPassParaMap;
	};
} // namespace Ailurus