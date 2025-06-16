#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Application/RenderSystem/Shader/ShaderStage.h>
#include <VulkanSystem/Vertex/VertexAttributeType.h>
#include <VulkanSystem/Vertex/IndexBufferFormat.h>

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

        static vk::ShaderStageFlagBits GetShaderStage(ShaderStage stage);

		static uint32_t SizeOf(AttributeType type);

		static uint32_t SizeOf(IndexBufferFormat type);

		static vk::Format GetFormat(AttributeType type);
	};
} // namespace Ailurus