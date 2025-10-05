#include "VulkanResourceManager.h"
#include "Ailurus/Utility/Logger.h"
#include "VulkanContext/Resource/DataBuffer/VulkanDeviceBuffer.h"
#include "VulkanContext/Resource/DataBuffer/VulkanHostBuffer.h"
#include "VulkanContext/Resource/VulkanResource.h"
#include "VulkanContext/Resource/Image/VulkanImage.h"
#include "VulkanContext/Resource/Image/VulkanSampler.h"
#include "VulkanContext/VulkanContext.h"

namespace Ailurus
{
	VulkanResourceManager::~VulkanResourceManager()
	{
	}

	VulkanDeviceBuffer* VulkanResourceManager::CreateDeviceBuffer(vk::DeviceSize size, DeviceBufferUsage usage, const std::string& debugName)
	{
		auto ptr = VulkanDeviceBuffer::Create(size, usage);
		if (ptr == nullptr)
			return nullptr;

		ptr->SetDebugName(debugName);

		VulkanDeviceBuffer* pBufferRaw = static_cast<VulkanDeviceBuffer*>(ptr.get());
		_resources.push_back(std::move(ptr));

		return pBufferRaw;
	}

	VulkanHostBuffer* VulkanResourceManager::CreateHostBuffer(vk::DeviceSize size, HostBufferUsage usage, bool coherentWithGpu, const std::string& debugName)
	{
		auto ptr = VulkanHostBuffer::Create(size, usage, coherentWithGpu);
		if (ptr == nullptr)
			return nullptr;

		ptr->SetDebugName(debugName);

		VulkanHostBuffer* pBufferRaw = static_cast<VulkanHostBuffer*>(ptr.get());
		_resources.push_back(std::move(ptr));

		return pBufferRaw;
	}

	VulkanImage* VulkanResourceManager::CreateImage(const Image& image, const std::string& debugName)
	{
		auto ptr = VulkanImage::Create(image);
		if (ptr == nullptr)
			return nullptr;

		ptr->SetDebugName(debugName);
		
		VulkanImage* pImageRaw = static_cast<VulkanImage*>(ptr.get());
		_resources.push_back(std::move(ptr));

		return pImageRaw;
	}

	VulkanSampler* VulkanResourceManager::CreateSampler(const std::string& debugName)
	{
		auto ptr = VulkanSampler::Create();
		if (ptr == nullptr)
			return nullptr;

		ptr->SetDebugName(debugName);
		
		VulkanSampler* pSamplerRaw = static_cast<VulkanSampler*>(ptr.get());
		_resources.push_back(std::move(ptr));

		return pSamplerRaw;
	}

	void VulkanResourceManager::GarbageCollect()
	{
		// Mark
		static std::vector<uint64_t> needDeletedResourceIndex;
		needDeletedResourceIndex.clear();
		for (auto i = 0; i < _resources.size(); i++)
		{
			if (_resources[i]->IsMarkDeleted() && _resources[i]->GetRefCount() == 0)
				needDeletedResourceIndex.push_back(i);
		}

		// Release
		for (auto index : needDeletedResourceIndex)
			_resources[index] = nullptr;

		// Clean up
		static std::vector<VulkanResourcePtr> resourcesBuffer;
		resourcesBuffer.clear();
		for (auto i = 0; i < _resources.size(); i++)
		{
			if (_resources[i] != nullptr)
				resourcesBuffer.push_back(std::move(_resources[i]));
		}

		std::swap(_resources, resourcesBuffer);
	}

} // namespace Ailurus