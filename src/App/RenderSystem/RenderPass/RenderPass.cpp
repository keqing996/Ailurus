#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/RenderPass/VulkanRenderPassForward.h"

namespace Ailurus
{
	RenderPass::RenderPass(RenderPassType passType)
	{
		switch (passType)
		{
			case RenderPassType::Forward:
				_pRHI = std::make_unique<VulkanRenderPassForward>();
				break;
		}
	}

	RenderPass::~RenderPass() = default;

	RenderPassType RenderPass::GetRenderPassType() const
	{
		return _pRHI->GetRenderPassType();
	}

	VulkanRenderPass* RenderPass::GetRHIRenderPass() const
	{
		return _pRHI.get();
	}

} // namespace Ailurus
