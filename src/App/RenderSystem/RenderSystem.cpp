#include "Ailurus/Application/RenderSystem/RenderSystem.h"
#include "Ailurus/Application/Application.h"
#include "Ailurus/Application/RenderSystem/RenderPass/RenderPass.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{
	RenderSystem::RenderSystem()
	{
		Application::Get<VulkanSystem>()->CreateDynamicContext();

		_pShaderLibrary.reset(new ShaderLibrary());

		CreateIntermidiateVariable();
		BuildRenderPass();
	}

	RenderSystem::~RenderSystem()
	{
		Application::Get<VulkanSystem>()->DestroyDynamicContext();
	}

	void RenderSystem::NeedRecreateSwapChain()
	{
		_needRebuildSwapChain = true;
	}

	ShaderLibrary* RenderSystem::GetShaderLibrary() const
	{
		return _pShaderLibrary.get();
	}

	void RenderSystem::SetMainCamera(CompCamera* pCamera)
	{
		if (pCamera == nullptr)
			return;

		_pMainCamera = pCamera;
	}

	CompCamera* RenderSystem::GetMainCamera() const
	{
		return _pMainCamera;
	}

	void RenderSystem::GraphicsWaitIdle() const
	{
		Application::Get<VulkanSystem>()->GetDevice().waitIdle();
	}

	void RenderSystem::ReBuildSwapChain()
	{
		GraphicsWaitIdle();

		_renderPassMap.clear();

		Application::Get<VulkanSystem>()->DestroyDynamicContext();
		Application::Get<VulkanSystem>()->CreateDynamicContext();

		BuildRenderPass();

		_needRebuildSwapChain = false;
	}

	void RenderSystem::BuildRenderPass()
	{
		_renderPassMap[RenderPassType::Forward] = std::make_unique<RenderPass>(RenderPassType::Forward);
	}

} // namespace Ailurus
