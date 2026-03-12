#pragma once

#include <string>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "ShaderStage.h"

namespace Ailurus
{
	class VulkanShader;

	class Shader : public NonCopyable
	{
	public:
		Shader(ShaderStage stage, const std::string& name);
		~Shader();

	public:
		ShaderStage GetStage() const;
		const std::string& GetName() const;

		const VulkanShader* GetImpl() const;

	private:
		ShaderStage _stage;
		std::string _name;
		std::unique_ptr<VulkanShader> _pImpl;
	};
} // namespace Ailurus