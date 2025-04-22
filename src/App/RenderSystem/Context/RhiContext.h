#pragma once

#include <functional>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	struct SwapChainConfig
	{
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		vk::SurfaceFormatKHR surfaceFormat = vk::Format::eUndefined;
		vk::Extent2D extent;
		uint32_t imageCount;
	};

	class RhiContext
	{
		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

		RhiContext() = delete;

	public:
		static bool enableValidation;
		static constexpr uint32_t PARALLEL_FRAME = 2;

	public:
		static bool Init(const GetWindowInstanceExtension& getWindowRequiredExtension, const WindowCreateSurfaceCallback& createSurface);
		static void Destroy(const WindowDestroySurfaceCallback& destroySurface);

		// Static context
		static vk::Device GetDevice();
		static vk::PhysicalDevice GetPhysicalDevice();
		static vk::SurfaceKHR GetSurface();
		static uint32_t GetPresentQueueIndex();
		static uint32_t GetComputeQueueIndex();
		static uint32_t GetGraphicQueueIndex();
		static vk::Queue GetPresentQueue();
		static vk::Queue GetGraphicQueue();
		static vk::Queue GetComputeQueue();
		static vk::CommandPool GetCommandPool();

		// Dynamic context
		static uint32_t CurrentParallelFrameIndex();
		static void RebuildDynamicContext();

		static void CreateSwapChain();
		static void DestroySwapChain();
		static void CreateCommandBuffer();
		static void DestroyCommandBuffer();
		static void CreateSynchronizationObjects();
		static void DestroySynchronizationObjects();

		static const SwapChainConfig& GetSwapChainConfig();
		static const vk::SwapchainKHR& GetSwapChain();
		static const std::vector<vk::ImageView>& GetSwapChainImageViews();

		static uint32_t GetCurrentFrameIndex();
		static const vk::CommandBuffer& GetCurrentFrameCommandBuffer();
		static const vk::Semaphore& GetCurrentFrameImageReadySemaphore();
		static const vk::Semaphore& GetCurrentFrameRenderFinishSemaphore();
		static const vk::Fence& GetCurrentFrameFence();
		static bool WaitNextFrame(bool* needRebuild);
		static bool SubmitThisFrame(bool* needRebuild);

	private:
		static void PrepareDispatcher();
		static void CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension);
		static void CreatDebugUtilsMessenger();
		static void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
		static void ChoosePhysicsDevice();
		static bool CreateLogicalDevice();
		static void CreateCommandPool();
		static void CreateDynamicContext();
		static void DestroyDynamicContext();

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

		// Dynamic context - swap chain
		static SwapChainConfig _swapChainConfig;
		static vk::SwapchainKHR _vkSwapChain;
		static std::vector<vk::Image> _vkSwapChainImages;
		static std::vector<vk::ImageView> _vkSwapChainImageViews;

		// Dynamic context - flight
		static uint32_t _currentParallelFrameIndex;
		static unsigned _currentSwapChainImageIndex;
		static std::vector<vk::CommandBuffer> _vkCommandBuffers;
		static std::vector<vk::Semaphore> _vkImageReadySemaphore;
		static std::vector<vk::Semaphore> _vkFinishRenderSemaphore;
		static std::vector<vk::Fence> _vkFences;

	};
} // namespace Ailurus