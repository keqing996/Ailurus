#include "Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h"

namespace Ailurus
{

	CompStaticMeshRender::CompStaticMeshRender(const AssetReference<Model>& model, const AssetReference<MaterialInstance>& material)
		: _modelAsset(model)
		, _materialAsset(material)
	{
	}

	CompStaticMeshRender::~CompStaticMeshRender() = default;

	const AssetReference<Model>& CompStaticMeshRender::GetModelAsset() const
	{
		return _modelAsset;
	}

	const AssetReference<MaterialInstance>& CompStaticMeshRender::GetMaterialAsset() const
	{
		return _materialAsset;
	}
} // namespace Ailurus
