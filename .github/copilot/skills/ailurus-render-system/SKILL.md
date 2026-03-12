# Ailurus Render System

## Scope
Central rendering orchestrator: forward rendering pipeline, shadow mapping, lighting, render pass management, and performance statistics.

## Key Files
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h`
- `src/Systems/RenderSystem/RenderSystem.cpp`
- `include/Ailurus/Systems/RenderSystem/RenderPass/RenderPassType.h`
- `include/Ailurus/Systems/RenderSystem/RenderStats.h`

## Architecture

### RenderSystem Class
Central orchestrator managing shader library, render passes, lighting, uniform buffers, post-processing, and statistics.

**Constructor:** `RenderSystem(bool enableImGui, bool enable3D)`
- Initializes ShaderLibrary
- Creates PostProcessChain with Bloom + ToneMapping effects

**Public API:**
| Method | Purpose |
|--------|---------|
| `SetMainCamera(CompCamera*)` | Set rendering camera |
| `GetMainCamera()` | Get current camera |
| `GetShaderLibrary()` | Shader cache access |
| `RenderScene()` | Full frame render pipeline |
| `CheckRebuildSwapChain()` | Handle resize |
| `GraphicsWaitIdle()` | GPU sync |
| `Set/IsVSyncEnabled()` | VSync control |
| `Set/IsMSAAEnabled()` | 4x MSAA toggle |
| `AddCallbackPre/PostSwapChainRebuild()` | Resize callbacks |
| `GetGlobalUniformSet()` | Global uniform schema |
| `GetRenderStats()` | Performance metrics |

### Rendering Pipeline Flow
```
RenderScene()
├─ RenderPrepare() — Reset stats, compute VP matrix, extract frustum
├─ CollectRenderingContext() — Gather meshes with frustum culling
│  └─ Sort forward pass by Material → MaterialInstance → VertexLayout
├─ CollectLights() — Scan for directional(4), point(8), spot(4) lights
├─ CalculateCascadeShadows() — 4-cascade CSM view-projection matrices
├─ UpdateGlobalUniformBuffer() — Upload camera, light, CSM data
├─ UpdateMaterialInstanceUniformBuffer() — Per-material uniforms
├─ RenderShadowPass() — Depth-only per cascade
├─ RenderPass(Forward) — HDR offscreen color+depth
├─ PostProcessChain::Execute() — Bloom → ToneMapping → swapchain
└─ RenderImGuiPass() — UI overlay (if enabled)
```

### RenderPassType Enum
```cpp
REFLECTION_ENUM(RenderPassType, Shadow, Forward, PostProcess, ImGui)
```

### Global Uniform Buffer Layout (std140)
```
viewProjectionMatrix          mat4
cameraPosition                vec3
numDirectionalLights          int
numPointLights                int
numSpotLights                 int
dirLightDirections[4]         vec4[]
dirLightColors[4]             vec4[]  (xyz=color, w=intensity)
pointLightPositions[8]        vec4[]
pointLightColors[8]           vec4[]
pointLightAttenuations[8]     vec4[]
spotLightPositions[4]         vec4[]
spotLightDirections[4]        vec4[]
spotLightColors[4]            vec4[]
spotLightAttenuations[4]      vec4[]
spotLightCutoffs[4]           vec4[]  (x=cos(inner), y=cos(outer))
cascadeViewProjMatrices[4]    mat4[]
cascadeSplitDistances[4]      float[]
```

### RenderIntermediateVariable (Per-Frame Transient)
Stores per-frame computed data: VP matrix, collected meshes per pass, light data packed as Vector4f arrays, frustum, CSM matrices and split distances.

### RenderStats
```cpp
struct RenderStats {
    uint32_t drawCalls, triangleCount, entityCount;
    uint32_t culledEntityCount, meshCount;
    float frameTimeMs;
    void Reset();
};
```

### Cascaded Shadow Mapping
- 4 cascades with practical split scheme (logarithmic + uniform blend)
- Frustum-aligned orthographic light projection per cascade
- Split distances stored for fragment shader cascade selection
- Shadow maps: 2048×2048 depth-only per cascade

### Frustum Culling
- Extract frustum from VP matrix (Gribb-Hartmann method)
- Test entity world AABB against frustum planes
- Culled entities skipped in all render passes
