#pragma once

#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class VulkanHelper
	{
	public:
		VulkanHelper() = delete;

	public:
		static vk::Bool32 VKAPI_PTR DebugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

		static void LogInstanceLayerProperties();

		static void LogInstanceExtensionProperties();

		static void LogPhysicalCards(vk::Instance vkInstance);

		static void LogChosenPhysicalCard(const vk::PhysicalDevice& vkPhysicalDevice, vk::SurfaceKHR vkSurface);
	};
} // namespace Ailurus