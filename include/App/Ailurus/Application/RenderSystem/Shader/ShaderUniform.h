#pragma once

#include <vector>
#include <string>
#include <initializer_list>
#include <unordered_map>
#include <memory>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/EnumReflection.h"
#include "Ailurus/Math/Matrix4x4.hpp"
#include "Uniform/UniformDataType.h"

namespace Ailurus
{
	class UniformBuffer;

	class ShaderUniform : public NonCopyable
	{
	public:
		ShaderUniform(std::initializer_list<std::pair<std::string, UniformDataType>> initializerList);
		~ShaderUniform() override;

	public:
		void SetFloat(const std::string& name, float value);
		void SetBool(const std::string& name, bool value);
		void SetInt(const std::string& name, int32_t value);
		void SetMatrix4x4f(const std::string& name, const Matrix4x4f& value);

		void TransitionDataToGpu() const;

	public:
		static size_t GetDataTypeSize(UniformDataType type);

	private:
		std::unordered_map<std::string, size_t> _nameToOffsetMap;
		std::unique_ptr<UniformBuffer> _pUniformBuffer;
	};
} // namespace Ailurus