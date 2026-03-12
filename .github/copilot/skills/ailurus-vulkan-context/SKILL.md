# Ailurus Vulkan Context & Initialization

## Scope
VulkanContext singleton, Vulkan instance/device creation, frame management, swap chain, function loading, and platform abstraction.

## Key Files
- `src/VulkanContext/VulkanContext.h` / `.cpp` — Central Vulkan singleton
- `src/VulkanContext/VulkanFunctionLoader.h` / `.cpp` — Dynamic Vulkan library loading
- `src/VulkanContext/VulkanPch.h` — Precompiled header for Vulkan includes
- `src/VulkanContext/Platform/VulkanPlatform.h` — Platform abstraction interface
- `src/VulkanContext/Platform/VulkanPlatformApple.cpp` — macOS/iOS specifics
- `src/VulkanContext/Platform/VulkanPlatformWindows.cpp` — Windows specifics
- `src/VulkanContext/Platform/VulkanPlatformAndroid.cpp` — Android specifics
- `src/VulkanContext/Platform/VulkanPlatformLinux.cpp` — Linux specifics

## Architecture

### VulkanContext (Static Singleton)
All members static. Manages global Vulkan state, frame contexts, and manager objects.

**Core Vulkan Objects:**
- `vk::Instance`, `vk::PhysicalDevice`, `vk::Device`
- `vk::SurfaceKHR`, `vk::DebugUtilsMessengerEXT`
- Present/Graphic/Compute queues and indices
- `vk::CommandPool` (graphics)

**Configuration:**
- API version: Vulkan 1.3
- Parallel frame count: 2 (double buffering)
- MSAA: configurable (default 4x)
- VSync: configurable
- Dynamic rendering (VK_KHR_dynamic_rendering, no render passes)

**Manager Objects:**
- `RenderTargetManager` — Depth, MSAA, offscreen, shadow map targets
- `VulkanResourceManager` — GPU resource factory (buffers, images, samplers)
- `VulkanVertexLayoutManager` — Vertex layout cache
- `VulkanPipelineManager` — Pipeline cache

### Frame Context (N-Buffered)
```cpp
struct FrameContext {
    std::optional<OnAirInfo> onAirInfo;           // In-flight frame state
    VulkanCommandBuffer* pRenderingCommandBuffer;  // Primary command buffer
    VulkanDescriptorAllocator* pFrameDescriptorAllocator; // Per-frame descriptors
    VulkanSemaphore* imageReadySemaphore;           // Swapchain acquire signal
    VulkanSemaphore* renderFinishSemaphore;         // Render complete signal
    VulkanFence* renderFinishFence;                 // CPU-GPU sync
};
```

### Initialization Sequence
```
Initialize(extensionFn, surfaceCreateFn, enableValidation)
├─ Load Vulkan library (VulkanFunctionLoader)
├─ Create Instance (extensions + validation)
├─ Create Debug Utils Messenger (if validation)
├─ Create Surface (via SDL callback)
├─ Choose Physical Device
├─ Create Logical Device + Queues
├─ Create Command Pool
├─ Create SwapChain
├─ Create Managers (RenderTarget, Resource, VertexLayout, Pipeline)
└─ Create N FrameContexts (cmd buffer, descriptors, semaphores, fence each)
```

### Frame Rendering Flow
```
RenderFrame(needRebuild, recordFn)
├─ Wait for previous frame fence (CPU blocks)
├─ AcquireNextImage() → imageReadySemaphore
├─ Record primary command buffer:
│  ├─ Execute pooled secondary command buffers
│  └─ Call user recordFn (scene rendering)
├─ Submit: wait imageReadySemaphore, signal renderFinishSemaphore + fence
├─ Present: wait renderFinishSemaphore
└─ Rotate frame index (0..N-1)
```

### Secondary Command Buffer Pool
- `RecordSecondaryCommandBuffer(recordFn)` — Records deferred GPU work
- Buffers pooled and recycled across frames
- Used for staging uploads (vertex/index/image data)

### VulkanFunctionLoader
- Loads platform-specific Vulkan library (libvulkan, MoltenVK, etc.)
- Provides `vk::detail::DynamicLoader` for vulkan-hpp dispatch
- Must be called before SDL initialization

### VulkanPlatform (per-platform)
- `GetVulkanLibraryPath()` — Platform-specific library location
- `GetFallbackPaths()` — Alternative library search paths
- `GetRequiredInstanceExtensions()` — Platform extensions
- `GetInstanceCreateFlags()` — Instance creation flags
- `GetRequiredDeviceExtensions()` — Device extensions
- **macOS**: Requires `VK_KHR_portability_enumeration`, `VK_KHR_portability_subset`

### Public API
| Method | Purpose |
|--------|---------|
| `Initialize()` | Full Vulkan setup |
| `Destroy()` | Full cleanup |
| `GetInstance/Device/PhysicalDevice()` | Core objects |
| `GetSwapChain()` | Swapchain access |
| `GetRenderTargetManager()` | Render targets |
| `GetPipelineManager()` | Pipeline cache |
| `GetResourceManager()` | Resource factory |
| `GetVertexLayoutManager()` | Vertex layouts |
| `RebuildSwapChain()` | Recreate on resize |
| `Set/IsVSyncEnabled()` | VSync control |
| `Set/GetMSAASamples()` | MSAA control |
| `RecordSecondaryCommandBuffer()` | Deferred GPU work |
| `RenderFrame()` | Per-frame rendering |
| `WaitDeviceIdle()` | GPU sync barrier |
