#pragma once

#include <Ailurus/Math/Vector4.hpp>
#include <Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h>
#include <VulkanContext/RenderPass/VulkanRenderPass.h>

namespace Ailurus
{
    class VulkanRenderPassImGui : public VulkanRenderPass
    {
    public:
        VulkanRenderPassImGui();
        ~VulkanRenderPassImGui() override;

    public:
		RenderPassType GetRenderPassType() override;
		vk::RenderPass GetRenderPass() const override;
		vk::RenderPassBeginInfo GetRenderPassBeginInfo(VulkanFrameBuffer* pTargetFrameBuffer) const override;

    private:
        Vector4f _clearColor = { 0.45f, 0.55f, 0.60f, 1.00f };
        vk::RenderPass _vkRenderPass;
    };

}