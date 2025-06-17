#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"
#include "Ailurus/Application/AssetsSystem/AssetRef.h"
#include "Material.h"
#include "MaterialUniformAccess.h"

namespace Ailurus
{
	class MaterialInstance : public Asset
	{
		friend class AssetsSystem;
		MaterialInstance(uint64_t assetId, const AssetRef<Material>& targetMaterial);

	public:
		struct RenderPassUniformBufferOffsetInfo
		{
			uint32_t offset;
			uint32_t size;

			RenderPassUniformBufferOffsetInfo(uint32_t offset, uint32_t size);
		};

		~MaterialInstance();

	public:
		auto GetTargetMaterial() const -> Material*;
		auto SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value) -> void;
		auto SetUniformValue(const MaterialUniformAccess& entry, const UniformValue& value) -> void;
		auto GetUniformBuffer() const -> VulkanUniformBuffer*;
		auto GetRenderPassUniformBufferOffset(RenderPassType pass) const -> std::optional<RenderPassUniformBufferOffsetInfo>;

	public:
		auto UpdateAllUniformValuesToUniformBuffer() -> void;

	private:
		// Target material
		AssetRef<Material> _targetMaterial;

		// Uniform access -> uniform value
		MaterialUniformValueMap _uniformValueMap;

		// Uniform buffer for all render passes and all uniform sets, binding points
		std::unique_ptr<VulkanUniformBuffer> _pUniformBuffer;

		// The offset and size of the uniform buffer for each render pass
		std::unordered_map<RenderPassType, RenderPassUniformBufferOffsetInfo> _renderPassUniformBufferOffsetMap;
	};

} // namespace Ailurus