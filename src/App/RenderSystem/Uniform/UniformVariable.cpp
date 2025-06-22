#include <Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	UniformVariableNumeric::UniformVariableNumeric(UniformValueType type)
		: _type(type)
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

	UniformVariableNumeric::~UniformVariableNumeric()
	{
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

	UniformVariableStructure::~UniformVariableStructure()
	{
	}

	void UniformVariableStructure::AddMember(const std::string& name, std::unique_ptr<UniformVariable>&& pUniformVar)
	{
		_members[name] = std::move(pUniformVar);
	}

	const std::unordered_map<std::string, std::unique_ptr<UniformVariable>>& UniformVariableStructure::GetMembers() const
	{
		return _members;
	}

	UniformVaribleType UniformVariableStructure::VaribleType() const
	{
		return UniformVaribleType::Structure;
	}

	UniformVariableArray::~UniformVariableArray()
	{
	}

	void UniformVariableArray::AddMember(std::unique_ptr<UniformVariable>&& pUniformVar)
	{
		_members.push_back(std::move(pUniformVar));
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
