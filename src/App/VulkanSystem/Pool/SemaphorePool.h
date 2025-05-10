#pragma once

#include <queue>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	class SemaphorePool : public NonCopyable, public NonMovable
	{
	public:
		vk::Semaphore Allocate()
		{
			if (_semaphores.empty())
			{

			}
		}

		void Free(vk::Semaphore& semaphore)
		{
			_semaphores.push(semaphore);
		}

		void Clear()
		{
		}

	private:
		vk::Semaphore CreateSemaphore();
		void DestroySemaphore(vk::Semaphore semaphore);

	private:
		std::queue<vk::Semaphore> _semaphores;
	};
} // namespace Ailurus