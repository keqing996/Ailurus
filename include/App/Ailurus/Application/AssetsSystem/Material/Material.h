#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "Ailurus/Application/AssetsSystem/Asset.h"
#include "Ailurus/Application/AssetsSystem/AssetReference.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformSet.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class Shader;
	class VulkanUniformBuffer;

	class Material : public Asset
	{
		struct MaterialSinglePass
		{
			StageShaderArray stageShaders;
			UniformSet uniformSet;;
			std::unique_ptr<VulkanUniformBuffer> pUniformBuffer;
		};

	public:
		const MaterialSinglePass* GetRenderPassParameters(RenderPassType pass) const;
		

	private:
		friend class AssetsSystem;
		std::unordered_map<RenderPassType, MaterialSinglePass> _renderPassParaMap;
	};
} // namespace Ailurus