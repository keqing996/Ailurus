# Ailurus Vulkan Data Buffers (Vertex, Index, Uniform)

## Scope
High-level buffer wrappers for vertex geometry, index data, and per-frame uniform data with double-buffering.

## Key Files
- `src/VulkanContext/DataBuffer/VulkanVertexBuffer.h` / `.cpp`
- `src/VulkanContext/DataBuffer/VulkanIndexBuffer.h` / `.cpp`
- `src/VulkanContext/DataBuffer/VulkanUniformBuffer.h` / `.cpp`

## Architecture

### VulkanVertexBuffer
Wraps GPU vertex data. Created by staging from CPU memory.

**Members:** `size_t _sizeInBytes`, `VulkanDeviceBuffer* _buffer`

**Creation:** CPU data → host staging → secondary cmd (copy + barrier) → device buffer. Staging buffer marked for deletion.

### VulkanIndexBuffer
Wraps GPU index data with format support.

**Members:**
- `vk::IndexType _indexType` — `eUint16` or `eUint32`
- `size_t _indexCount` — Number of indices
- `VulkanDeviceBuffer* _buffer`

**Creation:** Same staging pattern as vertex buffers.

### VulkanUniformBuffer (Double-Buffered)
Per-frame uniform data with CPU→GPU sync using double-buffering to avoid stalls.

**Members:**
```cpp
struct BufferPair {
    VulkanHostBuffer* cpuBuffer;   // CPU write target
    VulkanDeviceBuffer* gpuBuffer; // GPU read source
};
size_t _bufferSize;
std::optional<BufferPair> _currentBuffer;
std::deque<BufferPair> _backgroundBuffer;
```

**API:**
- `WriteData(offset, UniformValue)` — Write CPU-side data
- `TransitionDataToGpu(cmdBuffer)` — Copy CPU→GPU + barrier
- `GetThisFrameDeviceBuffer()` — GPU buffer for descriptor binding

**Strategy:**
- Current buffer pair for CPU writes
- Background buffers rotate each frame
- Prevents CPU-GPU synchronization stalls
