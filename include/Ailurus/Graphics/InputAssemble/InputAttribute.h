#pragma once

#include <vector>
#include <initializer_list>
#include <vulkan/vulkan.hpp>
#include "../../Utility/EnumReflection.h"

namespace Ailurus
{
    REFLECTION_ENUM(AttributeType,
        Vector2,
        Vector3,
        Vector4);

    class InputAttribute
    {
    public:
        explicit InputAttribute(const std::initializer_list<AttributeType>& attributes);
        explicit InputAttribute(const std::vector<AttributeType>& attributes);

        std::vector<vk::VertexInputAttributeDescription> GetAttributeDescription(uint32_t binding = 0) const;
        uint32_t GetStride() const;

    private:
        std::vector<AttributeType> _attribute;
    };

}
