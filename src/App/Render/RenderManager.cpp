#include "Ailurus/Application/Render/RenderManager.h"
#include "Ailurus/Application/Material/Material.h"
#include "Vulkan/Context/VulkanContext.h"
#include "Vulkan/Airport/Airport.h"

namespace Ailurus
{
	static void RenderForwardPass()
	{
	}

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
		flight->BeginRenderPass(_pForwardPass.get());
		{
			flight->DrawObject(_pRenderObj.get());
		}
		flight->EndRenderPass();

		// End
		flight.commandBuffer.end();

		// Fire
		VulkanContext::GetAirport()->TakeOff(flight, pSwapChain, &_needRebuildSwapChain);
	}
} // namespace Ailurus
