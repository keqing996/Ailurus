#include "Ailurus/Application/Component/CompMeshRender.h"

namespace Ailurus
{
	CompMeshRender::CompMeshRender() = default;

	CompMeshRender::~CompMeshRender() = default;

	Mesh* CompMeshRender::GetMesh() const
	{
		return _pMesh.get();
	}

	Material* CompMeshRender::GetMaterial() const
	{
		return _pMaterial.get();
	}

	void CompMeshRender::SetMaterial(const std::shared_ptr<Material>& pMat)
	{
		_pMaterial = pMat;
	}

	void CompMeshRender::SetMesh(const std::shared_ptr<Mesh>& pMesh)
	{
		_pMesh = pMesh;
	}
} // namespace Ailurus
