#include "Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h"
#include "Ailurus/Systems/SceneSystem/Entity/Entity.h"
#include "Ailurus/Application.h"
#include "Ailurus/Systems/AssetsSystem/AssetsSystem.h"
#include <nlohmann/json.hpp>

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

	AABBf CompStaticMeshRender::GetWorldAABB() const
	{
		return _modelAsset->GetLocalAABB().Transform(GetEntity()->GetModelMatrix());
	}

	nlohmann::json CompStaticMeshRender::Serialize() const
	{
		nlohmann::json j;
		j["type"] = "StaticMeshRender";

		const auto* pAssetsSystem = Application::Get<AssetsSystem>();
		if (_modelAsset)
			j["modelPath"] = pAssetsSystem->GetAssetPath(_modelAsset.Get()->GetAssetId());
		if (_materialAsset)
			j["materialPath"] = pAssetsSystem->GetAssetPath(_materialAsset.Get()->GetAssetId());

		return j;
	}
} // namespace Ailurus
