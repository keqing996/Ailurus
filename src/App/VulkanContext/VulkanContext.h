#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanDescriptorAllocator;
	class VulkanSwapChain;
	class VulkanCommandBuffer;
	class VulkanVertexLayoutManager;
	class VulkanPipelineManager;
	class VulkanResourceManager;
	class VulkanFlightManager;
	class VulkanFrameBufferManager;
	
	class VulkanContext : public NonCopyable, public NonMovable
	{
		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

	public:
		static void Initialize(const GetWindowInstanceExtension& getWindowRequiredExtension,
			const WindowCreateSurfaceCallback& createSurface,
			bool enableValidation);
		static bool Initialized();

		static void Destroy(const WindowDestroySurfaceCallback& destroySurface);

		// Getter
		static auto GetDevice() -> vk::Device;
		static auto GetPhysicalDevice() -> vk::PhysicalDevice;
		static auto GetSurface() -> vk::SurfaceKHR;
		static auto GetPresentQueueIndex() -> uint32_t;
		static auto GetComputeQueueIndex() -> uint32_t;
		static auto GetGraphicQueueIndex() -> uint32_t;
		static auto GetPresentQueue() -> vk::Queue;
		static auto GetGraphicQueue() -> vk::Queue;
		static auto GetComputeQueue() -> vk::Queue;
		static auto GetCommandPool() -> vk::CommandPool;
		static auto GetSwapChain() -> VulkanSwapChain*;
		static auto GetPipelineManager() -> VulkanPipelineManager*;
		static auto GetResourceManager() -> VulkanResourceManager*;
		static auto GetVertexLayoutManager() -> VulkanVertexLayoutManager*;
		static auto GetFlightManager() -> VulkanFlightManager*;
		static auto GetFrameBufferManager() -> VulkanFrameBufferManager*;

		// Swap chain
		static void RebuildSwapChain();

		// Render
		static bool RenderFrame(bool* needRebuildSwapchain, const std::function<void(VulkanCommandBuffer*, VulkanDescriptorAllocator*)>& recordCmdBufFunc);
		static void WaitDeviceIdle();

	private:
		// Init functions
		static void PrepareDispatcher();
		static void CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension, bool enableValidation);
		static void CreatDebugUtilsMessenger();
		static void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
		static void ChoosePhysicsDevice();
		static bool CreateLogicalDevice();
		static void CreateCommandPool();

	private:
		// Init
		static bool _initialized;

		// Static context
		static vk::Instance _vkInstance;
		static vk::DebugUtilsMessengerEXT _vkDebugUtilsMessenger;
		static vk::PhysicalDevice _vkPhysicalDevice;
		static vk::SurfaceKHR _vkSurface;
		static vk::Device _vkDevice;
		static uint32_t _presentQueueIndex;
		static uint32_t _graphicQueueIndex;
		static uint32_t _computeQueueIndex;
		static vk::Queue _vkPresentQueue;
		static vk::Queue _vkGraphicQueue;
		static vk::Queue _vkComputeQueue;
		static vk::CommandPool _vkGraphicCommandPool;

		// Swap chain
		static std::unique_ptr<VulkanSwapChain> _pSwapChain;

		// Managers
		static std::unique_ptr<VulkanResourceManager> _resourceManager;
		static std::unique_ptr<VulkanVertexLayoutManager> _vertexLayoutManager;
		static std::unique_ptr<VulkanPipelineManager> _pipelineManager;
		static std::unique_ptr<VulkanFlightManager> _flightManager;
		static std::unique_ptr<VulkanFrameBufferManager> _frameBufferManager;
	};
}