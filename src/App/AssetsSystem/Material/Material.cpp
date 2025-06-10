#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	Material::PerPass* Material::GetRenderPassParameters(RenderPassType pass) 
	{
		auto const passItr = _renderPassParaMap.find(pass);
		if (passItr == _renderPassParaMap.end())
			return nullptr;

		return &passItr->second;
	}

	void Material::SetUniformValue(RenderPassType pass, uint32_t bindingId, const std::string& access, const UniformValue& value)
	{
		auto* passParams = GetRenderPassParameters(pass);
		if (passParams == nullptr)
		{
			Logger::LogError("Material render pass not found: {}", EnumReflection<RenderPassType>::ToString(pass));
			return;
		}

		passParams->uniformSet.SetUniformValue(bindingId, access, value);
	}

} // namespace Ailurus
