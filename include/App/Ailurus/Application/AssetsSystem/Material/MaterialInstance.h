#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include "Ailurus/Application/RenderSystem/Uniform/UniformAccess.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/AssetsSystem/AssetRef.h"
#include "Material.h"

namespace Ailurus
{
	class UniformSetMemory;

	class MaterialInstance : public TypedAsset<AssetType::MaterialInstance>
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
		auto SetUniformValue(RenderPassType pass, const UniformAccess& entry, const UniformValue& value) -> void;
		auto GetUniformSetMemory(RenderPassType pass) const -> UniformSetMemory*;
		auto GetRenderPassUniformBufferOffset(RenderPassType pass) const -> std::optional<RenderPassUniformBufferOffsetInfo>;

	private:
		// Target material
		AssetRef<Material> _targetMaterial;

		// Uniform buffer for all render passes and all uniform sets, binding points
		std::unordered_map<RenderPassType, std::unique_ptr<UniformSetMemory>> _renderPassUniformBufferMap;
	};

} // namespace Ailurus