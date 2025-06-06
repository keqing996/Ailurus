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

	uint32_t UniformVariableNumeric::GetAlignment() const
	{
		switch (_type)
		{
			case UniformValueType::Int:
			case UniformValueType::Float:
				return 4;
			case UniformValueType::Vector2f:
				return 8;
			case UniformValueType::Vector3f:
			case UniformValueType::Vector4f:
				return 16;
			case UniformValueType::Mat3:
			case UniformValueType::Mat4:
				return 16;
			default:
				Logger::LogError("UniformVariableNumeric: Unsupported UniformValueType for alignment calculation.");
				return 0;
		}
	}

	const UniformValueType& UniformVariableNumeric::GetType() const
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

	uint32_t UniformVariableNumeric::GetSize() const
	{
		switch (_type)
		{
			case UniformValueType::Int:
				return sizeof(int32_t);
			case UniformValueType::Float:
				return sizeof(float);
			case UniformValueType::Vector2f:
				return sizeof(Vector2f);
			case UniformValueType::Vector3f:
				return sizeof(Vector3f);
			case UniformValueType::Vector4f:
				return sizeof(Vector4f);
			case UniformValueType::Mat3:
				return sizeof(Matrix4x4f);
			case UniformValueType::Mat4:
				return sizeof(Matrix4x4f);
			default:
				Logger::LogError("UniformVariableNumeric: Unsupported UniformValueType for size calculation.");
				return 0;
		}
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
		_pMembers.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariableStructure* UniformVariableStructure::AddStructureMember(const std::string& name)
	{
		auto member = std::make_unique<UniformVariableStructure>(name);
		UniformVariableStructure* memberPtr = member.get();
		_pMembers.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariableArray* UniformVariableStructure::AddArrayMember(const std::string& name)
	{
		auto member = std::make_unique<UniformVariableArray>(name);
		UniformVariableArray* memberPtr = member.get();
		_pMembers.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariable* UniformVariableStructure::operator[](const std::string& name)
	{
		for (const auto& member : _pMembers)
		{
			if (member->GetName() == name)
				return member.get();
		}

		return nullptr;
	}

	const UniformVariable* UniformVariableStructure::operator[](const std::string& name) const
	{
		for (const auto& member : _pMembers)
		{
			if (member->GetName() == name)
				return member.get();
		}

		return nullptr;
	}

	uint32_t UniformVariableStructure::GetSize() const
	{
		uint32_t size = 0;
		uint32_t maxAlignment = 0;

		for (const auto& member : _pMembers)
		{
			uint32_t memberSize = member->GetSize();
			uint32_t alignment = member->GetAlignment();

			// Align current offset
			size = (size + alignment - 1) & ~(alignment - 1);

			// Add member size
			size += memberSize;

			if (alignment > maxAlignment)
				maxAlignment = alignment;
		}

		// The structure's size must be aligned to its largest member's alignment (std140)
		size = (size + maxAlignment - 1) & ~(maxAlignment - 1);

		return size;
	}

	UniformVaribleType UniformVariableStructure::VaribleType() const
	{
		return UniformVaribleType::Structure;
	}

	uint32_t UniformVariableStructure::GetAlignment() const
	{
		uint32_t maxAlignment = 0;

		for (const auto& member : _pMembers)
		{
			uint32_t alignment = member->GetAlignment();
			if (alignment > maxAlignment)
				maxAlignment = alignment;
		}

		return maxAlignment;
	}

	UniformVariableArray::UniformVariableArray(const std::string& name)
		: UniformVariable(name)
	{
	}

	UniformVariableNumeric* UniformVariableArray::AddNumericMember(UniformValueType type)
	{
		auto member = std::make_unique<UniformVariableNumeric>(std::to_string(_pMembers.size()), type);
		UniformVariableNumeric* memberPtr = member.get();
		_pMembers.push_back(std::move(member));
		return memberPtr;
	}

	UniformVariableStructure* UniformVariableArray::AddStructureMember()
	{
		auto member = std::make_unique<UniformVariableStructure>(std::to_string(_pMembers.size()));
		UniformVariableStructure* memberPtr = member.get();
		_pMembers.push_back(std::move(member));
		return memberPtr;
	}

	uint32_t UniformVariableArray::GetSize() const
	{
		uint32_t size = 0;
		uint32_t maxAlignment = 0;

		for (const auto& member : _pMembers)
		{
			uint32_t memberSize = member->GetSize();
			uint32_t alignment = member->GetAlignment();

			// Align current offset
			size = (size + alignment - 1) & ~(alignment - 1);

			// Add member size
			size += memberSize;

			if (alignment > maxAlignment)
				maxAlignment = alignment;
		}

		// The array's size must be aligned to its largest member's alignment (std140)
		size = (size + maxAlignment - 1) & ~(maxAlignment - 1);

		return size;
	}

	UniformVaribleType UniformVariableArray::VaribleType() const
	{
		return UniformVaribleType::Array;
	}

	uint32_t UniformVariableArray::GetAlignment() const
	{
		uint32_t maxAlignment = 0;

		for (const auto& member : _pMembers)
		{
			uint32_t alignment = member->GetAlignment();
			if (alignment > maxAlignment)
				maxAlignment = alignment;
		}

		return maxAlignment;
	}
} // namespace Ailurus
