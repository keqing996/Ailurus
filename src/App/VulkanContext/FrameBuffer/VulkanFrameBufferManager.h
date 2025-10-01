#pragma once

#include "VulkanContext/VulkanPch.h"
#include <memory>
#include <unordered_map>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
    class VulkanFrameBuffer;
    class VulkanRenderPass;
    
    class VulkanFrameBufferManager : public NonCopyable, public NonMovable
    {
        struct BackBufferKey
        {
            VulkanRenderPass* pRenderPass;
            uint32_t imageIndex;
        };

        struct BackBufferKeyEqual
        {
            bool operator()(const BackBufferKey& lhs, const BackBufferKey& rhs) const;
        };

        struct BackBufferKeyHash
        {
            std::size_t operator()(const BackBufferKey& key) const;
        };

        using BackBufferMap = std::unordered_map<BackBufferKey, std::unique_ptr<VulkanFrameBuffer>, BackBufferKeyHash, BackBufferKeyEqual>;

    public:
        VulkanFrameBufferManager();
        ~VulkanFrameBufferManager();

    public:
        void ClearBackBuffers();
        auto GetBackBuffer(VulkanRenderPass* pRenderPass, uint32_t imageIndex) -> VulkanFrameBuffer*;

    private:
        BackBufferMap _vkBackBuffers;
    };
}