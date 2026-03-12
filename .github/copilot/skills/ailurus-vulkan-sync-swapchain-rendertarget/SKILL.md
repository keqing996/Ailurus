# Ailurus Vulkan Synchronization, SwapChain & Render Targets

## Scope
CPU-GPU synchronization primitives, swap chain management, and render target allocation.

## Key Files
- `src/VulkanContext/Fence/VulkanFence.h` / `.cpp` — CPU-GPU sync
- `src/VulkanContext/Semaphore/VulkanSemaphore.h` / `.cpp` — GPU-GPU sync
- `src/VulkanContext/SwapChain/VulkanSwapChain.h` / `.cpp` — Display output
- `src/VulkanContext/SwapChain/SwapChainConfig.h` — Swap chain configuration
- `src/VulkanContext/RenderTarget/RenderTarget.h` / `.cpp` — Single attachment
- `src/VulkanContext/RenderTarget/RenderTargetManager.h` / `.cpp` — Target factory

## Architecture

### VulkanFence (CPU-GPU Sync)
RAII wrapper. Used per-frame to sync CPU with GPU completion.
- `VulkanFence(initSignaled)` — Optionally pre-signaled
- `Reset()` — Manual reset
- Per-frame pattern: submit with fence → wait on fence → reset → reuse

### VulkanSemaphore (GPU-GPU Sync)
RAII wrapper for timeline-less semaphores.
- `imageReadySemaphore` — Signaled when swapchain image acquired
- `renderFinishSemaphore` — Signaled when render completes, waited by present

### VulkanSwapChain
Wraps swapchain with automatic image/view creation.

**Configuration (SwapChainConfig):**
```cpp
struct SwapChainConfig {
    vk::PresentModeKHR presentMode;  // eFifo (VSync) or eMailbox/eImmediate
    vk::SurfaceFormatKHR surfaceFormat;
    vk::Extent2D extent;
    uint32_t imageCount;             // 2 or 3
};
```

**Present Mode Selection:**
- VSync on: `eFifo` (guaranteed support)
- VSync off: Try `eMailbox` → fallback `eImmediate`

**API:**
- `AcquireNextImage(semaphore, needRebuild)` → swapchain image index or nullopt

### RenderTarget
Single render attachment: image + memory + view.

**Configuration:**
```cpp
struct RenderTargetConfig {
    uint32_t width, height;
    vk::Format format;
    vk::SampleCountFlagBits samples;
    vk::ImageUsageFlags usage;
    vk::ImageAspectFlags aspectMask;
    bool transient;  // TRANSIENT_ATTACHMENT for MSAA optimization
};
```

### RenderTargetManager
Manages all render attachments for the rendering pipeline.

**Managed Targets:**
| Target | Format | Purpose |
|--------|--------|---------|
| Depth | eD32Sfloat | Main depth buffer |
| MSAA Color | eR16G16B16A16Sfloat | MSAA color (if MSAA>1) |
| MSAA Depth | eD32Sfloat | MSAA depth (if MSAA>1) |
| Offscreen Color | eR16G16B16A16Sfloat | HDR offscreen target |
| Shadow Maps ×4 | eD32Sfloat | CSM cascades (2048×2048) |

**Constants:**
- `SHADOW_MAP_CASCADE_COUNT = 4`
- `SHADOW_MAP_RESOLUTION = 2048`

**Rendering Pipeline:**
1. Forward pass → MSAA color/depth (or offscreen if MSAA=1)
2. MSAA resolve → offscreen color (automatic in dynamic rendering)
3. Shadow passes → shadow map cascades (depth only)
4. Post-process → read offscreen color for effects

**API:** `Rebuild()` on resize, getters for all image views/images.
