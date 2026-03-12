# Ailurus Post-Processing System

## Scope
Post-process effect chain, render target ping-pong, pipeline factory, and built-in effects (Bloom, ToneMapping).

## Key Files
- `src/Systems/RenderSystem/PostProcess/PostProcessChain.h` / `.cpp`
- `src/Systems/RenderSystem/PostProcess/PostProcessEffect.h`
- `src/Systems/RenderSystem/PostProcess/PostProcessResourcePool.h` / `.cpp`
- `src/Systems/RenderSystem/PostProcess/PostProcessPipelineFactory.h` / `.cpp`
- `src/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h` / `.cpp`
- `src/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h` / `.cpp`

## Architecture

### PostProcessChain (Orchestrator)
Manages ordered effect chain with ping-pong render target execution.

**API:**
- `Init(shaderLibrary, width, height, format)` — Initialize chain
- `Execute(cmdBuffer, input, inputView, output, outputView, extent, descriptorAllocator)` — Run all enabled effects
- `OnResize(width, height, format)` — Rebuild on resize
- `AddEffect<T>(args...)` / `InsertEffect<T>(index, args...)` — Add effects
- `RemoveEffect(name|index)` — Remove effects
- `HasEnabledEffects()` — Query active state

**Execution Model:**
- Single effect: Input → Output directly
- Multiple effects: Ping-pong between intermediate RTs
  - Effect 0: Input → Ping
  - Effect 1: Ping → Pong
  - Effect 2: Pong → Output

### PostProcessEffect (Base Class)
**Virtual Interface:**
```cpp
virtual const std::string& GetName() const = 0;
virtual void Init(resourcePool, factory, width, height, format) = 0;
virtual void Render(cmdBuffer, inputView, outputView, extent, descriptorAllocator) = 0;
virtual void OnResize(resourcePool, factory, width, height, format) = 0;
virtual void Shutdown() = 0;
bool IsEnabled() const;
void SetEnabled(bool);
```

### PostProcessResourcePool
Centralized RT allocation with stable pointers across resizes.

**RTSpec:** `{ float widthScale, heightScale; vk::Format format; }`
**RTHandle:** Stable pointer with `GetImage()`, `GetImageView()`, `GetWidth()`, `GetHeight()`

**API:**
- `RegisterRT(RTSpec)` → `RTHandle*` (stable across resizes)
- `Build(baseWidth, baseHeight)` / `Rebuild(w, h)` — Create/recreate
- `Shutdown()` — Release all

### PostProcessPipelineFactory
**API:**
- `CreatePipeline(PostProcessPipelineDesc)` — No vertex input, no depth
- `GetFullscreenVertexShader()` — Shared fullscreen.vert

**PostProcessPipelineDesc:**
```cpp
struct { vk::Format outputFormat; string fragShaderPath;
         const VulkanDescriptorSetLayout* pDescriptorSetLayout;
         uint32_t pushConstantSize; bool blendEnabled; };
```

### Built-in Effects

#### BloomMipChainEffect
HDR bloom using progressive mip-chain filtering.

**Algorithm:**
1. Bright extract with soft knee threshold → DownMip[0] (50% res)
2. Downsample cascade: 5 levels, 50% reduction each (Kawase/13-tap)
3. Upsample cascade: blend coarser + current (9-tap tent filter)
4. Final composite: original + bloom with intensity blend

**Parameters:**
- `threshold` (default 1.0) — Bloom cutoff
- `softKnee` (default 0.5) — Threshold smoothing
- `bloomIntensity` (default 0.5) — Bloom contribution
- `blendFactor` (default 0.7) — Upsample blend weight

**Resources:** 4 pipelines, 5 down-levels, 4 up-levels, 2 descriptor set layouts.

#### ToneMappingEffect
HDR → LDR conversion.

**Algorithm:** ACES filmic tone mapping (Stephen Hill approximation) + gamma correction.

**Parameters:**
- `exposure` (default 1.0) — Pre-tonemap exposure
- `gamma` (default 2.2) — Output gamma

**Push constants:** `struct { float exposure; float gamma; }`

Must be the final effect in chain (outputs to LDR swapchain format).
