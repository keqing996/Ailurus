#include "Ailurus/Application/Render/InputAttribute.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
	InputAttribute::InputAttribute(const std::initializer_list<AttributeType>& attributes)
	{
		_attribute.insert(_attribute.end(), attributes.begin(), attributes.end());
	}

	InputAttribute::InputAttribute(const std::vector<AttributeType>& attributes)
		: _attribute(attributes)
	{
	}

	uint32_t InputAttribute::GetStride() const
	{
		uint32_t stride = 0;
		for (auto i = 0; i < _attribute.size(); i++)
			stride += SizeOf(_attribute[i]);
		return stride;
	}

	const std::vector<AttributeType>& InputAttribute::GetAttributes() const
	{
		return _attribute;
	}

	uint32_t InputAttribute::SizeOf(AttributeType type)
	{
		switch (type)
		{
			case AttributeType::Vector2:
				return sizeof(float) * 2;
			case AttributeType::Vector3:
				return sizeof(float) * 3;
			case AttributeType::Vector4:
				return sizeof(float) * 4;
		}

		Logger::LogError("Fail to get attribute size, attribute type = {}",
			EnumReflection<AttributeType>::ToString(type));
		return 0;
	}
} // namespace Ailurus
