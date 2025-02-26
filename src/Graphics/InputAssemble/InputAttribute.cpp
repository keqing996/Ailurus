#include "Ailurus/Graphics/InputAssemble/InputAttribute.h"
#include "Ailurus/Utility/Logger.h"

namespace Ailurus
{
    static vk::Format ConvertToVkFormat(AttributeType type)
    {
        switch (type)
        {
            case AttributeType::Vector2:
                return vk::Format::eR32G32Sfloat;
            case AttributeType::Vector3:
                return vk::Format::eR32G32B32Sfloat;
            case AttributeType::Vector4:
                return vk::Format::eR32G32B32A32Sfloat;
        }

        Logger::LogError("Fail to convert attribute type to vk format, attribute type = {}",
            EnumReflection<AttributeType>::ToString(type));
        return vk::Format::eUndefined;
    }

    static uint32_t GetAttributeSize(AttributeType type)
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

    InputAttribute::InputAttribute(const std::initializer_list<AttributeType>& attributes)
    {
        _attribute.insert(_attribute.end(), attributes.begin(), attributes.end());
    }

    InputAttribute::InputAttribute(const std::vector<AttributeType>& attributes)
        : _attribute(attributes)
    {
    }

    std::vector<vk::VertexInputAttributeDescription> InputAttribute::GetAttributeDescription() const
    {
        std::vector<vk::VertexInputAttributeDescription> result;

        uint32_t offset = 0;
        uint32_t binding = 0;
        for (auto i = 0; i < _attribute.size(); i++)
        {
            AttributeType attr = _attribute[i];

            vk::VertexInputAttributeDescription attributeDescriptions;
            attributeDescriptions.setBinding(binding)
                .setLocation(i)
                .setFormat(ConvertToVkFormat(attr))
                .setOffset(offset);

            result.push_back(attributeDescriptions);

            offset += GetAttributeSize(attr);
            binding++;
        }

        return result;
    }

    uint32_t InputAttribute::GetStride() const
    {
        uint32_t stride = 0;
        for (auto i = 0; i < _attribute.size(); i++)
            stride += GetAttributeSize(_attribute[i]);
        return stride;
    }
}
