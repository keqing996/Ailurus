#pragma once

#include <string>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/Shader/ShaderStage.h"

namespace Ailurus
{
	class RhiShader;

	class Shader : public NonCopyable
	{
	public:
		Shader(ShaderStage stage, const std::string& name);
		~Shader() override;

	public:
		ShaderStage GetStage() const;
		const std::string& GetName() const;

		const RhiShader* GetImpl() const;

	private:
		ShaderStage _stage;
		std::string _name;
		std::unique_ptr<RhiShader> _pImpl;
	};
} // namespace Ailurus