# Ailurus Vulkan Command Buffers

## Scope
Command buffer recording, dynamic rendering, resource lifetime tracking, and draw command abstraction.

## Key Files
- `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.h` / `.cpp`

## Architecture

### VulkanCommandBuffer (RAII)
Wraps `vk::CommandBuffer` with resource lifetime tracking. Allocated from VulkanContext command pool.

**Members:**
- `vk::CommandBuffer _buffer` — Underlying Vulkan handle
- `bool _isPrimary` — Primary vs secondary
- `bool _isRecording` — Recording state
- `std::unordered_set<VulkanResource*> _referencedResources` — Tracked GPU resources

**Resource Tracking Pattern:**
- All buffers/images used during recording get `AddRef()` called
- On recycling/destruction, `RemoveRef()` called on all referenced resources
- Prevents GPU use-after-free (resources can't be deleted while in-flight)

### Recording API
| Method | Purpose |
|--------|---------|
| `Begin()` / `End()` | Start/stop command recording |
| `CopyBuffer(src, dst, size)` | Buffer-to-buffer copy + tracking |
| `BufferMemoryBarrier(...)` | Insert pipeline barrier |
| `ImageMemoryBarrier(image, oldLayout, newLayout, ...)` | Image layout transition |

### Dynamic Rendering API (VK_KHR_dynamic_rendering)
| Method | Purpose |
|--------|---------|
| `BeginRendering(color, depth, resolve, extent, clear, useDepth)` | Begin color+depth pass |
| `BeginDepthOnlyRendering(depthView, extent)` | Begin depth-only pass (shadows) |
| `EndRendering()` | End rendering |
| `SetViewportAndScissor()` / `SetViewportAndScissor(w, h)` | Dynamic viewport/scissor |

### Draw Commands
| Method | Purpose |
|--------|---------|
| `BindPipeline(pipeline)` | Bind graphics pipeline |
| `BindVertexBuffer(vertexBuffer)` | Bind vertex data |
| `BindIndexBuffer(indexBuffer)` | Bind index data |
| `BindDescriptorSet(layout, sets)` | Bind descriptor sets |
| `DrawIndexed(indexCount)` | Indexed draw call |
| `DrawNonIndexed(vertexCount)` | Non-indexed draw call |
| `PushConstantModelMatrix(pipeline, matrix)` | Push model matrix |
| `PushConstantShadowData(pipeline, matrix, cascadeIndex)` | Push shadow data |
| `ExecuteSecondaryCommandBuffer(secondary)` | Execute secondary |
