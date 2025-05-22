#include "Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h"

namespace Ailurus
{
	CompStaticMeshRender::CompStaticMeshRender() = default;

	CompStaticMeshRender::~CompStaticMeshRender() = default;

	Mesh* CompStaticMeshRender::GetMesh() const
	{
		return _pMesh.get();
	}

	Material* CompStaticMeshRender::GetMaterial() const
	{
		return _pMaterial.get();
	}

	void CompStaticMeshRender::SetMaterial(const std::shared_ptr<Material>& pMat)
	{
		_pMaterial = pMat;
	}

	void CompStaticMeshRender::SetMesh(const std::shared_ptr<Mesh>& pMesh)
	{
		_pMesh = pMesh;
	}
} // namespace Ailurus
