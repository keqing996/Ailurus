#pragma once

#include <vector>
#include <initializer_list>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(AttributeType,
		Position,
		Normal,
		TexCoord,
		Tangent,
		Bitangent,
		Color);

	REFLECTION_ENUM(IndexBufferFormat,
		UInt16,
		UInt32);

	class VertexAttributeDescription
	{
	public:
		explicit VertexAttributeDescription(const std::initializer_list<AttributeType>& attributes);
		explicit VertexAttributeDescription(const std::vector<AttributeType>& attributes);

	public:
		uint32_t GetStride() const;
		const std::vector<AttributeType>& GetAttributes() const;

		static uint32_t SizeOf(AttributeType type);
		static uint32_t SizeOf(IndexBufferFormat type);

	private:
		std::vector<AttributeType> _attribute;
	};

} // namespace Ailurus
