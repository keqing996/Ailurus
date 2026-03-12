#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "UniformValue.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(UniformVariableType,
		Numeric,
		Structure,
		Array);

	class UniformVariable
	{
	public:
		virtual UniformVariableType VariableType() const = 0;
		virtual ~UniformVariable() = default;
	};

	class UniformVariableNumeric;
	class UniformVariableStructure;
	class UniformVariableArray;

	class UniformVariableNumeric : public UniformVariable
	{
	public:
		UniformVariableNumeric(UniformValueType type);
		~UniformVariableNumeric() override;

		auto ValueType() const -> const UniformValueType&;
		auto VariableType() const -> UniformVariableType override;

	private:
		UniformValueType _type;
	};

	class UniformVariableStructure : public UniformVariable
	{
	public:
		struct Member
		{
			std::string name;
			std::unique_ptr<UniformVariable> pVariable;
		};

	public:
		~UniformVariableStructure() override;

	public:
		auto AddMember(const std::string& name, std::unique_ptr<UniformVariable>&& pUniformVar) -> void;
		auto GetMembers() const -> const std::vector<Member>&;
		auto VariableType() const -> UniformVariableType override;

	private:
		std::vector<Member> _members;
	};

	class UniformVariableArray : public UniformVariable
	{
	public:
		UniformVariableArray() = default;
		UniformVariableArray(std::unique_ptr<UniformVariable>&& elementType, int count);
		~UniformVariableArray() override;

	public:
		auto AddMember(std::unique_ptr<UniformVariable>&& pUniformVar) -> void;
		auto GetMembers() const -> const std::vector<std::unique_ptr<UniformVariable>>&;
		auto VariableType() const -> UniformVariableType override;

	private:
		std::vector<std::unique_ptr<UniformVariable>> _members;
	};

} // namespace Ailurus