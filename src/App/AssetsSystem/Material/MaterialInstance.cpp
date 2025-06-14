#include "Ailurus/Application/AssetsSystem/Material/MaterialUniformAccess.h"
#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
    MaterialInstance::MaterialInstance(uint64_t assetId, const AssetRef<Material>& targetMaterial)
        : Asset(assetId)
        , targetMaterial(targetMaterial)
    {
    }

    Material* MaterialInstance::GetTargetMaterial() const
    {
        return targetMaterial.Get();
    }

    void MaterialInstance::UpdateMaterialUniformData()
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

	void MaterialInstance::SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		MaterialUniformAccess entry{ pass, bindingId, access };
		uniformValueMap[entry] = value;
	}

	void MaterialInstance::SetUniformValue(const MaterialUniformAccess& entry, const UniformValue& value)
	{
		uniformValueMap[entry] = value;
	}
} // namespace Ailurus