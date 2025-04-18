#pragma once

#include <vector>
#include <string>
#include <initializer_list>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Math/Matrix4x4.hpp"

namespace Ailurus
{
	REFLECTION_ENUM(UniformDataType,
		None,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix3x3f,
		Matrix4x4f,
		Int,
		Int2,
		Int3,
		Int4)

	class UniformBuffer;

	class ShaderUniform : public NonCopyable
	{
	public:
		ShaderUniform(std::initializer_list<std::pair<std::string, UniformDataType>> initializerList);
		~ShaderUniform() override;

	public:
		size_t GetSize() const;
		void Upload(const std::string& name, float value);
		void Upload(const std::string& name, Matrix4x4f value);

	public:
		static size_t GetDataTypeSize(UniformDataType type);

	private:
		std::vector<UniformDataType> _dataTypeList;
		std::unordered_map<std::string, size_t> _nameToIndexMap;
		std::unique_ptr<UniformBuffer> _pUniformBuffer;
	};
} // namespace Ailurus