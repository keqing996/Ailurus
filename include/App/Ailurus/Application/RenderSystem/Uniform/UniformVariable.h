#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "UniformValue.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(UniformVaribleType,
		Numeric,
		Structure,
		Array);

	class UniformVariable
	{
	public:
		virtual UniformVaribleType VaribleType() const = 0;
	};

	class UniformVariableNumeric;
	class UniformVariableStructure;
	class UniformVariableArray;

	class UniformVariableNumeric : public UniformVariable
	{
	public:
		UniformVariableNumeric(UniformValueType type);

		auto ValueType() const -> const UniformValueType&;
		auto GetValue() const -> const UniformValue&;
		auto SetValue(UniformValue newValue) -> void;
		auto VaribleType() const -> UniformVaribleType override;

	private:
		UniformValueType _type;
		UniformValue _value;
	};

	class UniformVariableStructure : public UniformVariable
	{
	public:
		auto AddMember(const std::string& name, std::unique_ptr<UniformVariable>&& pUniformVar) -> void;
		auto GetMembers() const -> const std::unordered_map<std::string, std::unique_ptr<UniformVariable>>&;
		auto VaribleType() const -> UniformVaribleType override;

	private:
		std::unordered_map<std::string, std::unique_ptr<UniformVariable>> _members;
	};

	class UniformVariableArray : public UniformVariable
	{
	public:
		auto AddMember(std::unique_ptr<UniformVariable>&& pUniformVar) -> void;
		auto GetMembers() const -> const std::vector<std::unique_ptr<UniformVariable>>&;
		auto VaribleType() const -> UniformVaribleType override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _members;
	};

} // namespace Ailurus