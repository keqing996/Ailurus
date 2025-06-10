#include <Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	UniformVariable::UniformVariable(const std::string& name)
		: _name(name)
	{
	}

	const std::string& UniformVariable::GetName() const
	{
		return _name;
	}

	UniformVariableNumeric::UniformVariableNumeric(const std::string& name, UniformValueType type)
		: UniformVariable(name)
		, _type(type)
	{
		switch (type)
		{
			case UniformValueType::Int:
				_value = 0;
				break;
			case UniformValueType::Float:
				_value = 0.0f;
				break;
			case UniformValueType::Vector2f:
				_value = Vector2f(0.0f, 0.0f);
				break;
			case UniformValueType::Vector3f:
				_value = Vector3f(0.0f, 0.0f, 0.0f);
				break;
			case UniformValueType::Vector4f:
				_value = Vector4f(0.0f, 0.0f, 0.0f, 1.0f);
				break;
			case UniformValueType::Mat3:
				_value = Matrix4x4f::Identity;
				break;
			case UniformValueType::Mat4:
				_value = Matrix4x4f::Identity;
				break;
			default:
				Logger::LogError("UniformVariableNumeric: Unsupported UniformValueType.");
				break;
		}
	}

	const UniformValueType& UniformVariableNumeric::ValueType() const
	{
		return _type;
	}

	const UniformValue& UniformVariableNumeric::GetValue() const
	{
		return _value;
	}

	void UniformVariableNumeric::SetValue(UniformValue newValue)
	{
		_value = newValue;
	}

	UniformVaribleType UniformVariableNumeric::VaribleType() const
	{
		return UniformVaribleType::Numeric;
	}

	UniformVariableStructure::UniformVariableStructure(const std::string& name)
		: UniformVariable(name)
	{
	}

	UniformVariableNumeric* UniformVariableStructure::AddNumericMember(const std::string& name, UniformValueType type)
	{
		auto member = std::make_unique<UniformVariableNumeric>(name, type);
		UniformVariableNumeric* memberPtr = member.get();
		_members.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariableStructure* UniformVariableStructure::AddStructureMember(const std::string& name)
	{
		auto member = std::make_unique<UniformVariableStructure>(name);
		UniformVariableStructure* memberPtr = member.get();
		_members.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariableArray* UniformVariableStructure::AddArrayMember(const std::string& name)
	{
		auto member = std::make_unique<UniformVariableArray>(name);
		UniformVariableArray* memberPtr = member.get();
		_members.push_back(std::move(member));
		return memberPtr;
	}

	const std::vector<std::unique_ptr<UniformVariable>>& UniformVariableStructure::GetMembers() const
	{
		return _members;
	}

	UniformVaribleType UniformVariableStructure::VaribleType() const
	{
		return UniformVaribleType::Structure;
	}

	UniformVariableArray::UniformVariableArray(const std::string& name)
		: UniformVariable(name)
	{
	}

	UniformVariableNumeric* UniformVariableArray::AddNumericMember(UniformValueType type)
	{
		auto member = std::make_unique<UniformVariableNumeric>(std::to_string(_members.size()), type);
		UniformVariableNumeric* memberPtr = member.get();
		_members.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariableStructure* UniformVariableArray::AddStructureMember()
	{
		auto member = std::make_unique<UniformVariableStructure>(std::to_string(_members.size()));
		UniformVariableStructure* memberPtr = member.get();
		_members.push_back(std::move(member));
		return memberPtr;
	}

	const std::vector<std::unique_ptr<UniformVariable>>& UniformVariableArray::GetMembers() const
	{
		return _members;
	}

	UniformVaribleType UniformVariableArray::VaribleType() const
	{
		return UniformVaribleType::Array;
	}
} // namespace Ailurus
