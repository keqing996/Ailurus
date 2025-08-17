#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Utility/String.h>

namespace Ailurus
{
	static uint32_t AlignOffset(uint32_t offset, uint32_t alignment)
	{
		return (offset + alignment - 1) & ~(alignment - 1);
	}

	static uint32_t GetStd140BaseAlignment(const UniformVariable* pUniform)
	{
		switch (pUniform->VaribleType())
		{
			case UniformVaribleType::Numeric:
			{
				const auto pNumericUniform = static_cast<const UniformVariableNumeric*>(pUniform);
				switch (pNumericUniform->ValueType())
				{
					case UniformValueType::Int:
					case UniformValueType::Float:
						return 4;
					case UniformValueType::Vector2:
						return 8;
					case UniformValueType::Vector3:
					case UniformValueType::Vector4:
						return 16;
					case UniformValueType::Mat4:
						return 16;
					default:
						Logger::LogError("UniformVariableNumeric: Unsupported UniformValueType for alignment calculation.");
						return 0;
				}
			}
			case UniformVaribleType::Structure:
			{
				uint32_t maxAlignment = 0;
				const auto pStructureUniform = static_cast<const UniformVariableStructure*>(pUniform);
				for (const auto& [_, member] : pStructureUniform->GetMembers())
					maxAlignment = std::max(maxAlignment, GetStd140BaseAlignment(member.get()));
				return AlignOffset(maxAlignment, 16);
			}
			case UniformVaribleType::Array:
			{
				const auto pArrayUniform = static_cast<const UniformVariableArray*>(pUniform);
				const auto& members = pArrayUniform->GetMembers();
				auto firstElementAlignment = members.empty() ? 16 : GetStd140BaseAlignment(members.front().get());
				return AlignOffset(firstElementAlignment, 16);
			}
		}

		return 0;
	}

	static void PopulateUniformOffsets(UniformVariable* pUniform, const std::string& prefix,
		uint32_t& currentOffset, std::unordered_map<std::string, uint32_t>& offsetMap)
	{
		switch (pUniform->VaribleType())
		{
			case UniformVaribleType::Numeric:
			{
				offsetMap[prefix] = currentOffset;

				const auto pNumericUniform = static_cast<const UniformVariableNumeric*>(pUniform);
				currentOffset += UniformValue::GetSize(pNumericUniform->ValueType());
				break;
			}
			case UniformVaribleType::Structure:
			{
				const auto pStructureUniform = static_cast<const UniformVariableStructure*>(pUniform);
				for (const auto& [name, pChildUniform] : pStructureUniform->GetMembers())
				{
					std::string memberName = prefix + "." + name;
					PopulateUniformOffsets(pChildUniform.get(), memberName, currentOffset, offsetMap);
				}
				break;
			}
			case UniformVaribleType::Array:
			{
				const auto pArrayUniform = static_cast<const UniformVariableArray*>(pUniform);
				const auto& members = pArrayUniform->GetMembers();
				for (auto i = 0; i < members.size(); ++i)
				{
					std::string elementName = prefix + "[" + std::to_string(i) + "]";
					UniformVariable* pMemberUniform = members[i].get();
					PopulateUniformOffsets(pMemberUniform, elementName, currentOffset, offsetMap);
					currentOffset = AlignOffset(currentOffset, GetStd140BaseAlignment(pMemberUniform));
				}
				break;
			}
			default:
				Logger::LogError("Unknown variable type encountered during offset population.");
		}

		uint32_t baseAlignment = GetStd140BaseAlignment(pUniform);
		currentOffset = AlignOffset(currentOffset, baseAlignment);
	}

	UniformBindingPoint::UniformBindingPoint(uint32_t bindingPoint, const std::vector<ShaderStage>& shaderStage, 
		const std::string& name, std::unique_ptr<UniformVariable>&& pUniform)
		: _bindingPoint(bindingPoint)
		, _usingStages(shaderStage)
		, _bindingPointName(name)
		, _pUniformVariable(std::move(pUniform))
	{
		uint32_t offset = 0;
		PopulateUniformOffsets(_pUniformVariable.get(), _bindingPointName, offset, _accessNameToBufferOffsetMap);
		_totalSize = offset;
	}

	UniformBindingPoint::~UniformBindingPoint() = default;

	const std::vector<ShaderStage>& UniformBindingPoint::GetUsingStages() const
	{
		return _usingStages;
	}

	uint32_t UniformBindingPoint::GetBindingPoint() const
	{
		return _bindingPoint;
	}

	const UniformVariable* UniformBindingPoint::GetUniform() const
	{
		return _pUniformVariable.get();
	}

	UniformVariable* UniformBindingPoint::GetUniform()
	{
		return _pUniformVariable.get();
	}

	uint32_t UniformBindingPoint::GetTotalSize() const
	{
		return _totalSize;
	}

	std::optional<uint32_t> UniformBindingPoint::GetAccessOffset(const std::string& accessName) const
	{
		auto it = _accessNameToBufferOffsetMap.find(accessName);
		if (it != _accessNameToBufferOffsetMap.end())
			return it->second;

		Logger::LogError("Access name '{}' not found in binding point {}.", accessName, _bindingPoint);
		return std::nullopt;
	}
} // namespace Ailurus