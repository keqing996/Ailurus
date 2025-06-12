#pragma once

#include <cstdint>
#include <string>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"
#include "Ailurus/Application/AssetsSystem/AssetReference.h"
#include "Material.h"
#include "MaterialUniformAccess.h"

namespace Ailurus
{
	class ReadOnlyMaterialInstance : public Asset
	{
	public:
		Material* GetTargetMaterial() const;
		void UpdateMaterialUniformData();

	protected:
		friend class AssetsSystem;
		ReadOnlyMaterialInstance(uint64_t assetId, const AssetReference<Material>& targetMaterial);

	protected:
		AssetReference<Material> targetMaterial;
		MaterialUniformValueMap uniformValueMap;
	};

	class ReadWriteMaterialInstance : public ReadOnlyMaterialInstance
	{
	public:
		void SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value);
	
	private:
		friend class AssetsSystem;
		ReadWriteMaterialInstance(uint64_t assetId, const AssetReference<Material>& targetMaterial);
	};

} // namespace Ailurus