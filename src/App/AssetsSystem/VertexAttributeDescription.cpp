#include "Ailurus/Application/AssetsSystem/Mesh/VertexAttributeDescription.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	VertexAttributeDescription::VertexAttributeDescription(const std::initializer_list<AttributeType>& attributes)
	{
		_attribute.insert(_attribute.end(), attributes.begin(), attributes.end());
	}

	VertexAttributeDescription::VertexAttributeDescription(const std::vector<AttributeType>& attributes)
		: _attribute(attributes)
	{
	}

	uint32_t VertexAttributeDescription::GetStride() const
	{
		uint32_t stride = 0;
		for (auto i = 0; i < _attribute.size(); i++)
			stride += SizeOf(_attribute[i]);
		return stride;
	}

	const std::vector<AttributeType>& VertexAttributeDescription::GetAttributes() const
	{
		return _attribute;
	}

	uint32_t VertexAttributeDescription::SizeOf(AttributeType type)
	{
		switch (type)
		{
			case AttributeType::Position:
				return sizeof(float) * 3;
			case AttributeType::Normal:
				return sizeof(float) * 3;
			case AttributeType::TexCoord:
				return sizeof(float) * 2;
			case AttributeType::Tangent:
				return sizeof(float) * 3;
			case AttributeType::Bitangent:
				return sizeof(float) * 3;
			case AttributeType::Color:
				return sizeof(float) * 4;
		}

		Logger::LogError("Fail to get attribute size, attribute type = {}",
			EnumReflection<AttributeType>::ToString(type));
		return 0;
	}

	uint32_t VertexAttributeDescription::SizeOf(IndexBufferFormat type)
	{
		switch (type)
		{
			case IndexBufferFormat::UInt16:
				return sizeof(uint16_t);
			case IndexBufferFormat::UInt32:
				return sizeof(uint32_t);
		}

		Logger::LogError("Fail to get index format size, index format type = {}",
			EnumReflection<IndexBufferFormat>::ToString(type));
		return 0;
	}
} // namespace Ailurus
