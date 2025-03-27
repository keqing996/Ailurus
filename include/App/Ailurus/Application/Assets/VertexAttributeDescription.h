#pragma once

#include <vector>
#include <initializer_list>
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(AttributeType,
		Vector2,
		Vector3,
		Vector4);

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

	private:
		std::vector<AttributeType> _attribute;
	};

} // namespace Ailurus
