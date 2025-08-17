#pragma once

#include <deque>
#include <optional>
#include <Ailurus/Application/RenderSystem/Uniform/UniformValue.h>

namespace Ailurus
{
	class VulkanHostBuffer;
	class VulkanDeviceBuffer;
	class VulkanCommandBuffer;

	class VulkanUniformBuffer
	{
		struct BufferPair
		{
			VulkanHostBuffer* cpuBuffer;
			VulkanDeviceBuffer* gpuBuffer;
		};

	public:
		explicit VulkanUniformBuffer(size_t bufferSize);
		~VulkanUniformBuffer();

	public:
		uint32_t GetBufferSize() const;
		void WriteData(uint32_t offset, const UniformValue& value);
		void TransitionDataToGpu(VulkanCommandBuffer* pCommandBuffer);
		VulkanDeviceBuffer* GetThisFrameDeviceBuffer();

	private:
		BufferPair CreateBufferPair() const;
		void EnsureCurrentBufferValid();

	private:
		size_t _bufferSize;
		std::optional<BufferPair> _currentBuffer;
		std::deque<BufferPair> _backgroundBuffer;
	};
} // namespace Ailurus