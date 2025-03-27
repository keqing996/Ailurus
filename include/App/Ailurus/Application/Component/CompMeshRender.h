#pragma once

#include <memory>
#include "CompRender.h"
#include "Ailurus/Application/Assets/Mesh.h"

namespace Ailurus
{
	class Material;

	class CompMeshRender : public CompRender
	{
	public:
		CompMeshRender();
		~CompMeshRender() override;

	public:
		ComponentType GetType() const override;
		Mesh* GetMesh() const;
		Material* GetMaterial() const;
		void SetMaterial(const std::shared_ptr<Material>& pMat);
		void SetMesh(const std::shared_ptr<Mesh>& pMesh);

	private:
		std::shared_ptr<Mesh> _pMesh;
		std::shared_ptr<Material> _pMaterial;
	};
} // namespace Ailurus