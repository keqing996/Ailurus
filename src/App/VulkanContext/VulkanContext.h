#pragma once

#include "VulkanContext/VulkanPch.h"
#include <vector>
#include <memory>
#include <functional>
#include <optional>
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
	class VulkanSemaphore;
	class VulkanFence;
	
	class VulkanContext : public NonCopyable, public NonMovable
	{
		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;
		using RenderFunction = std::function<void(uint32_t, VulkanCommandBuffer*, VulkanDescriptorAllocator*)>;
		using RecordSecondaryCommandBufferFunction = std::function<void(VulkanCommandBuffer*)>;

	public:
		static void Initialize(const GetWindowInstanceExtension& getWindowRequiredExtension,
			const WindowCreateSurfaceCallback& createSurface,
			bool enableValidation);
		static bool Initialized();
		static void Destroy(const WindowDestroySurfaceCallback& destroySurface);

		// Getter
		static auto GetApiVersion() -> uint32_t;
		static auto GetInstance() -> vk::Instance;
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
		static auto GetFrameBufferManager() -> VulkanFrameBufferManager*;
		static auto GetParallelFrameCount() -> uint32_t;

		// Swap chain
		static void RebuildSwapChain();

		// Render
		static void RecordSecondaryCommandBuffer(const RecordSecondaryCommandBufferFunction& recordFunction);
		static void RenderFrame(bool* needRebuildSwapChain, const RenderFunction& recordCmdBufFunc);
		static void WaitDeviceIdle();

	private:
		// Init functions
		static void PrepareDispatcher();
		static bool CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension, bool enableValidation);
		static bool CreatDebugUtilsMessenger();
		static bool CreateSurface(const WindowCreateSurfaceCallback& createSurface);
		static void ChoosePhysicsDevice();
		static bool CreateLogicalDevice();
		static bool CreateCommandPool();

		// Frame context
		static bool WaitFrameFinish(uint32_t index);

	private:
		struct OnAirInfo
		{
			uint64_t frameCount;
			std::vector<std::unique_ptr<VulkanCommandBuffer>> secondaryCommandBuffers;
		};

		struct FrameContext
		{
			std::optional<OnAirInfo> onAirInfo;
			std::unique_ptr<VulkanCommandBuffer> pRenderingCommandBuffer;
			std::unique_ptr<VulkanDescriptorAllocator> pFrameDescriptorAllocator;
			std::unique_ptr<VulkanSemaphore> imageReadySemaphore;
			std::unique_ptr<VulkanSemaphore> renderFinishSemaphore;
			std::unique_ptr<VulkanFence> renderFinishFence;
		};

	private:
		static uint32_t _parallelFrameCount;
		static uint32_t _apiVersion;

		// Init
		static bool _globalDispatcherLoaded;
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
		static std::unique_ptr<VulkanFrameBufferManager> _frameBufferManager;

		// Flight
		static uint32_t _currentFrameIndex;
		static std::vector<FrameContext> _frameContext;

		// Secondary command buffer
		static std::vector<std::unique_ptr<VulkanCommandBuffer>> _recordedSecondaryCommandBuffers;
		static std::vector<std::unique_ptr<VulkanCommandBuffer>> _secondaryCommandBufferPool;
	};
}