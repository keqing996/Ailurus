#pragma once

#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Application/RenderSystem/Shader/ShaderStage.h"

namespace Ailurus
{
	REFLECTION_ENUM(MaterialUniformVariableType,
		Int,
		Uint,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4)

	class MaterialUniformVariable
	{
	public:
		virtual ~MaterialUniformVariable() = default;
		MaterialUniformVariable(const std::string& uniformBlockName, const std::string& uniformValueName);

	public:
		virtual auto GetType() const -> MaterialUniformVariableType = 0;
		virtual auto GetData() const -> const std::byte* = 0;

		auto GetUniformBlockName() const -> const std::string&;
		auto GetUniformValueName() const -> const std::string&;

	protected:
		std::vector<ShaderStage> _usedStages;
		std::string _uniformBlockName;
		std::string _uniformValueName;
	};

} // namespace Ailurus