#include "SemaphorePool.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/VulkanSystem.h"

namespace Ailurus
{

	vk::Semaphore SemaphorePool::CreateSemaphore()
	{
		auto device = Application::Get<VulkanSystem>()->GetDevice();

		vk::SemaphoreCreateInfo semaphoreInfo;
		return device.createSemaphore(semaphoreInfo);
	}

	void SemaphorePool::DestroySemaphore(vk::Semaphore semaphore)
	{
		Application::Get<VulkanSystem>()->GetDevice().destroySemaphore(semaphore);
	}
} // namespace Ailurus