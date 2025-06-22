#pragma once

#include "CompRender.h"
#include "Ailurus/Application/AssetsSystem/Model/Model.h"
#include "Ailurus/Application/AssetsSystem/Material/MaterialInstance.h"
#include "Ailurus/Application/AssetsSystem/AssetRef.h"

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
		const AssetRef<MaterialInstance>& GetMaterialAsset() const;

	private:
		AssetRef<Model> _modelAsset;
		AssetRef<MaterialInstance> _materialAsset;
	};
} // namespace Ailurus