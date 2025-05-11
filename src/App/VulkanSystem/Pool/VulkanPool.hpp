#pragma once

#include <queue>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"

namespace Ailurus
{
	template <typename T>
	class VulkanPool : public NonCopyable, public NonMovable
	{
	public:
		~VulkanPool()
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

		void Free(const T& res)
		{
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
} // namespace Ailurus