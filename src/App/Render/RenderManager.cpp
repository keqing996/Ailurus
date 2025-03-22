#include "Ailurus/Application/Render/RenderManager.h"
#include "Ailurus/Application/Material/Material.h"
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/RenderPass/RenderPass.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Vulkan/Airport/Airport.h"
#include "Vulkan/RenderPass/RHIRenderPass.h"

namespace Ailurus
{
	RenderManager::RenderManager()
	{
		_renderPassMap[RenderPassType::Forward] = std::make_unique<RenderPass>(RenderPassType::Forward);
	}

	RenderManager::~RenderManager() = default;

	Material* RenderManager::GetMaterial(const std::string& name) const
	{
		if (const auto itr = _materialMap.find(name); itr != _materialMap.end())
			return itr->second.get();

		return nullptr;
	}

	Material* RenderManager::AddMaterial(const std::string& name)
	{
		_materialMap[name] = std::make_unique<Material>();
		return GetMaterial(name);
	}

	void RenderManager::RenderScene()
	{
		if (_needRebuildSwapChain)
			VulkanContext::RebuildDynamicContext();

		auto pSwapChain = VulkanContext::GetSwapChain();
		auto opFlight = VulkanContext::GetAirport()->WaitNextFlight(pSwapChain, &_needRebuildSwapChain);
		if (!opFlight.has_value())
			return;

		auto flight = opFlight.value();

		// Begin
		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		flight.commandBuffer.begin(beginInfo);

		// Render pass
		RenderForwardPass(&flight);

		// End
		flight.commandBuffer.end();

		// Fire
		VulkanContext::GetAirport()->TakeOff(flight, pSwapChain, &_needRebuildSwapChain);
	}

	void RenderManager::RenderForwardPass(const Flight* pFlight)
	{
		if (_pCurrentRenderPass != nullptr)
		{
			Logger::LogError("Command buffer begin render pass while last render pass not ended");
			return;
		}

		_pCurrentRenderPass = _renderPassMap[RenderPassType::Forward].get();

		pFlight->commandBuffer.beginRenderPass(_pCurrentRenderPass->GetRHIRenderPass()
			->GetRenderPassBeginInfo(*pFlight), {});

		//	flight->DrawObject(_pRenderObj.get());

		pFlight->commandBuffer.endRenderPass();

		_pCurrentRenderPass = nullptr;
	}
} // namespace Ailurus
