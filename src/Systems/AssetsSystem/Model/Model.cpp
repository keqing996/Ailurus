#include <Ailurus/Systems/AssetsSystem/Model/Model.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Math/Vector2.hpp>
#include <Ailurus/Math/Vector3.hpp>

namespace Ailurus
{
	Model::Model(uint64_t assetId, std::vector<std::unique_ptr<Mesh>>&& meshes)
		: TypedAsset(assetId)
		, _meshes(std::move(meshes))
	{
		// Merge all mesh AABBs
		if (!_meshes.empty())
		{
			_localAABB = _meshes[0]->GetLocalAABB();
			for (size_t i = 1; i < _meshes.size(); i++)
				_localAABB = AABBf::Merge(_localAABB, _meshes[i]->GetLocalAABB());
		}
	}

	const std::vector<std::unique_ptr<Mesh>>& Model::GetMeshes() const
	{
		return _meshes;
	}

	const AABBf& Model::GetLocalAABB() const
	{
		return _localAABB;
	}
} // namespace Ailurus