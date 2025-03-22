#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Vulkan/RenderPass/RHIRenderPassForward.h"

namespace Ailurus
{
	RenderPass::RenderPass(RenderPassType passType)
	{
		switch (passType)
		{
			case RenderPassType::Forward:
				_pRHI = std::make_unique<RHIRenderPassForward>();
				break;
		}
	}

	RenderPass::~RenderPass() = default;

	RenderPassType RenderPass::GetRenderPassType() const
	{
		return _pRHI->GetRenderPassType();
	}

	const RHIRenderPass* RenderPass::GetRHIRenderPass() const
	{
		return _pRHI.get();
	}

} // namespace Ailurus
