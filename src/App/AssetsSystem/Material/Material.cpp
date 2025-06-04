#include <cstdint>
#include <fstream>
#include <nlohmann/json.hpp>
#include <Ailurus/Application/AssetsSystem/Material/Material.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformVariableNumeric.h>
#include <Ailurus/Application/Application.h>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	const Material::RenderPassParameters* Material::GetRenderPassParameters(RenderPassType pass) const
	{
		auto const passItr = _renderPassParaMap.find(pass);
		if (passItr == _renderPassParaMap.end())
			return nullptr;

		return &passItr->second;
	}
} // namespace Ailurus
