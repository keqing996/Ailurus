#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Application/RenderSystem/Uniform/UniformLayoutHelper.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Utility/String.h>

namespace Ailurus
{
	static uint32_t GetStd140BaseAlignment(const UniformVariable* pUniform)
	{
		switch (pUniform->VaribleType())
		{
			case UniformVaribleType::Numeric:
			{
				const auto pNumericUniform = static_cast<const UniformVariableNumeric*>(pUniform);
				return UniformLayoutHelper::GetStd140BaseAlignment(pNumericUniform->ValueType());
			}
			case UniformVaribleType::Structure:
			{
				uint32_t maxAlignment = 0;
				const auto pStructureUniform = static_cast<const UniformVariableStructure*>(pUniform);
				for (const auto& [_, member] : pStructureUniform->GetMembers())
					maxAlignment = std::max(maxAlignment, GetStd140BaseAlignment(member.get()));
				return UniformLayoutHelper::AlignOffset(maxAlignment, 16);
			}
			case UniformVaribleType::Array:
			{
				const auto pArrayUniform = static_cast<const UniformVariableArray*>(pUniform);
				const auto& members = pArrayUniform->GetMembers();
				auto firstElementAlignment = members.empty() ? 16 : GetStd140BaseAlignment(members.front().get());
				return UniformLayoutHelper::AlignOffset(firstElementAlignment, 16);
			}
		}

		return 0;
	}

	static void PopulateUniformOffsets(UniformVariable* pUniform, const std::string& prefix,
		uint32_t& currentOffset, std::unordered_map<std::string, uint32_t>& offsetMap, bool isTopLevel = true)
	{
		switch (pUniform->VaribleType())
		{
			case UniformVaribleType::Numeric:
			{
				// Align to the proper boundary before placing the value
				const auto pNumericUniform = static_cast<const UniformVariableNumeric*>(pUniform);
				uint32_t alignment = UniformLayoutHelper::GetStd140BaseAlignment(pNumericUniform->ValueType());
				currentOffset = UniformLayoutHelper::AlignOffset(currentOffset, alignment);
				
				offsetMap[prefix] = currentOffset;
				currentOffset += UniformValue::GetSize(pNumericUniform->ValueType());
				break;
			}
			case UniformVaribleType::Structure:
			{
				const auto pStructureUniform = static_cast<const UniformVariableStructure*>(pUniform);
				// Align structure to its base alignment before starting
				if (isTopLevel)
				{
					uint32_t structAlignment = GetStd140BaseAlignment(pUniform);
					currentOffset = UniformLayoutHelper::AlignOffset(currentOffset, structAlignment);
				}
				
				for (const auto& [name, pChildUniform] : pStructureUniform->GetMembers())
				{
					std::string memberName = prefix + "." + name;
					PopulateUniformOffsets(pChildUniform.get(), memberName, currentOffset, offsetMap, false);
				}
				
				// Align structure size to its base alignment at the end
				if (isTopLevel)
				{
					uint32_t structAlignment = GetStd140BaseAlignment(pUniform);
					currentOffset = UniformLayoutHelper::AlignOffset(currentOffset, structAlignment);
				}
				break;
			}
			case UniformVaribleType::Array:
			{
				const auto pArrayUniform = static_cast<const UniformVariableArray*>(pUniform);
				const auto& members = pArrayUniform->GetMembers();
				
				if (!members.empty())
				{
					// Get the array stride according to std140 rules
					UniformValueType elementType = UniformValueType::Int; // Default, will be overridden
					if (members.front()->VaribleType() == UniformVaribleType::Numeric)
					{
						const auto pFirstElement = static_cast<const UniformVariableNumeric*>(members.front().get());
						elementType = pFirstElement->ValueType();
					}
					
					// Calculate array stride using UniformLayoutHelper
					uint32_t arrayStride = UniformLayoutHelper::GetStd140ArrayStride(elementType);
					
					for (auto i = 0; i < members.size(); ++i)
					{
						std::string elementName = prefix + "[" + std::to_string(i) + "]";
						uint32_t elementOffset = currentOffset + (i * arrayStride);
						offsetMap[elementName] = elementOffset;
					}
					
					// Move past the entire array
					currentOffset += members.size() * arrayStride;
				}
				break;
			}
			default:
				Logger::LogError("Unknown variable type encountered during offset population.");
		}
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