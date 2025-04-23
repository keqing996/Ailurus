#pragma once

#include <optional>
#include <vulkan/vulkan.hpp>
#include "VulkanBuffer.h"
#include "Ailurus/Utility/EnumReflection.h"

namespace Ailurus
{
	REFLECTION_ENUM(CpuBufferUsage,
		TransferSrc)

	REFLECTION_ENUM(GpuBufferUsage,
		Vertex,
		Index,
		Uniform)

	class DataBufferUtil
	{
	public:
		DataBufferUtil() = delete;

	public:
		static std::optional<CpuBuffer> CreateCpuBuffer(vk::DeviceSize size, CpuBufferUsage usage, bool coherentWithGpu = true);
		static std::optional<GpuBuffer> CreateGpuBuffer(vk::DeviceSize size, GpuBufferUsage usage);

		static void DestroyBuffer(CpuBuffer& cpuBuffer);
		static void DestroyBuffer(GpuBuffer& gpuBuffer);

		static void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
	};
} // namespace Ailurus