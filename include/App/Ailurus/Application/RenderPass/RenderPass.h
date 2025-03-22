#pragma once

#include <memory>
#include "Ailurus/Application/RenderPass/RenderPassType.h"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
	class RHIRenderPass;

	class RenderPass : public NonCopyable
	{
	public:
		explicit RenderPass(RenderPassType passType);
		~RenderPass() override;

	public:
		RenderPassType GetRenderPassType() const;
		const RHIRenderPass* GetRHIRenderPass() const;

	private:
		std::unique_ptr<RHIRenderPass> _pRHI = nullptr;
	};
} // namespace Ailurus