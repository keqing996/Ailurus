#include <Ailurus/Application/RenderSystem/Uniform/UniformVariable.h>
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

	UniformVaribleType UniformVariableNumeric::VaribleType() const
	{
		return UniformVaribleType::Numeric;
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
