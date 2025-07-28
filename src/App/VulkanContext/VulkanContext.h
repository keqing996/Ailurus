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
	class VulkanVertexLayoutManager;
	class VulkanPipelineManager;
	class VulkanResourceManager;
	class VulkanFlightManager;
	
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
		static auto GetFlightManager() -> VulkanFlightManager*;

		// Swap chain
		static void RebuildSwapChain();

		// Render
		static bool RenderFrame(bool* needRebuild);
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

		// Managers
		inline static std::unique_ptr<VulkanResourceManager> _resourceManager = nullptr;
		inline static std::unique_ptr<VulkanVertexLayoutManager> _vertexLayoutManager = nullptr;
		inline static std::unique_ptr<VulkanPipelineManager> _pipelineManager = nullptr;
		inline static std::unique_ptr<VulkanFlightManager> _flightManager = nullptr;
	};
}