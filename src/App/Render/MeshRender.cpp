#include "Ailurus/Application/Render/MeshRender.h"

namespace Ailurus
{
	MeshRender::MeshRender() = default;

	MeshRender::~MeshRender() = default;

	Mesh* MeshRender::GetMesh() const
	{
		return _pMesh.get();
	}

	Material* MeshRender::GetMaterial() const
	{
		return _pMaterial.get();
	}

	void MeshRender::SetMaterial(const std::shared_ptr<Material>& pMat)
	{
		_pMaterial = pMat;
	}

	void MeshRender::SetMesh(const std::shared_ptr<Mesh>& pMesh)
	{
		_pMesh = pMesh;
	}
} // namespace Ailurus
