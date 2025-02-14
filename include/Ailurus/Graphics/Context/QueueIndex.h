#pragma once

#include <optional>

namespace Ailurus
{
    struct QueueIndex
    {
        std::optional<uint32_t> graphicQueueIndex = std::nullopt;
        std::optional<uint32_t> presentQueueIndex = std::nullopt;
    };
}