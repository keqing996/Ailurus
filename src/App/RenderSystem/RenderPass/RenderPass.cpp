#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/RenderPass/VulkanRenderPassForward.h"

namespace Ailurus
{
	RenderPass::RenderPass(RenderPassType passType)
		: _pVulkanRenderPass(nullptr)
	{
		switch (passType)
		{
			case RenderPassType::Forward:
				_pVulkanRenderPass = std::make_unique<VulkanRenderPassForward>();
				break;
		}
	}

	RenderPass::~RenderPass() = default;

	RenderPassType RenderPass::GetRenderPassType() const
	{
		return _pVulkanRenderPass->GetRenderPassType();
	}

	VulkanRenderPass* RenderPass::GetRHIRenderPass() const
	{
		return _pVulkanRenderPass.get();
	}

} // namespace Ailurus
