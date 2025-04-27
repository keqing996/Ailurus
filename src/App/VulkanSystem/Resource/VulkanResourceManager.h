#pragma once

#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanResource.h"
#include <concepts>

namespace Ailurus
{
	class VulkanResourceManager : public NonCopyable, public NonMovable
	{
	public:
		template <typename T> requires std::derived_from<T, VulkanResource>
		T* CreateResource();

		void DestroyResource(VulkanResource* pResource);


	private:
		friend class VulkanSystem;

	private:
	};
} // namespace Ailurus