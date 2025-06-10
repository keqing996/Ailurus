#include <Ailurus/Application/AssetsSystem/Material/MaterialInstance.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
    Material* ReadOnlyMaterialInstance::GetTargetMaterial() const
    {
        return targetMaterial.Get();
    }

	std::size_t ReadOnlyMaterialInstance::EntryHash::operator()(const Entry& entry) const
	{
		return std::hash<uint32_t>()(static_cast<uint32_t>(entry.pass))
			^ std::hash<uint32_t>()(entry.setId)
			^ std::hash<uint32_t>()(entry.bindingId)
			^ std::hash<std::string>()(entry.access);
	}

	bool ReadOnlyMaterialInstance::EntryEqual::operator()(const Entry& lhs, const Entry& rhs) const
	{
		return lhs.pass == rhs.pass && lhs.setId == rhs.setId
			&& lhs.bindingId == rhs.bindingId && lhs.access == rhs.access;
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
            const MaterialSinglePass* passParams = targetMaterial->GetRenderPassParameters(entry.pass);
            if (passParams == nullptr)
            {
                Logger::LogError("Render pass parameters not found for pass: {}", EnumReflection<RenderPassType>::ToString(entry.pass));
                continue;
            }

            passParams->uniformSet.SetUniformValue(entry.setId, entry.bindingId, uniformValue);
        }
    }
} // namespace Ailurus