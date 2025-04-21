#pragma once

#include <memory>
#include <unordered_map>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Application/Render/Shader/Shader.h"

namespace Ailurus
{
	class ShaderLibrary: public NonCopyable
	{
	public:
		~ShaderLibrary() override;

	public:
		void Clear();
		Shader* GetShader(ShaderStage stage, const std::string& path);

		template <ShaderStage stage>
		Shader* GetShader(const std::string& path)
		{
			return GetShader(stage, path);
		}

	private:
		friend class Render;
		ShaderLibrary() = default;

	private:
		Shader* LoadShader(ShaderStage stage, const std::string& path);

	private:
		std::unordered_map<ShaderStage, std::unordered_map<std::string, std::unique_ptr<Shader>>> _library;
	};
} // namespace Ailurus
