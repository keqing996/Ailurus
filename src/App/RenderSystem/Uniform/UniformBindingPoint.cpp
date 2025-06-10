#include <Ailurus/Application/RenderSystem/Uniform/UniformBindingPoint.h>
#include <Ailurus/Utility/Logger.h>
#include <Ailurus/Utility/String.h>

namespace Ailurus
{
	static uint32_t AlignOffset(uint32_t offset, uint32_t alignment)
	{
		return (offset + alignment - 1) & ~(alignment - 1);
	}

	static uint32_t GetUniformVariableGetSize(UniformValueType type)
	{
		switch (type)
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
			case UniformVaribleType::Structure:
			{
				uint32_t maxAlignment = 0;
				const auto pStructureUniform = static_cast<const UniformVariableStructure*>(pUniform);
				for (const auto& member : pStructureUniform->GetMembers())
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
	}

	static void PopulateUniformOffsets(UniformVariable* pUniform, const std::string& prefix,
		uint32_t& currentOffset, std::unordered_map<std::string, uint32_t>& offsetMap)
	{
		uint32_t baseAlignment = GetStd140BaseAlignment(pUniform);
		currentOffset = AlignOffset(currentOffset, baseAlignment);

		if (!prefix.empty())
			offsetMap[prefix] = currentOffset;

		switch (pUniform->VaribleType())
		{
			case UniformVaribleType::Numeric:
			{
				const auto pNumericUniform = static_cast<const UniformVariableNumeric*>(pUniform);
				currentOffset += GetUniformVariableGetSize(pNumericUniform->ValueType());
				break;
			}
			case UniformVaribleType::Structure:
			{
				const auto pStructureUniform = static_cast<const UniformVariableStructure*>(pUniform);
				for (const auto& member : pStructureUniform->GetMembers())
				{
					std::string memberName = prefix.empty() ? member->GetName() : prefix + "." + member->GetName();
					PopulateUniformOffsets(member.get(), memberName, currentOffset, offsetMap);
				}
				break;
			}
			case UniformVaribleType::Array:
			{
				const auto pArrayUniform = static_cast<const UniformVariableArray*>(pUniform);
				const auto& members = pArrayUniform->GetMembers();
				uint32_t elementAlignment = GetStd140BaseAlignment(members.front().get());
				for (auto i = 0; i < members.size(); ++i)
				{
					std::string elementName = prefix + "[" + std::to_string(i) + "]";
					PopulateUniformOffsets(members[i].get(), elementName, currentOffset, offsetMap);
					currentOffset = AlignOffset(currentOffset, elementAlignment);
				}
				break;
			}
			default:
				Logger::LogError("Unknown variable type encountered during offset population.");
		}
	}

	UniformBindingPoint::UniformBindingPoint(uint32_t bindingPoint, std::unique_ptr<UniformVariable>&& pUniform)
		: _bindingPoint(bindingPoint)
		, _pUniformVariable(std::move(pUniform))
	{
		uint32_t offset = 0;
		PopulateUniformOffsets(_pUniformVariable.get(), "", offset, _accessNameToBufferOffsetMap);
		_totalSize = offset;
	}

	UniformBindingPoint::~UniformBindingPoint() = default;

	void UniformBindingPoint::AddUsingStage(ShaderStage stage)
	{
		_usingStages.push_back(stage);
	}

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
} // namespace Ailurus