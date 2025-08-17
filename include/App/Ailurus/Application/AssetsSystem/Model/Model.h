#pragma once

#include <vector>
#include <memory>
#include "Ailurus/Application/AssetsSystem/Asset.h"
#include "Ailurus/Application/AssetsSystem/Mesh/Mesh.h"

namespace Ailurus
{
	class Model : public TypedAsset<AssetType::Model> 
	{
	public:
		const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const;

	private:
		friend class AssetsSystem;
		Model(uint64_t assetId, std::vector<std::unique_ptr<Mesh>>&& meshes);

	private:
		std::vector<std::unique_ptr<Mesh>> _meshes;
	};
}