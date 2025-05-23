#pragma once

#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class VulkanDescriptorPool : public NonCopyable, public NonMovable
	{
		struct DescriptorCapacity
		{
			size_t uniformNum;
			size_t samplerNum;

			bool IsEnoughFor(const DescriptorCapacity& requirement) const;
			void Allocate(const DescriptorCapacity& requirement);
		};

		struct PoolCapacity
		{
			size_t setsNum;
			DescriptorCapacity descriptorCapacity;

			bool IsEnoughFor(const DescriptorCapacity& requirement) const;
			void Allocate(const DescriptorCapacity& requirement);
		};

		struct Entry
		{
			vk::DescriptorPool pool;
			PoolCapacity originalCapacity;
			PoolCapacity currentCapacity;

			explicit Entry(PoolCapacity capacity);
			~Entry();
			bool IsEnoughFor(const DescriptorCapacity& requirement) const;
			vk::DescriptorSet Allocate(const DescriptorCapacity& requirement, const vk::DescriptorSetLayout& layout);
		};

	public:
		explicit VulkanDescriptorPool(const DescriptorPoolCapacityConfig& capacity);
		~VulkanDescriptorPool();

	public:
		vk::DescriptorSet AllocateDescriptorSet(const vk::DescriptorSetLayout& layout);

	private:
		uint32_t _capacity;
	};

} // namespace Ailurus