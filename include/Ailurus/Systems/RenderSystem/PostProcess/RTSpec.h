#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
    struct RTSpec
    {
        float widthScale = 1.0f;
        float heightScale = 1.0f;
        vk::Format format = vk::Format::eR16G16B16A16Sfloat;

        bool operator==(const RTSpec& other) const
        {
            return widthScale == other.widthScale
                && heightScale == other.heightScale
                && format == other.format;
        }
    };
} // namespace Ailurus
