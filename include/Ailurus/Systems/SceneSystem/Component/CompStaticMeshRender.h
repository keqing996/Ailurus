#pragma once

#include "CompRender.h"
#include "Ailurus/Systems/AssetsSystem/Model/Model.h"
#include "Ailurus/Systems/AssetsSystem/Material/MaterialInstance.h"
#include "Ailurus/Systems/AssetsSystem/AssetRef.h"
#include "Ailurus/Math/AABB.hpp"

namespace Ailurus
{
	class Material;

	class CompStaticMeshRender : public TComponent<ComponentType::StaticMeshRender, CompRender>
	{
	public:
		CompStaticMeshRender(const AssetRef<Model>& model, const AssetRef<MaterialInstance>& material);
		~CompStaticMeshRender() override;

	public:
		const AssetRef<Model>& GetModelAsset() const;
		const AssetRef<MaterialInstance>& GetMaterialInstanceAsset() const;
		AABBf GetWorldAABB() const;

		nlohmann::json Serialize() const override;

	private:
		AssetRef<Model> _modelAsset;
		AssetRef<MaterialInstance> _materialAsset;
	};
} // namespace Ailurus