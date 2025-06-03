#pragma once

#include <vector>
#include <memory>
#include "Ailurus/Application/AssetsSystem/Asset.h"
#include "Ailurus/Application/AssetsSystem/AssetReference.h"
#include "Ailurus/Application/AssetsSystem/Mesh/Mesh.h"

namespace Ailurus
{
	class Model : public Asset
	{
	public:
		Model(std::vector<std::unique_ptr<Mesh>>&& meshes);
		const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const;

	private:
		friend class AssetsSystem;
		std::vector<std::unique_ptr<Mesh>> _meshes;
	};
}