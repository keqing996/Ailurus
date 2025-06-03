#pragma once

#include <cstdint>
#include <memory>
#include "UniformValue.h"

namespace Ailurus
{
	class UniformVariable
	{
	public:
		UniformVariable(const std::string& name);

		const std::string& GetName() const;
		virtual uint32_t GetSize() const = 0;
		virtual bool IsStructure() const = 0;
		virtual uint32_t GetAlignment() const = 0;

	protected:
		std::string _name;
	};

	class UniformVariableNumeric : public UniformVariable
	{
	public:
		UniformVariableNumeric(const std::string& name, UniformValueType type);

		const UniformValueType& GetType() const;
		const UniformValue& GetValue() const;
		void SetValue(UniformValue newValue);
		uint32_t GetSize() const override;
		uint32_t GetAlignment() const override;
		bool IsStructure() const override;
		
	private:
		UniformValueType _type;
		UniformValue _value;
	};

	class UniformVariableStructure : public UniformVariable
	{
	public:
		UniformVariableStructure(const std::string& name);

		UniformVariableNumeric* AddNumericMember(const std::string& name, UniformValueType type);
		UniformVariableStructure* AddStructureMember(const std::string& name);
		UniformVariable* operator[](const std::string& name);
		const UniformVariable* operator[](const std::string& name) const;
		uint32_t GetSize() const override;
		uint32_t GetAlignment() const override;
		bool IsStructure() const override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _pMembers;
	};

} // namespace Ailurus