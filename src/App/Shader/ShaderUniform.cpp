#include "Ailurus/Application/Shader/ShaderUniform.h"
#include "Ailurus/Utility/Logger.h"
#include "Vulkan/DataBuffer/UniformBuffer.h"

namespace Ailurus
{
	ShaderUniform::ShaderUniform(std::initializer_list<std::pair<std::string, UniformDataType>> initializerList)
	{
		size_t totalSize = 0;
		for (const auto& [name, type] : initializerList)
		{
			if (_nameToOffsetMap.contains(name))
			{
				Logger::LogError("Uniform already has name {}", name);
				continue;
			}

			_nameToOffsetMap[name] = totalSize;
			totalSize += GetDataTypeSize(type);
		}

		_data.resize(totalSize);
		std::fill(_data.begin(), _data.end(), 0);

		_pUniformBuffer = std::make_unique<UniformBuffer>(totalSize);
		Upload();
	}

	ShaderUniform::~ShaderUniform()
	{
	}

	void ShaderUniform::SetFloat(const std::string& name, float value)
	{
		const auto itr = _nameToOffsetMap.find(name);
		if (itr == _nameToOffsetMap.end())
			return;

		const size_t offset = itr->second;
		const auto pWritePos = reinterpret_cast<float*>(_data.data() + offset);
		*pWritePos = value;
	}

	void ShaderUniform::SetBool(const std::string& name, bool value)
	{
		const auto itr = _nameToOffsetMap.find(name);
		if (itr == _nameToOffsetMap.end())
			return;

		const size_t offset = itr->second;
		const auto pWritePos = reinterpret_cast<bool*>(_data.data() + offset);
		*pWritePos = value;
	}

	void ShaderUniform::SetInt(const std::string& name, int32_t value)
	{
		const auto itr = _nameToOffsetMap.find(name);
		if (itr == _nameToOffsetMap.end())
			return;

		const size_t offset = itr->second;
		const auto pWritePos = reinterpret_cast<int32_t*>(_data.data() + offset);
		*pWritePos = value;
	}

	void ShaderUniform::SetMatrix4x4f(const std::string& name, const Matrix4x4f& value)
	{
		const auto itr = _nameToOffsetMap.find(name);
		if (itr == _nameToOffsetMap.end())
			return;

		const size_t offset = itr->second;
		const auto pWritePos = _data.data() + offset;

		auto colMajMat = value.Transpose();
		std::memcpy(pWritePos, colMajMat.GetRowMajorDataPtr(), sizeof(Matrix4x4f));
	}

	void ShaderUniform::Upload() const
	{
		_pUniformBuffer->UpdateData(0, _data.data(), _data.size());
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