#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <Ailurus/Utility/NonCopyable.h>
#include <Ailurus/Utility/NonMovable.h>

namespace Ailurus
{
	class VulkanDescriptorPool;
	class VulkanSwapChain;
	class VulkanVertexLayoutManager;
	class VulkanPipelineManager;
	class VulkanResourceManager;
	
	class VulkanContext : public NonCopyable, public NonMovable
	{
		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

	public:
		static void Initialize(const GetWindowInstanceExtension& getWindowRequiredExtension,
			const WindowCreateSurfaceCallback& createSurface,
			bool enableValidation);

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

		// Pool objects
		static auto AllocateCommandBuffer() -> vk::CommandBuffer;
		static auto AllocateSemaphore() -> vk::Semaphore;
		static auto AllocateFence() -> vk::Fence;
		static auto AllocateDescriptorPool() -> std::unique_ptr<VulkanDescriptorPool>;
		static void FreeCommandBuffer(vk::CommandBuffer commandBuffer, bool destroyImmediately = false);
		static void FreeSemaphore(vk::Semaphore semaphore, bool destroyImmediately = false);
		static void FreeFence(vk::Fence fence, bool destroyImmediately = false);
		static void FreeDescriptorPool(std::unique_ptr<VulkanDescriptorPool>&& pDescriptorPool, bool destroyImmediately = false);

		// Render
		static bool RenderFrame(bool* needRebuild);

	private:
		// Init functions
		static void PrepareDispatcher();
		static void CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension, bool enableValidation);
		static void CreatDebugUtilsMessenger();
		static void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
		static void ChoosePhysicsDevice();
		static bool CreateLogicalDevice();
		static void CreateCommandPool();

		// Pool objects
		static auto CreateCommandBuffer() -> vk::CommandBuffer;
		static auto CreateFence() -> vk::Fence;
		static auto CreateSemaphore() -> vk::Semaphore;
		static void DestroyCommandBuffer(vk::CommandBuffer buffer);
		static void DestroyFence(vk::Fence fence);
		static void DestroySemaphore(vk::Semaphore semaphore);

	private:
		// Init
		inline static bool _initialized = false;

		// Static context
		inline static vk::Instance _vkInstance = nullptr;
		inline static vk::DebugUtilsMessengerEXT _vkDebugUtilsMessenger = nullptr;
		inline static vk::PhysicalDevice _vkPhysicalDevice = nullptr;
		inline static vk::SurfaceKHR _vkSurface = nullptr;
		inline static vk::Device _vkDevice = nullptr;
		inline static uint32_t _presentQueueIndex = 0;
		inline static uint32_t _graphicQueueIndex = 0;
		inline static uint32_t _computeQueueIndex = 0;
		inline static vk::Queue _vkPresentQueue = nullptr;
		inline static vk::Queue _vkGraphicQueue = nullptr;
		inline static vk::Queue _vkComputeQueue = nullptr;
		inline static vk::CommandPool _vkGraphicCommandPool = nullptr;

		// Swap chain
		inline static std::unique_ptr<VulkanSwapChain> _pSwapChain = nullptr;

		// Object pools
		inline static std::vector<vk::CommandBuffer> _queuedCommandBuffers {};
		inline static std::vector<vk::Fence> _queuedFences {};
		inline static std::vector<vk::Semaphore> _queuedSemaphores {};
		inline static std::vector<std::unique_ptr<VulkanDescriptorPool>> _queuedDescriptorPools {};

		// Managers
		inline static std::unique_ptr<VulkanResourceManager> _resourceManager;
		inline static std::unique_ptr<VulkanVertexLayoutManager> _vertexLayoutManager;
		inline static std::unique_ptr<VulkanPipelineManager> _pipelineManager;

		// Frame context
		uint32_t _currentParallelFrameIndex = 0;
		std::vector<std::unique_ptr<FrameContext>> _frameContexts {};
	};
}