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
		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetAlignment() const = 0;
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

		const UniformValueType& GetType() const;
		const UniformValue& GetValue() const;
		void SetValue(UniformValue newValue);
		uint32_t GetSize() const override;
		uint32_t GetAlignment() const override;
		UniformVaribleType VaribleType() const override;

	private:
		UniformValueType _type;
		UniformValue _value;
	};

	class UniformVariableStructure : public UniformVariable
	{
	public:
		explicit UniformVariableStructure(const std::string& name);

		UniformVariableNumeric* AddNumericMember(const std::string& name, UniformValueType type);
		UniformVariableStructure* AddStructureMember(const std::string& name);
		UniformVariableArray* AddArrayMember(const std::string& name);
		UniformVariable* operator[](const std::string& name);
		const UniformVariable* operator[](const std::string& name) const;
		uint32_t GetSize() const override;
		uint32_t GetAlignment() const override;
		UniformVaribleType VaribleType() const override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _pMembers;
	};

	class UniformVariableArray : public UniformVariable
	{
	public:
		UniformVariableArray(const std::string& name);

		UniformVariableNumeric* AddNumericMember(UniformValueType type);
		UniformVariableStructure* AddStructureMember();
		uint32_t GetSize() const override;
		uint32_t GetAlignment() const override;
		UniformVaribleType VaribleType() const override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _pMembers;
	};

} // namespace Ailurus