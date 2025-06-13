#pragma once

#include "CompRender.h"
#include "Ailurus/Application/AssetsSystem/Model/Model.h"
#include "Ailurus/Application/AssetsSystem/Material/MaterialInstance.h"
#include "Ailurus/Application/AssetsSystem/AssetReference.h"

namespace Ailurus
{
	class Material;

	class CompStaticMeshRender : public TComponent<ComponentType::StaticMeshRender, CompRender>
	{
	public:
		CompStaticMeshRender(const AssetReference<Model>& model, const AssetReference<MaterialInstance>& material);
		~CompStaticMeshRender() override;

	public:
		const AssetReference<Model>& GetModelAsset() const { return _modelAsset; }
		const AssetReference<MaterialInstance>& GetMaterialAsset() const { return _materialAsset; }

	private:
		AssetReference<Model> _modelAsset;
		AssetReference<MaterialInstance> _materialAsset;
	};
} // namespace Ailurus