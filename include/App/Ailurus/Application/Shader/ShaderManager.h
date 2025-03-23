#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Application/Shader/Shader.h"

namespace Ailurus
{
	class ShaderManager
	{
	public:
		ShaderManager() = default;

	public:
		void Clear();
		Shader* GetShader(ShaderStage stage, const std::string& path);

		template <ShaderStage stage>
		Shader* GetShader(const std::string& path)
		{
			return GetShader(stage, path);
		}

	private:
		Shader* LoadShader(ShaderStage stage, const std::string& path);

	private:
		std::unordered_map<ShaderStage, std::unordered_map<std::string, std::unique_ptr<Shader>>> _library;
	};
} // namespace Ailurus
