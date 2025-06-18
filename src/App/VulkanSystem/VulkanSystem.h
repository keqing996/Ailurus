#pragma once

#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanSystem/FrameContext/FrameContext.h"
#include "VulkanSystem/Pool/VulkanObjectPool.hpp"

namespace Ailurus
{
	struct SwapChainConfig
	{
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		vk::SurfaceFormatKHR surfaceFormat = vk::Format::eUndefined;
		vk::Extent2D extent;
		uint32_t imageCount;
	};

	class VulkanSystem : public NonCopyable, public NonMovable
	{
		friend class Application;

		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

	public:
		~VulkanSystem();

	public:
		inline static bool enableValidation = true;
		static constexpr uint32_t PARALLEL_FRAME = 2;

	public:
		bool Initialized() const;

		// Getters
		auto GetDevice() const -> vk::Device;
		auto GetPhysicalDevice() const -> vk::PhysicalDevice;
		auto GetSurface() const -> vk::SurfaceKHR;
		auto GetPresentQueueIndex() const -> uint32_t;
		auto GetComputeQueueIndex() const -> uint32_t;
		auto GetGraphicQueueIndex() const -> uint32_t;
		auto GetPresentQueue() const -> vk::Queue;
		auto GetGraphicQueue() const -> vk::Queue;
		auto GetComputeQueue() const -> vk::Queue;
		auto GetCommandPool() const -> vk::CommandPool;
		auto GetSwapChainConfig() const -> const SwapChainConfig&;
		auto GetSwapChain() const -> const vk::SwapchainKHR&;
		auto GetSwapChainImageViews() -> const std::vector<vk::ImageView>&;
		auto GetCurrentParallelFrameIndex() const -> uint32_t;
		auto GetFrameContext() const -> const FrameContext*;
		auto GetFrameContext() -> FrameContext*;
		auto GetResourceManager() const -> class VulkanResourceManager*;
		auto GetVertexLayoutManager() const -> class VulkanVertexLayoutManager*;

		// Pool
		auto AllocateCommandBuffer() -> vk::CommandBuffer;
		auto AllocateSemaphore() -> vk::Semaphore;
		auto AllocateFence() -> vk::Fence;
		auto AllocateDescriptorPool() -> std::unique_ptr<VulkanDescriptorPool>;
		auto FreeCommandBuffer(vk::CommandBuffer commandBuffer) -> void;
		auto FreeSemaphore(vk::Semaphore semaphore) -> void;
		auto FreeFence(vk::Fence fence) -> void;
		auto FreeDescriptorPool(std::unique_ptr<VulkanDescriptorPool>&& pDescriptorPool) -> void;

		// Dynamic context
		void CreateDynamicContext();
		void DestroyDynamicContext();

		// Render
		void WaitDeviceIdle() const;
		bool RenderFrame(bool* needRebuild);

	private:
		VulkanSystem(const GetWindowInstanceExtension& getWindowRequiredExtension,
			const WindowCreateSurfaceCallback& createSurface,
			const WindowDestroySurfaceCallback& destroySurface);

	private:
		void PrepareDispatcher();
		void CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension);
		void CreatDebugUtilsMessenger();
		void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
		void ChoosePhysicsDevice();
		bool CreateLogicalDevice();
		void CreateCommandPool();
		void CreateSwapChain();
		void DestroySwapChain();

	private:
		// Init
		bool _initialized = false;

		// Destroy callback
		WindowDestroySurfaceCallback _destroySurfaceCallback = nullptr;

		// Static context
		vk::Instance _vkInstance = nullptr;
		vk::DebugUtilsMessengerEXT _vkDebugUtilsMessenger = nullptr;
		vk::PhysicalDevice _vkPhysicalDevice = nullptr;
		vk::SurfaceKHR _vkSurface = nullptr;
		vk::Device _vkDevice = nullptr;
		uint32_t _presentQueueIndex = 0;
		uint32_t _graphicQueueIndex = 0;
		uint32_t _computeQueueIndex = 0;
		vk::Queue _vkPresentQueue = nullptr;
		vk::Queue _vkGraphicQueue = nullptr;
		vk::Queue _vkComputeQueue = nullptr;
		vk::CommandPool _vkGraphicCommandPool = nullptr;

		// Static context - pools
		VulkanObjectPool<vk::CommandBuffer> _commandBufferPool{};
		VulkanObjectPool<vk::Fence> _fencePool{};
		VulkanObjectPool<vk::Semaphore> _semaphorePool{};
		VulkanObjectPool<VulkanDescriptorPool> _descriptorPool{};

		// Manager
		std::unique_ptr<class VulkanResourceManager> _resourceManager;
		std::unique_ptr<class VulkanVertexLayoutManager> _vertexLayoutManager;

		// Dynamic context - swap chain
		SwapChainConfig _swapChainConfig{};
		vk::SwapchainKHR _vkSwapChain = nullptr;
		std::vector<vk::Image> _vkSwapChainImages{};
		std::vector<vk::ImageView> _vkSwapChainImageViews{};

		// Dynamic context - rendering frame
		uint32_t _currentParallelFrameIndex = 0;
		std::vector<std::unique_ptr<FrameContext>> _frameContexts{};
	};
} // namespace Ailurus