#pragma once

#include <memory>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanDescriptorSetLayout;

	class DescriptorSetSchema : public NonCopyable, public NonMovable
	{
	public:
		virtual ~DescriptorSetSchema();

		auto GetDescriptorSetLayout() const -> VulkanDescriptorSetLayout*;

	protected:
		std::unique_ptr<VulkanDescriptorSetLayout> _pDescriptorSetLayout;
	};
} // namespace Ailurus
