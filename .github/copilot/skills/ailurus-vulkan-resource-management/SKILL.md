# Ailurus Vulkan Resource Management

## Scope
GPU resource lifecycle, reference counting, deferred deletion, buffer/image creation, and garbage collection.

## Key Files
- `src/VulkanContext/Resource/VulkanResource.h` / `.cpp` — Base resource with refcount
- `src/VulkanContext/Resource/VulkanResourceManager.h` / `.cpp` — Factory + GC
- `src/VulkanContext/Resource/DataBuffer/VulkanDataBuffer.h` / `.cpp` — Buffer base
- `src/VulkanContext/Resource/DataBuffer/VulkanDeviceBuffer.h` / `.cpp` — GPU-local buffers
- `src/VulkanContext/Resource/DataBuffer/VulkanHostBuffer.h` / `.cpp` — CPU-visible buffers
- `src/VulkanContext/Resource/Image/VulkanImage.h` / `.cpp` — GPU textures
- `src/VulkanContext/Resource/Image/VulkanSampler.h` / `.cpp` — Texture samplers

## Architecture

### VulkanResource (Base Class)
All GPU resources inherit from this. Provides reference counting for safe deferred deletion.

**API:**
- `AddRef(cmdBuffer)` / `RemoveRef(cmdBuffer)` — Track command buffer references
- `GetRefCount()` — Active reference count
- `MarkDelete()` — Mark for deferred deletion
- `IsMarkDeleted()` — Check deletion flag

**Lifecycle:**
1. Created by ResourceManager
2. Used in command buffers → refs tracked
3. `MarkDelete()` when no longer needed
4. `GarbageCollect()` deletes when `markDeleted && refCount==0`

### VulkanResourceManager (Factory + GC)
**API:**
- `CreateDeviceBuffer(size, usage)` → `VulkanDeviceBuffer*`
- `CreateHostBuffer(size, usage, coherent)` → `VulkanHostBuffer*`
- `CreateImage(Image)` → `VulkanImage*`
- `CreateSampler()` → `VulkanSampler*`
- `GarbageCollect()` — Remove safe-to-delete resources

Uses `VulkanResourcePtr` (unique_ptr with custom deleter) for ownership.

### Buffer Types

**VulkanDataBuffer** (base): Buffer + memory allocation helpers.

**VulkanDeviceBuffer** (GPU-local):
- Memory: `eDeviceLocal` — fast GPU access
- Usages: Vertex (`eVertexBuffer|eTransferDst`), Index (`eIndexBuffer|eTransferDst`), Uniform (`eUniformBuffer|eTransferDst`)
- Requires staging from host buffer

**VulkanHostBuffer** (CPU-visible):
- Memory: `eHostVisible` (+ optional `eHostCoherent`)
- Persistently mapped (`void* mappedAddr`)
- Used for staging data before GPU transfer

### Staging Upload Pattern
```
1. Create VulkanDeviceBuffer (GPU)
2. Create VulkanHostBuffer (staging)
3. memcpy data → host buffer mapping
4. Record secondary command: copy host→device + barrier
5. Mark host buffer for deletion (deferred)
```

### Image Resources

**VulkanImage:**
- Members: width, height, format, vk::Image, DeviceMemory, ImageView
- Creation: Image object → staging buffer → copy command → layout transition
- Optimal tiling, eSampled usage

**VulkanSampler:**
- Simple wrapper around `vk::Sampler`
- Inherits VulkanResource for reference tracking

### Design Patterns
- **RAII with custom deleters**: VulkanResourcePtr ensures proper Vulkan cleanup order
- **Deferred deletion**: Resources survive until all referencing command buffers complete
- **Staging pattern**: CPU data → host buffer → GPU buffer (recorded as secondary cmd)
