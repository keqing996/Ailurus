#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Rhi/RenderPass/RhiRenderPassForward.h"

namespace Ailurus
{
	RenderPass::RenderPass(RenderPassType passType)
	{
		switch (passType)
		{
			case RenderPassType::Forward:
				_pRHI = std::make_unique<RhiRenderPassForward>();
				break;
		}
	}

	RenderPass::~RenderPass() = default;

	RenderPassType RenderPass::GetRenderPassType() const
	{
		return _pRHI->GetRenderPassType();
	}

	RhiRenderPass* RenderPass::GetRHIRenderPass() const
	{
		return _pRHI.get();
	}

} // namespace Ailurus
