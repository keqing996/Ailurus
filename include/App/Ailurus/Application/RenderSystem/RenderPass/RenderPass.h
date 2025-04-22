#pragma once

#include <memory>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class RhiRenderPass;

	class RenderPass : public NonCopyable, public NonMovable
	{
	public:
		explicit RenderPass(RenderPassType passType);
		~RenderPass();

	public:
		RenderPassType GetRenderPassType() const;
		RhiRenderPass* GetRHIRenderPass() const;

	private:
		std::unique_ptr<RhiRenderPass> _pRHI = nullptr;
	};
} // namespace Ailurus