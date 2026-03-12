# Ailurus Vulkan Descriptor System

## Scope
Descriptor set layout creation, per-frame descriptor allocation with caching, and descriptor writing.

## Key Files
- `src/VulkanContext/Descriptor/VulkanDescriptorSet.h` — Type alias
- `src/VulkanContext/Descriptor/VulkanDescriptorSetLayout.h` / `.cpp` — Layout definition
- `src/VulkanContext/Descriptor/VulkanDescriptorAllocator.h` / `.cpp` — Pool + cache
- `src/VulkanContext/Descriptor/VulkanDescriptorWriter.h` / `.cpp` — Fluent writer

## Architecture

### VulkanDescriptorSetLayout
Defines descriptor set binding structure. Tracks resource requirements per type.

**Constructors:**
1. From `UniformSet` + optional texture bindings → auto-creates UBO + sampler bindings
2. From raw `vk::DescriptorSetLayoutBinding` vector

**Supported Types:**
- `eUniformBuffer` — Uniform buffers
- `eCombinedImageSampler` — Textures with samplers

### VulkanDescriptorAllocator (Per-Frame)
Each FrameContext has its own allocator. Manages pools and caches descriptors.

**Pool Management:**
- Default capacity: 200 sets, 400 buffers/images per pool
- Auto-allocates new pools when current exhausted
- `ResetPools()` called each frame — resets all pools, clears cache

**Caching:**
- Cache key: `(descriptorSetLayout, bindingHash)`
- `HashBuffers(buffers)` generates cache key from buffer bindings
- Cache hit avoids redundant allocation within same frame
- Tracks statistics: hits, misses, hit rate

**API:**
- `AllocateDescriptorSet(layout, cacheKey)` — Allocate or retrieve cached
- `ResetPools()` — Frame reset
- `LogStatistics()` — Debug output

### VulkanDescriptorWriter (Fluent Builder)
Builds descriptor writes, then applies them in one batch.

**API (chainable):**
```cpp
writer.WriteBuffer(binding, buffer, offset, range)
      .WriteImage(binding, imageView, sampler, layout)
      .WriteDynamicBuffer(binding, buffer, offset, range)
      .WriteStorageBuffer(binding, buffer, offset, range)
      .UpdateSet(descriptorSet);
```

**Members:** Stores deques of `DescriptorBufferInfo` and `DescriptorImageInfo` to keep pointers valid until `UpdateSet()`.

### Design Patterns
- **Per-frame lifecycle:** Pools reset each frame, descriptors are transient
- **Caching within frame:** Same binding config → reuse descriptor set
- **Fluent builder:** Chain writes, then batch-apply
