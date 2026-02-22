#include "Ailurus/Application/RenderSystem/PostProcess/PostProcessChain.h"
#include "VulkanContext/CommandBuffer/VulkanCommandBuffer.h"
#include "Ailurus/Utility/Logger.h"
#include <algorithm>

namespace Ailurus
{
    void PostProcessChain::Init(ShaderLibrary* pShaderLibrary, uint32_t width, uint32_t height, vk::Format format)
    {
        _width = width;
        _height = height;
        _format = format;

        _factory.Init(pShaderLibrary);

        // Register ping-pong intermediate render targets
        RTSpec pingPongSpec;
        pingPongSpec.widthScale = 1.0f;
        pingPongSpec.heightScale = 1.0f;
        pingPongSpec.format = format;

        _pPingRT = _resourcePool.RegisterRT(pingPongSpec);
        _pPongRT = _resourcePool.RegisterRT(pingPongSpec);

        _resourcePool.Build(width, height);

        // Init existing effects (if any were added before Init)
        for (auto& pEffect : _effects)
            pEffect->Init(_resourcePool, _factory, width, height, format);

        _initialized = true;
    }

    void PostProcessChain::Execute(
        VulkanCommandBuffer* pCmdBuffer,
        vk::Image inputImage, vk::ImageView inputImageView,
        vk::Image outputImage, vk::ImageView outputImageView,
        vk::Extent2D extent, VulkanDescriptorAllocator* pDescriptorAllocator)
    {
        // Collect enabled effects
        std::vector<PostProcessEffect*> enabledEffects;
        for (auto& pEffect : _effects)
        {
            if (pEffect->IsEnabled())
                enabledEffects.push_back(pEffect.get());
        }

        if (enabledEffects.empty())
            return;

        const size_t effectCount = enabledEffects.size();

        if (effectCount == 1)
        {
            // Single effect: input → output directly
            enabledEffects[0]->Render(pCmdBuffer, inputImageView, outputImageView, extent, pDescriptorAllocator);
            return;
        }

        // Multiple effects: ping-pong
        vk::ImageView currentInput = inputImageView;

        for (size_t i = 0; i < effectCount; i++)
        {
            const bool isLast = (i == effectCount - 1);

            if (isLast)
            {
                // Last effect writes to final output
                enabledEffects[i]->Render(pCmdBuffer, currentInput, outputImageView, extent, pDescriptorAllocator);
            }
            else
            {
                // Write to intermediate ping/pong RT
                RTHandle* dstRT = (i % 2 == 0) ? _pPingRT : _pPongRT;

                vk::Image dstImage = dstRT->GetImage();
                vk::ImageView dstView = dstRT->GetImageView();

                // Transition intermediate RT to ColorAttachment before rendering
                pCmdBuffer->ImageMemoryBarrier(
                    dstImage,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::AccessFlags{},
                    vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::PipelineStageFlagBits::eTopOfPipe,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput);

                enabledEffects[i]->Render(pCmdBuffer, currentInput, dstView, extent, pDescriptorAllocator);

                // Transition intermediate RT to ShaderReadOnly for next effect
                pCmdBuffer->ImageMemoryBarrier(
                    dstImage,
                    vk::ImageLayout::eColorAttachmentOptimal,
                    vk::ImageLayout::eShaderReadOnlyOptimal,
                    vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::AccessFlagBits::eShaderRead,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eFragmentShader);

                currentInput = dstView;
            }
        }
    }

    void PostProcessChain::OnResize(uint32_t width, uint32_t height, vk::Format format)
    {
        _width = width;
        _height = height;
        _format = format;

        _resourcePool.Rebuild(width, height);

        for (auto& pEffect : _effects)
            pEffect->OnResize(_resourcePool, _factory, width, height, format);
    }

    void PostProcessChain::Shutdown()
    {
        for (auto& pEffect : _effects)
            pEffect->Shutdown();
        _effects.clear();

        _resourcePool.Shutdown();
        _initialized = false;
    }

    void PostProcessChain::RemoveEffect(const std::string& name)
    {
        auto it = std::find_if(_effects.begin(), _effects.end(),
            [&name](const std::unique_ptr<PostProcessEffect>& pEffect) {
                return pEffect->GetName() == name;
            });

        if (it != _effects.end())
        {
            (*it)->Shutdown();
            _effects.erase(it);
        }
    }

    void PostProcessChain::RemoveEffect(size_t index)
    {
        if (index < _effects.size())
        {
            _effects[index]->Shutdown();
            _effects.erase(_effects.begin() + static_cast<ptrdiff_t>(index));
        }
    }

    PostProcessEffect* PostProcessChain::GetEffect(const std::string& name)
    {
        auto it = std::find_if(_effects.begin(), _effects.end(),
            [&name](const std::unique_ptr<PostProcessEffect>& pEffect) {
                return pEffect->GetName() == name;
            });

        return (it != _effects.end()) ? it->get() : nullptr;
    }

    bool PostProcessChain::HasEnabledEffects() const
    {
        return std::any_of(_effects.begin(), _effects.end(),
            [](const std::unique_ptr<PostProcessEffect>& pEffect) {
                return pEffect->IsEnabled();
            });
    }
} // namespace Ailurus
