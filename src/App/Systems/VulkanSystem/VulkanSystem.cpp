#include "VulkanSystem.h"
#include <memory>
#include <optional>
#include <array>
#include "Ailurus/Utility/Logger.h"
#include "Ailurus/Application/Application.h"
#include "VulkanSystem/Descriptor/VulkanDescriptorPool.h"
#include "VulkanSystem/Helper/VulkanHelper.h"
#include "VulkanSystem/FrameContext/FrameContext.h"
#include "VulkanSystem/Resource/VulkanResourceManager.h"
#include "VulkanSystem/Vertex/VulkanVertexLayoutManager.h"
#include "VulkanSystem/Pipeline/VulkanPipelineManager.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Ailurus
{
	VulkanSystem::VulkanSystem(const GetWindowInstanceExtension& getWindowRequiredExtension,
		const WindowCreateSurfaceCallback& createSurface,
		const WindowDestroySurfaceCallback& destroySurface)
		: _destroySurfaceCallback(destroySurface)
		, _resourceManager(nullptr)
		, _vertexLayoutManager(nullptr)
		, _pipelineManager(nullptr)
	{
		PrepareDispatcher();

		VulkanHelper::LogInstanceLayerProperties();
		VulkanHelper::LogInstanceExtensionProperties();

		CreateInstance(getWindowRequiredExtension);
		CreatDebugUtilsMessenger();
		CreateSurface(createSurface);

		VulkanHelper::LogPhysicalCards(_vkInstance);

		ChoosePhysicsDevice();

		VulkanHelper::LogChosenPhysicalCard(_vkPhysicalDevice, _vkSurface);

		if (!CreateLogicalDevice())
			return;

		CreateCommandPool();

		_resourceManager = std::make_unique<VulkanResourceManager>();
		_vertexLayoutManager = std::make_unique<VulkanVertexLayoutManager>();
		_pipelineManager = std::make_unique<VulkanPipelineManager>();

		_initialized = true;
	}

	VulkanSystem::~VulkanSystem()
	{
		if (!_initialized)
			return;

		DestroyDynamicContext();

		_pipelineManager = nullptr;
		_vertexLayoutManager = nullptr;
		_resourceManager = nullptr;

		if (_vkDevice)
		{
			_vkDevice.destroyCommandPool(_vkGraphicCommandPool);
			_vkDevice.destroy();
		}

		if (_vkSurface)
			_destroySurfaceCallback(_vkInstance, _vkSurface);

		if (_vkDebugUtilsMessenger)
			_vkInstance.destroyDebugUtilsMessengerEXT(_vkDebugUtilsMessenger);

		if (_vkInstance)
			_vkInstance.destroy();

		_initialized = false;
	}

	bool VulkanSystem::Initialized() const
	{
		return _initialized;
	}



	uint32_t VulkanSystem::GetCurrentParallelFrameIndex() const
	{
		return _currentParallelFrameIndex;
	}

	void VulkanSystem::CreateSwapChain()
	{
		
	}

	void VulkanSystem::DestroySwapChain()
	{
		
	}

	

	const FrameContext* VulkanSystem::GetFrameContext() const
	{
		return _frameContexts[_currentParallelFrameIndex].get();
	}

	

	FrameContext* VulkanSystem::GetFrameContext()
	{
		return _frameContexts[_currentParallelFrameIndex].get();
	}

	





	void VulkanSystem::CreateDynamicContext()
	{
		_currentParallelFrameIndex = 0;

		// Create swap chain
		CreateSwapChain();

		// Create frame context
		_frameContexts.clear();
		for (auto i = 0; i < PARALLEL_FRAME; i++)
		{
			_frameContexts.push_back(std::make_unique<FrameContext>());
			_frameContexts[i]->EnsureFrameInitialized();
		}
	}

	void VulkanSystem::DestroyDynamicContext()
	{
		if (!_initialized)
			return;

		WaitDeviceIdle();

		// Destroy frame context
		_frameContexts.clear();

		// Destroy the swap chain
		DestroySwapChain();
	}

	void VulkanSystem::WaitDeviceIdle() const
	{
		// Fence all flinging frame -> Make sure tash all command buffers, semaphores
		// and fences are recycled.
		for (auto& pFrameContext : _frameContexts)
			pFrameContext->WaitFinish();

		// Wait gpu end
		_vkDevice.waitIdle();
	}

	bool VulkanSystem::RenderFrame(bool* needRebuild)
	{
		
	}

	
} // namespace Ailurus
