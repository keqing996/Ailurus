#include <Ailurus/Application/AssetsSystem/Model/Model.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Math/Vector2.hpp>
#include <Ailurus/Math/Vector3.hpp>

namespace Ailurus
{
	Model::Model(uint64_t assetId, std::vector<std::unique_ptr<Mesh>>&& meshes)
		: TypedAsset(assetId)
		, _meshes(std::move(meshes))
	{
	}

	const std::vector<std::unique_ptr<Mesh>>& Model::GetMeshes() const
	{
		return _meshes;
	}
} // namespace Ailurus