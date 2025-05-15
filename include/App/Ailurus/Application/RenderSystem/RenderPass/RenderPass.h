#pragma once

#include <memory>
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h"
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class VulkanRenderPass;

	class RenderPass : public NonCopyable, public NonMovable
	{
	public:
		explicit RenderPass(RenderPassType passType);
		~RenderPass();

	public:
		RenderPassType GetRenderPassType() const;
		VulkanRenderPass* GetRHIRenderPass() const;

	private:
		std::unique_ptr<VulkanRenderPass> _pRHI = nullptr;
	};
} // namespace Ailurus