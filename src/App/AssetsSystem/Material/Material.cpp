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

	void Material::InitUniformBuffer()
	{
		for (auto& [pass, passParams] : _renderPassParaMap)
		{
			const size_t bufferSize = passParams.uniformSet.GetBufferSize();
			passParams.pUniformBuffer = std::make_unique<VulkanUniformBuffer>(passParams.uniformSet);
		}
	}
} // namespace Ailurus
