#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include "RTSpec.h"
#include "RTHandle.h"

namespace Ailurus
{
    /// @brief Centralized RT management for post-processing.
    /// Effects register RT requirements during Init via RTSpec.
    /// On resize, all registered RTs are rebuilt transparently.
    class PostProcessResourcePool : public NonCopyable, public NonMovable
    {
    public:
        /// @brief Register a new render target. Returns a stable pointer valid for the lifetime of the pool.
        RTHandle* RegisterRT(const RTSpec& spec);

        /// @brief Build all registered render targets using the given base dimensions.
        void Build(uint32_t baseWidth, uint32_t baseHeight);

        /// @brief Rebuild all registered render targets (used on resize).
        void Rebuild(uint32_t baseWidth, uint32_t baseHeight);

        /// @brief Destroy all render targets.
        void Shutdown();

    private:
        void BuildHandle(RTHandle& handle, uint32_t baseWidth, uint32_t baseHeight);

    private:
        std::vector<std::unique_ptr<RTHandle>> _handles;
    };
} // namespace Ailurus
