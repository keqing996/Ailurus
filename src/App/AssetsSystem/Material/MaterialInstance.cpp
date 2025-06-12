#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
    ReadOnlyMaterialInstance::ReadOnlyMaterialInstance(uint64_t assetId, const AssetReference<Material>& targetMaterial)
        : Asset(assetId)
        , targetMaterial(targetMaterial)
    {
    }

    Material* ReadOnlyMaterialInstance::GetTargetMaterial() const
    {
        return targetMaterial.Get();
    }

	std::size_t ReadOnlyMaterialInstance::EntryHash::operator()(const Entry& entry) const
	{
		return std::hash<uint32_t>()(static_cast<uint32_t>(entry.pass))
			^ std::hash<uint32_t>()(entry.bindingId)
			^ std::hash<std::string>()(entry.access);
	}

	bool ReadOnlyMaterialInstance::EntryEqual::operator()(const Entry& lhs, const Entry& rhs) const
	{
		return lhs.pass == rhs.pass && lhs.bindingId == rhs.bindingId 
            && lhs.access == rhs.access;
	}

    void ReadOnlyMaterialInstance::UpdateMaterialUniformData()
    {
        if (targetMaterial == nullptr)
        {
            Logger::LogError("Target material is null, cannot update uniform data.");
            return;
        }

        for (const auto& [entry, uniformValue] : uniformValueMap)
        {
            targetMaterial.Get()->UpdateUniformValue(entry.pass, entry.bindingId, 
                entry.access, uniformValue);
        }
	}

    ReadWriteMaterialInstance::ReadWriteMaterialInstance(uint64_t assetId, const AssetReference<Material>& targetMaterial)
        : ReadOnlyMaterialInstance(assetId, targetMaterial)
    {
    }

	void ReadWriteMaterialInstance::SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		Entry entry{ pass, bindingId, access };
		uniformValueMap[entry] = value;
	}
} // namespace Ailurus