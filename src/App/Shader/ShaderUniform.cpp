#include "Ailurus/Application/Shader/ShaderUniform.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	ShaderUniform::ShaderUniform(std::initializer_list<std::pair<std::string, UniformDataType>> initializerList)
	{
		size_t index = 0;
		for (const auto& [name, type] : initializerList)
		{
			_dataTypeList.push_back(type);
			_nameToIndexMap[name] = index++;
		}
	}

	ShaderUniform::~ShaderUniform()
	{
	}

	size_t ShaderUniform::GetSize() const
	{
	}

	void ShaderUniform::Upload(const std::string& name, float value)
	{
	}

	void ShaderUniform::Upload(const std::string& name, Matrix4x4f value)
	{
	}

	size_t ShaderUniform::GetDataTypeSize(UniformDataType type)
	{
		static std::unordered_map<UniformDataType, unsigned int> map = {
			{ UniformDataType::None, 0 },
			{ UniformDataType::Bool, 1 },
			{ UniformDataType::Float, 4 },
			{ UniformDataType::Float2, 4 * 2 },
			{ UniformDataType::Float3, 4 * 3 },
			{ UniformDataType::Float4, 4 * 4 },
			{ UniformDataType::Matrix3x3f, 4 * 3 * 3 },
			{ UniformDataType::Matrix4x4f, 4 * 4 * 4 },
			{ UniformDataType::Int, 4 },
			{ UniformDataType::Int2, 4 * 2 },
			{ UniformDataType::Int3, 4 * 3 },
			{ UniformDataType::Int4, 4 * 4 },
		};

		if (map.contains(type))
			return map[type];

		Logger::LogError("Do not know {} data size.", EnumReflection<UniformDataType>::ToString(type));
		return 0;
	}
} // namespace Ailurus