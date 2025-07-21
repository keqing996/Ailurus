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

	class VulkanContext : public NonCopyable, public NonMovable
	{
		using GetWindowInstanceExtension = std::function<std::vector<const char*>()>;
		using WindowCreateSurfaceCallback = std::function<vk::SurfaceKHR(const vk::Instance&)>;
		using WindowDestroySurfaceCallback = std::function<void(const vk::Instance&, const vk::SurfaceKHR&)>;

	public:
		~VulkanContext();

	public:

		static constexpr uint32_t PARALLEL_FRAME = 2;

	public:
		bool Initialized() const;

		// Getters

		auto GetSwapChainConfig() const -> const SwapChainConfig&;
		auto GetSwapChain() const -> const vk::SwapchainKHR&;
		auto GetSwapChainImageViews() -> const std::vector<vk::ImageView>&;
		auto GetCurrentParallelFrameIndex() const -> uint32_t;
		auto GetFrameContext() const -> const FrameContext*;
		auto GetFrameContext() -> FrameContext*;
		auto GetResourceManager() const -> class VulkanResourceManager*;
		auto GetVertexLayoutManager() const -> class VulkanVertexLayoutManager*;
		auto GetPipelineManager() const -> class VulkanPipelineManager*;

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
		VulkanContext();


		void CreateSwapChain();
		void DestroySwapChain();



		// Static context - pools


		// Manager
		std::unique_ptr<class VulkanResourceManager> _resourceManager;
		std::unique_ptr<class VulkanVertexLayoutManager> _vertexLayoutManager;
		std::unique_ptr<class VulkanPipelineManager> _pipelineManager;

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