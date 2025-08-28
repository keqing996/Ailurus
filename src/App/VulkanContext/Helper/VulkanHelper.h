#pragma once

#include <vulkan/vulkan.hpp>
#include <Ailurus/Application/RenderSystem/Shader/ShaderStage.h>
#include <Ailurus/Application/RenderSystem/Vertex/VertexAttributeType.h>
#include <Ailurus/Application/RenderSystem/Vertex/IndexBufferFormat.h>
#include <Ailurus/Application/RenderSystem/Enum/MultiSampling.h>
#include <Ailurus/Application/RenderSystem/Enum/StencilOperation.h>
#include <Ailurus/Application/RenderSystem/FrameBuffer/FrameBufferUsage.h>

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

		static uint32_t CalculateVertexLayoutStride(const std::vector<AttributeType>& attributes);

		static auto ConvertToVkEnum(MultiSamplingType t) -> vk::SampleCountFlagBits;
		static auto ConvertToVkEnum(StencilLoadType t) -> vk::AttachmentLoadOp;
		static auto ConvertToVkEnum(StencilWriteType t) -> vk::AttachmentStoreOp;
		static auto GetFrameBufferInitLayoutForRenderPass(FrameBufferUsage usage) -> vk::ImageLayout;
		static auto GetFrameBufferFinalLayoutForRenderPass(FrameBufferUsage usage) -> vk::ImageLayout;
	};
} // namespace Ailurus