#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Application/RenderSystem/Uniform/UniformValue.h"
#include "Ailurus/Application/AssetsSystem/AssetReference.h"
#include "Material.h"

namespace Ailurus
{
	class ReadOnlyMaterialInstance : public Asset
	{
	protected:
		struct Entry
		{
			RenderPassType pass;
			uint32_t bindingId;
			std::string access;
		};

		struct EntryHash
		{
			std::size_t operator()(const Entry& entry) const;
		};

		struct EntryEqual
		{
			bool operator()(const Entry& lhs, const Entry& rhs) const;
		};

	public:
		Material* GetTargetMaterial() const;
		void UpdateMaterialUniformData();

	protected:
		friend class AssetsSystem;
		ReadOnlyMaterialInstance(uint64_t assetId, const AssetReference<Material>& targetMaterial);

	protected:
		AssetReference<Material> targetMaterial;
		std::unordered_map<Entry, UniformValue, EntryHash, EntryEqual> uniformValueMap;
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