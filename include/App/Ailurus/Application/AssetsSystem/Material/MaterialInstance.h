#pragma once

#include <cstdint>
#include <string>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"
#include "Ailurus/Application/AssetsSystem/AssetRef.h"
#include "Material.h"
#include "MaterialUniformAccess.h"

namespace Ailurus
{
	class MaterialInstance : public Asset
	{
	public:
		Material* GetTargetMaterial() const;
		void UpdateMaterialUniformData();
		void SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value);
		void SetUniformValue(const MaterialUniformAccess& entry, const UniformValue& value);

	protected:
		friend class AssetsSystem;
		MaterialInstance(uint64_t assetId, const AssetRef<Material>& targetMaterial);

	protected:
		AssetRef<Material> targetMaterial;
		MaterialUniformValueMap uniformValueMap;
	};

} // namespace Ailurus