#include "Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h"

namespace Ailurus
{

	CompStaticMeshRender::CompStaticMeshRender(const AssetRef<Model>& model, const AssetRef<MaterialInstance>& material)
		: _modelAsset(model)
		, _materialAsset(material)
	{
	}

	CompStaticMeshRender::~CompStaticMeshRender() = default;

	const AssetRef<Model>& CompStaticMeshRender::GetModelAsset() const
	{
		return _modelAsset;
	}

	const AssetRef<MaterialInstance>& CompStaticMeshRender::GetMaterialInstanceAsset() const
	{
		return _materialAsset;
	}
} // namespace Ailurus
