#pragma once

#include <cstdint>
#include <memory>
#include "UniformValue.h"

namespace Ailurus
{
	enum class UniformVaribleType
	{
		Numeric,
		Structure,
		Array
	};

	class UniformVariable
	{
	public:
		UniformVariable(const std::string& name);

		const std::string& GetName() const;
		virtual UniformVaribleType VaribleType() const = 0;

	protected:
		std::string _name;
	};

	class UniformVariableNumeric;
	class UniformVariableStructure;
	class UniformVariableArray;

	class UniformVariableNumeric : public UniformVariable
	{
	public:
		UniformVariableNumeric(const std::string& name, UniformValueType type);

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
		explicit UniformVariableStructure(const std::string& name);

		auto AddNumericMember(const std::string& name, UniformValueType type) -> UniformVariableNumeric*;
		auto AddStructureMember(const std::string& name) -> UniformVariableStructure*;
		auto AddArrayMember(const std::string& name) -> UniformVariableArray*;
		auto GetMembers() const -> const std::vector<std::unique_ptr<UniformVariable>>&;
		auto VaribleType() const -> UniformVaribleType override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _members;
	};

	class UniformVariableArray : public UniformVariable
	{
	public:
		UniformVariableArray(const std::string& name);

		auto AddNumericMember(UniformValueType type) -> UniformVariableNumeric*;
		auto AddStructureMember() -> UniformVariableStructure*;
		auto GetMembers() const -> const std::vector<std::unique_ptr<UniformVariable>>&;
		auto VaribleType() const -> UniformVaribleType override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _members;
	};

} // namespace Ailurus