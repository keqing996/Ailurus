#pragma once

#include <vector>
#include <memory>
#include "Ailurus/Systems/AssetsSystem/Asset.h"
#include "Ailurus/Systems/AssetsSystem/Mesh/Mesh.h"
#include "Ailurus/Math/AABB.hpp"

namespace Ailurus
{
	class Model : public TypedAsset<AssetType::Model> 
	{
	public:
		const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const;
		const AABBf& GetLocalAABB() const;

	private:
		friend class AssetsSystem;
		Model(uint64_t assetId, std::vector<std::unique_ptr<Mesh>>&& meshes);

	private:
		std::vector<std::unique_ptr<Mesh>> _meshes;
		AABBf _localAABB;
	};
}