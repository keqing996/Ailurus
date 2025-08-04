#include "Ailurus/Application/RenderSystem/Shader/Shader.h"
#include "VulkanContext/Shader/VulkanShader.h"

namespace Ailurus
{
	Shader::Shader(ShaderStage stage, const std::string& name)
		: _stage(stage)
		, _name(name)
		, _pImpl(std::make_unique<VulkanShader>(name))
	{
	}

	Shader::~Shader()
	{
	}

	ShaderStage Shader::GetStage() const
	{
		return _stage;
	}

	const std::string& Shader::GetName() const
	{
		return _name;
	}

	const VulkanShader* Shader::GetImpl() const
	{
		return _pImpl.get();
	}
} // namespace Ailurus
