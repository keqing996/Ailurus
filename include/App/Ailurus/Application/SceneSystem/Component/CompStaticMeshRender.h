#pragma once

#include <memory>
#include "CompRender.h"
#include "Ailurus/Application/AssetsSystem/Model/Model.h"

namespace Ailurus
{
	class Material;

	class CompStaticMeshRender : public TComponent<ComponentType::StaticMeshRender, CompRender>
	{
	public:
		CompStaticMeshRender(const AssetReference<Model>& model);
		~CompStaticMeshRender() override;

	public:
		Mesh* GetMesh() const;
		Material* GetMaterial() const;
		void SetMaterial(const std::shared_ptr<Material>& pMat);
		void SetMesh(const std::shared_ptr<Mesh>& pMesh);

	private:
		AssetReference<Model> _modelAssets;
		std::shared_ptr<Material> _pMaterial;
	};
} // namespace Ailurus