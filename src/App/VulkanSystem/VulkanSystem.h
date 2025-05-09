#pragma once

#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "Ailurus/Utility/NonCopyable.h"
#include "Ailurus/Utility/NonMovable.h"
#include "VulkanSystem/FrameContext/FrameContext.h"

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

		// Static context
		vk::Device GetDevice() const;
		vk::PhysicalDevice GetPhysicalDevice() const;
		vk::SurfaceKHR GetSurface() const;
		uint32_t GetPresentQueueIndex() const;
		uint32_t GetComputeQueueIndex() const;
		uint32_t GetGraphicQueueIndex() const;
		vk::Queue GetPresentQueue() const;
		vk::Queue GetGraphicQueue() const;
		vk::Queue GetComputeQueue() const;
		vk::CommandPool GetCommandPool() const;

		// Dynamic context
		uint32_t CurrentParallelFrameIndex() const;
		void RebuildDynamicContext();

		void CreateSwapChain();
		void DestroySwapChain();

		const SwapChainConfig& GetSwapChainConfig() const;
		const vk::SwapchainKHR& GetSwapChain() const;
		const std::vector<vk::ImageView>& GetSwapChainImageViews();

		uint32_t GetCurrentFrameIndex() const;
		const FrameContext* GetFrameContext() const;
		bool WaitNextFrame(bool* needRebuild);
		bool SubmitThisFrame(bool* needRebuild);

	private:
		VulkanSystem(const GetWindowInstanceExtension& getWindowRequiredExtension,
			const WindowCreateSurfaceCallback& createSurface,
			const WindowDestroySurfaceCallback& destroySurface);

	private:
		FrameContext* GetFrameContext();
		void PrepareDispatcher();
		void CreateInstance(const GetWindowInstanceExtension& getWindowRequiredExtension);
		void CreatDebugUtilsMessenger();
		void CreateSurface(const WindowCreateSurfaceCallback& createSurface);
		void ChoosePhysicsDevice();
		bool CreateLogicalDevice();
		void CreateCommandPool();
		void CreateDynamicContext();
		void DestroyDynamicContext();

	private:
		// Init
		bool _initialized = false;

		// Destroy callback
		WindowDestroySurfaceCallback _destorySurfaceCallback = nullptr;

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

		// Dynamic context - swap chain
		SwapChainConfig _swapChainConfig{};
		vk::SwapchainKHR _vkSwapChain = nullptr;
		std::vector<vk::Image> _vkSwapChainImages{};
		std::vector<vk::ImageView> _vkSwapChainImageViews{};

		// Dynamic context - flight
		uint32_t _currentParallelFrameIndex = 0;
		unsigned _currentSwapChainImageIndex = 0;
		std::vector<std::unique_ptr<FrameContext>> _frameContexts{};
	};
} // namespace Ailurus