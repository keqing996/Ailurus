#pragma once

#include <memory>
#include <queue>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanSystem/Descriptor/VulkanDescriptorPool.h"

namespace Ailurus
{
	template <typename T>
	class VulkanObjectPool : public NonCopyable, public NonMovable
	{
	public:
		~VulkanObjectPool()
		{
			Clear();
		}

		T Allocate()
		{
			if (_availableQueue.empty())
				return Create();

			const auto ret = _availableQueue.front();
			_availableQueue.pop();
			return ret;
		}

		void Free(const T& res, bool destroy = false)
		{
			if (destroy)
				Destroy(res);
			else
				_availableQueue.push(res);
		}

		void Clear()
		{
			while (!_availableQueue.empty())
			{
				const auto& buffer = _availableQueue.front();
				Destroy(buffer);
				_availableQueue.pop();
			}
		}

	private:
		static T Create();
		static void Destroy(T res);

	private:
		std::queue<T> _availableQueue;
	};

	template <>
	class VulkanObjectPool<VulkanDescriptorPool>
	{
	public:
		~VulkanObjectPool();

	public:
		std::unique_ptr<VulkanDescriptorPool> Allocate();
		void Free(std::unique_ptr<VulkanDescriptorPool> pool);
		void Clear();

	private:
		std::vector<std::unique_ptr<VulkanDescriptorPool>> _pools;
	};
} // namespace Ailurus