#pragma once

#include <functional>
#include <vulkan/vulkan.hpp>

namespace Ailurus
{
	class SwapChain;
	class Airport;

	class VulkanContext
	{
		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

		VulkanContext() = delete;

	public:
		static bool enableValidation;

	public:
		static bool Init(const GetWindowInstanceExtension& getWindowRequiredExtension, const WindowCreateSurfaceCallback& createSurface);
		static void Destroy(const WindowDestroySurfaceCallback& destroySurface);

		static vk::Device GetDevice();
		static vk::PhysicalDevice GetPhysicalDevice();
		static uint32_t GetPresentQueueIndex();
		static uint32_t GetComputeQueueIndex();
		static uint32_t GetGraphicQueueIndex();
		static vk::Queue GetPresentQueue();
		static vk::Queue GetGraphicQueue();
		static vk::Queue GetComputeQueue();
		static vk::CommandPool GetCommandPool();

		static void RebuildDynamicContext();
		static SwapChain* GetSwapChain();
		static Airport* GetAirport();

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

		static std::unique_ptr<SwapChain> _pSwapChain;
		static std::unique_ptr<Airport> _pAirport;
	};
} // namespace Ailurus