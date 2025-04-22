#pragma once

#include <memory>
#include "Ailurus/Application/Render/RenderPass/RenderPassType.h"
#include "Ailurus/Utility/NonCopyable.h"

namespace Ailurus
{
	class RhiRenderPass;

	class RenderPass : public NonCopyable
	{
	public:
		explicit RenderPass(RenderPassType passType);
		~RenderPass() override;

	public:
		RenderPassType GetRenderPassType() const;
		RhiRenderPass* GetRHIRenderPass() const;

	private:
		std::unique_ptr<RhiRenderPass> _pRHI = nullptr;
	};
} // namespace Ailurus