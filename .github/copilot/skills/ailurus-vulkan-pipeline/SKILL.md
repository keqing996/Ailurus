# Ailurus Vulkan Pipeline System

## Scope
Graphics pipeline creation, caching, vertex layout management, and shader module wrapping.

## Key Files
- `src/VulkanContext/Pipeline/VulkanPipeline.h` / `.cpp` — Graphics pipeline
- `src/VulkanContext/Pipeline/VulkanPipelineEntry.h` / `.cpp` — Cache key
- `src/VulkanContext/Pipeline/VulkanPipelineManager.h` / `.cpp` — Pipeline cache
- `src/VulkanContext/Vertex/VulkanVertexLayout.h` / `.cpp` — Vertex attribute layout
- `src/VulkanContext/Vertex/VulkanVertexLayoutManager.h` / `.cpp` — Layout cache
- `src/VulkanContext/Shader/VulkanShader.h` / `.cpp` — SPIR-V shader module

## Architecture

### VulkanPipeline
Immutable graphics pipeline wrapping `vk::Pipeline` + `vk::PipelineLayout`.

**Two Constructor Variants:**

1. **Standard Scene Pipeline:** Color + depth, vertex input, triangle list, back-face culling
```cpp
VulkanPipeline(colorFormat, depthFormat, shaderArray, vertexLayout, uniformSets, pushConstantSize)
```

2. **Post-Process Pipeline:** No vertex input, full-screen triangle, optional blend
```cpp
VulkanPipeline(colorFormat, shaderArray, descriptorSetLayouts, pushConstantSize, blendEnabled)
```

**Pipeline Configuration:**
- Depth: Always enabled for standard (eLess comparison)
- MSAA: From VulkanContext settings
- Dynamic state: Viewport + scissor
- Rasterization: Fill, clockwise front face, back-face culling
- Push constants: Model matrix (64 bytes) or shadow data (64 + 4 bytes)

### VulkanPipelineEntry (Cache Key)
```cpp
struct VulkanPipelineEntry {
    RenderPassType renderPass;   // Shadow, Forward, etc.
    uint32_t materialAssetId;    // Material asset ID
    uint64_t vertexLayoutId;     // Compressed layout identifier
};
```
Hashed by XOR of all three fields. Used by PipelineManager for lookup.

### VulkanPipelineManager (Cache)
- `GetPipeline(entry)` — Get or create pipeline
- Lazy creation: cache miss → load material shaders → create pipeline → store
- Shadow pass: `colorFormat = eUndefined` (depth only), extra push constant for cascade index

### VulkanVertexLayout
Defines vertex attribute descriptions for pipeline input.

**Supported Attributes (AttributeType enum):**
| Attribute | Format | Size |
|-----------|--------|------|
| Position | vec3 | 12 bytes |
| Normal | vec3 | 12 bytes |
| TexCoord | vec2 | 8 bytes |
| Tangent | vec3 | 12 bytes |
| Bitangent | vec3 | 12 bytes |
| Color | vec4 | 16 bytes |

**API:** `GetStride()`, `GetAttributes()`, `GetVulkanAttributeDescription()`

### VulkanVertexLayoutManager (Dedup Cache)
**Compression:** Up to 16 attributes encoded into uint64_t (4 bits each).
- Same attribute combination = same layout ID
- `CreateLayout(attributes)` → layout ID
- `GetLayout(layoutId)` → `VulkanVertexLayout*`

### VulkanShader
Wraps compiled SPIR-V into `vk::ShaderModule`.
- Constructors: from file path or from memory
- `GeneratePipelineCreateInfo(stage)` — Create pipeline stage info
- `IsValid()` — Validation check
