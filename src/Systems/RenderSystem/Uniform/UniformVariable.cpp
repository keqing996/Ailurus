#include <Ailurus/Systems/RenderSystem/Uniform/UniformVariable.h>
#include <Ailurus/Utility/Logger.h>

namespace Ailurus
{
	UniformVariableNumeric::UniformVariableNumeric(UniformValueType type)
		: _type(type)
	{
	}

	UniformVariableNumeric::~UniformVariableNumeric()
	{
	}

	const UniformValueType& UniformVariableNumeric::ValueType() const
	{
		return _type;
	}

	UniformVariableType UniformVariableNumeric::VariableType() const
	{
		return UniformVariableType::Numeric;
	}

	UniformVariableStructure::~UniformVariableStructure()
	{
	}

	void UniformVariableStructure::AddMember(const std::string& name, std::unique_ptr<UniformVariable>&& pUniformVar)
	{
		_members.push_back({ name, std::move(pUniformVar) });
	}

	const std::vector<UniformVariableStructure::Member>& UniformVariableStructure::GetMembers() const
	{
		return _members;
	}

	UniformVariableType UniformVariableStructure::VariableType() const
	{
		return UniformVariableType::Structure;
	}

	UniformVariableArray::UniformVariableArray(std::unique_ptr<UniformVariable>&& elementType, int count)
	{
		_members.reserve(count);
		if (elementType->VariableType() == UniformVariableType::Numeric)
		{
			const auto pNumeric = static_cast<const UniformVariableNumeric*>(elementType.get());
			const auto valueType = pNumeric->ValueType();
			for (int i = 0; i < count; ++i)
			{
				_members.push_back(std::make_unique<UniformVariableNumeric>(valueType));
			}
		}
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

	UniformVariableType UniformVariableArray::VariableType() const
	{
		return UniformVariableType::Array;
	}
} // namespace Ailurus
