# Ailurus Renderer Architecture

## Overview

The Ailurus rendering system is a modern, Vulkan-based renderer designed with a layered architecture that separates high-level rendering logic from low-level graphics API management. The architecture emphasizes performance through caching, sorting, and frame parallelism while maintaining flexibility for different rendering scenarios.

## Architecture Layers

The renderer is organized into three distinct layers:

1. **Application Layer** - High-level rendering orchestration and scene management
2. **Vulkan Context Layer** - Mid-level Vulkan abstraction and resource management
3. **System Integration Layer** - Integration with other systems (ImGui, Input, etc.)

```
┌─────────────────────────────────────────────────────┐
│          Application Layer                          │
│  ┌──────────────┐  ┌────────────────────────────┐  │
│  │ RenderSystem │  │ Scene/Material/Asset Mgmt  │  │
│  └──────────────┘  └────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────┐
│        Vulkan Context Layer (VulkanContext)         │
│  ┌──────────┐  ┌──────────┐  ┌─────────────────┐  │
│  │ Pipeline │  │ Resource │  │ CommandBuffer   │  │
│  │ Manager  │  │ Manager  │  │ SwapChain       │  │
│  └──────────┘  └──────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────┐
│              Vulkan API (vk::*)                     │
└─────────────────────────────────────────────────────┘
```

## Core Components

### 1. VulkanContext

**Location**: `src/App/VulkanContext/`

**Role**: Singleton managing global Vulkan state, device lifecycle, and per-frame rendering orchestration.

#### Key Responsibilities

- **Device Management**: Creates and manages Vulkan instance, physical device, logical device, and queues
- **Frame Context Management**: Maintains multiple frame contexts (typically 2-3) for frame parallelism
- **Rendering Orchestration**: Coordinates the main render loop and frame synchronization
- **Resource Lifetime**: Manages all Vulkan resources through specialized managers

#### Frame Context Structure

Each frame has its own context to enable GPU parallelism:

```cpp
struct FrameContext {
    VulkanCommandBuffer* primaryCmdBuffer;            // Main command buffer for frame
    VulkanDescriptorAllocator* frameDescriptorAllocator;  // Per-frame descriptor allocator
    VulkanSemaphore* imageReadySemaphore;             // Signals when swapchain image is ready
    VulkanSemaphore* renderFinishSemaphore;           // Signals when rendering is complete
    VulkanFence* renderFinishFence;                   // CPU-GPU synchronization
};
```

#### Rendering Flow

The `RenderFrame()` method orchestrates the entire frame rendering:

1. **Wait for Frame Completion**: Wait on fence from N-2 frames ago
2. **Acquire Swapchain Image**: Get next image to render to (with semaphore signal)
3. **Command Recording**: 
   - Reset and begin primary command buffer
   - Execute secondary command buffers if any
   - Call RenderSystem's render callback
4. **Submit to GPU**: Submit commands with proper synchronization primitives
5. **Present**: Display the rendered image to the screen

#### Manager Components

- **PipelineManager**: Caches graphics/compute pipelines
- **ResourceManager**: Manages GPU buffers, images, and samplers
- **VertexLayoutManager**: Tracks vertex input layouts
- **RenderTargetManager**: Manages render targets (color/depth/MSAA)
- **SwapChain**: Manages presentation surface

---

### 2. RenderSystem

**Location**: `src/App/Systems/RenderSystem/`

**Role**: High-level rendering orchestration that bridges scene data with Vulkan rendering.

#### Key Responsibilities

- **Camera Management**: Maintains active camera and computes view-projection matrices
- **Shader Management**: Loads and manages shaders through ShaderLibrary
- **Material System**: Manages material definitions and instances
- **Uniform Management**: Updates and manages uniform buffers (global and per-material)
- **Render Pass Execution**: Orchestrates different rendering passes (Forward, Deferred, etc.)

#### Rendering Pipeline

The main rendering sequence in `RenderScene()`:

1. **RenderPrepare()**: 
   - Update camera view-projection matrix
   - Prepare intermediate rendering variables

2. **CollectRenderingContext()**:
   - Iterate all entities with mesh render components
   - Filter by render pass type (Forward, Shadow, etc.)
   - Collect material, mesh, and transform data
   - **Sort meshes** by Material → MaterialInstance → VertexLayout

3. **UpdateGlobalUniformBuffer()**:
   - Update shared uniforms (camera, lighting)
   - Allocate global descriptor set

4. **UpdateMaterialInstanceUniformBuffer()**:
   - Update per-material uniforms
   - Bind material textures
   - Allocate material descriptor sets

5. **RenderPass()**:
   - Begin render pass with appropriate attachments
   - Iterate sorted mesh list
   - Issue draw calls with state changes minimized

#### Mesh Sorting Strategy

Meshes are sorted to minimize GPU state changes:

```
Primary Sort: Material (minimizes pipeline changes)
  └─ Secondary Sort: MaterialInstance (minimizes descriptor set changes)
      └─ Tertiary Sort: VertexLayout (ensures pipeline compatibility)
```

This sorting dramatically reduces the number of expensive state changes during rendering.

---

### 3. Pipeline System

**Location**: `src/App/VulkanContext/Pipeline/`

**Role**: Manages graphics and compute pipeline creation, caching, and state.

#### Pipeline Cache Key

Pipelines are cached using a composite key:

```cpp
struct VulkanPipelineEntry {
    RenderPassType renderPass;        // Forward, Deferred, Shadow, etc.
    uint32_t materialAssetId;         // Material identifier
    uint64_t vertexLayoutId;          // Vertex layout identifier
};
```

This ensures that pipelines are reused when the same combination is encountered, avoiding expensive pipeline creation.

#### Pipeline Creation

A pipeline is created with:
- Shader stages (vertex, fragment, compute)
- Vertex input layout (position, normal, UV, etc.)
- Uniform descriptor set layouts
- Render pass compatibility
- Fixed-function state (blend, depth, culling, etc.)

The `VulkanPipeline` class encapsulates:
- `vk::Pipeline` - The actual Vulkan pipeline object
- `vk::PipelineLayout` - Layout derived from uniform sets
- Type (Graphics or Compute)

---

### 4. Uniform System

**Location**: `src/App/Systems/RenderSystem/Uniform/`

**Role**: Type-safe system for declaring, updating, and binding shader uniforms.

#### Hierarchy

```
UniformSet (set = 0, 1, 2, etc.)
│
├─ UniformBindingPoint (binding = 0, 1, 2, etc.)
│   │
│   ├─ Layout Information
│   │   ├─ Binding index
│   │   ├─ Descriptor type (UBO, SSBO, sampler)
│   │   ├─ Shader stages
│   │   └─ Size and alignment
│   │
│   └─ UniformVariable (tree structure)
│       │
│       ├─ UniformVariableNumeric (int, float, vec2, vec3, vec4, mat4)
│       ├─ UniformVariableStructure (nested struct with members)
│       └─ UniformVariableArray (arrays of variables)
│
└─ VulkanDescriptorSetLayout (Vulkan descriptor layout for this set)
```

#### Memory Management

The `UniformSetMemory` class handles the CPU-side uniform data:

```cpp
class UniformSetMemory {
    UniformValueMap uniformValueMap;      // Stores uniform values
    VulkanUniformBuffer* uniformBuffer;   // GPU buffer
    
    // Set a uniform value (e.g., "material.albedo")
    void SetUniformValue(uint32_t binding, const string& accessor, const UniformValue& value);
    
    // Upload data to GPU and update descriptor set
    void UpdateToDescriptorSet(VulkanCommandBuffer*, VulkanDescriptorSet*);
};
```

#### Data Flow

```
CPU Side:
  Application → SetUniformValue() → UniformValueMap (in-memory storage)

Per Frame:
  TransitionDataToGpu() → Copy to VulkanUniformBuffer (GPU buffer)
                       → Update descriptor set binding
                       
GPU Side:
  Descriptor set bound → Shader access → Uniform data
```

#### Accessor System

Uniforms use a path-based accessor for nested structures:

```glsl
// GLSL Shader
layout(set = 0, binding = 0) uniform MaterialUniforms {
    vec4 albedo;
    float metallic;
    float roughness;
} material;
```

```cpp
// C++ Access
uniformMemory.SetUniformValue(0, "material.albedo", vec4(1.0, 0.5, 0.0, 1.0));
uniformMemory.SetUniformValue(0, "material.metallic", 0.8f);
```

---

### 5. Command Buffer System

**Location**: `src/App/VulkanContext/CommandBuffer/`

**Role**: Records and executes GPU commands.

#### Types

- **Primary Command Buffers**: Main command buffers submitted to queues
- **Secondary Command Buffers**: Can be executed within primary buffers, allows parallel recording

#### Key Operations

```cpp
class VulkanCommandBuffer {
    // Lifecycle
    void Begin();
    void End();
    
    // Rendering
    void BeginRendering(colorView, depthView, resolveView, extent, clearColor);
    void EndRendering();
    
    // Pipeline State
    void BindPipeline(VulkanPipeline* pipeline);
    void BindVertexBuffer(VulkanVertexBuffer* buffer);
    void BindIndexBuffer(VulkanIndexBuffer* buffer);
    void BindDescriptorSet(layout, descriptorSets[], firstSet, setCount);
    
    // Push Constants
    void PushConstantModelMatrix(pipeline, matrix);
    
    // Drawing
    void DrawIndexed(indexCount, instanceCount, firstIndex);
    void DrawNonIndexed(vertexCount, instanceCount, firstVertex);
    
    // Synchronization
    void ImageMemoryBarrier(image, oldLayout, newLayout, srcStage, dstStage);
};
```

#### Dynamic Rendering

The renderer uses Vulkan's dynamic rendering (no render pass objects):

```cpp
BeginRendering(
    colorAttachmentView,       // Color output
    depthAttachmentView,       // Depth testing
    resolveAttachmentView,     // MSAA resolve target (if applicable)
    renderExtent,              // Viewport size
    clearColor                 // Clear value
);
// ... draw commands ...
EndRendering();
```

This provides flexibility in render target configuration without pre-defined render passes.

---

### 6. Resource Management

**Location**: `src/App/VulkanContext/Resource/`

**Role**: Manages lifetime of all GPU resources (buffers, images, samplers).

#### VulkanResourceManager

Central manager for all GPU resources:

```cpp
class VulkanResourceManager {
    // Buffer creation
    VulkanDeviceBuffer* CreateDeviceBuffer(size, usage);    // GPU-only (fast)
    VulkanHostBuffer* CreateHostBuffer(size, usage);        // CPU-GPU coherent
    
    // Image creation
    VulkanImage* CreateImage(width, height, format, usage);
    VulkanSampler* CreateSampler(filter, addressMode);
    
    // Resource destruction (deferred)
    void GarbageCollect();
};
```

#### Buffer Types

```
VulkanDataBuffer (abstract base)
│
├─ VulkanDeviceBuffer (device-local, high performance)
│   │
│   ├─ VulkanVertexBuffer (vertex data)
│   ├─ VulkanIndexBuffer (index data)
│   └─ VulkanUniformBuffer (dynamic uniform data)
│
└─ VulkanHostBuffer (host-visible, CPU-GPU coherent)
    └─ Used for staging and frequent CPU updates
```

#### Memory Management Strategy

- **Device Buffers**: Allocated in GPU VRAM for maximum performance
- **Host Buffers**: Allocated in CPU-accessible memory for dynamic updates
- **Staging**: Large static data is uploaded via staging buffers
- **Garbage Collection**: Deferred destruction ensures resources aren't freed while in use by GPU

---

### 7. Descriptor System

**Location**: `src/App/VulkanContext/Descriptor/`

**Role**: Manages descriptor set allocation, caching, and binding.

#### VulkanDescriptorAllocator

Smart descriptor allocator with automatic caching:

```cpp
class VulkanDescriptorAllocator {
    struct CacheKey {
        vk::DescriptorSetLayout layout;
        size_t bindingHash;  // Hash of buffer contents and types
    };
    
    VulkanDescriptorSet* Allocate(
        VulkanDescriptorSetLayout* layout,
        const vector<VulkanUniformBuffer*>& uniformBuffers,
        const vector<VulkanImage*>& images,
        const vector<VulkanSampler*>& samplers
    );
};
```

#### Caching Strategy

Descriptor sets are cached to avoid redundant allocations:

1. **Layout Match**: Same descriptor set layout
2. **Content Match**: Same bound resources (buffers/images/samplers)
3. **Hash Check**: Fast hash comparison before full comparison

This dramatically reduces descriptor set allocations, especially for repeated materials.

#### Allocation Flow

```
UniformSetMemory
  ↓
VulkanDescriptorSetLayout (defines structure)
  ↓
Check cache: CacheKey(layout, hash(buffers))
  ↓
Cache Hit? → Reuse existing VulkanDescriptorSet
Cache Miss? → Allocate new from pool → Update bindings → Cache it
```

---

### 8. Render Target & MSAA

**Location**: `src/App/VulkanContext/RenderTarget/`

**Role**: Manages render targets for color, depth, and MSAA.

#### Render Target Types

- **Color Targets**: For color output (with optional MSAA)
- **Depth Targets**: For depth testing and shadow mapping
- **Resolve Targets**: For resolving MSAA to single-sample

#### MSAA Implementation

When MSAA is enabled:

```cpp
// With MSAA
BeginRendering(
    msaaColorView,      // Render to MSAA target
    msaaDepthView,      // MSAA depth
    resolveView,        // Resolve to this (swapchain or texture)
    extent,
    clearColor
);
```

When MSAA is disabled:

```cpp
// Without MSAA
BeginRendering(
    swapchainColorView,  // Render directly to swapchain
    depthView,           // Single-sample depth
    nullptr,             // No resolve needed
    extent,
    clearColor
);
```

#### Configuration

```cpp
struct RenderTargetConfig {
    uint32_t width;
    uint32_t height;
    vk::Format format;
    vk::SampleCountFlagBits samples;  // MSAA level (1x, 2x, 4x, 8x)
    vk::ImageUsageFlags usage;
};
```

---

### 9. SwapChain & Presentation

**Location**: `src/App/VulkanContext/SwapChain/`

**Role**: Manages the presentation surface and image acquisition.

#### VulkanSwapChain

Handles the swapchain lifecycle:

```cpp
class VulkanSwapChain {
    // Acquire next image to render to
    optional<uint32_t> AcquireNextImage(
        VulkanSemaphore* signalSemaphore,
        bool* needRebuild
    );
    
    // Present rendered image
    void Present(
        uint32_t imageIndex,
        VulkanSemaphore* waitSemaphore
    );
    
    // Rebuild swapchain (window resize, etc.)
    void Rebuild(newWidth, newHeight);
};
```

#### Image Acquisition

```cpp
auto imageIndex = swapChain->AcquireNextImage(imageReadySemaphore, &needRebuild);

if (needRebuild) {
    // Window resized or swapchain out-of-date
    swapChain->Rebuild(newWidth, newHeight);
    return;
}

// Render to swapChain->GetImageView(imageIndex)
```

#### Synchronization

Critical for proper frame pacing:

- **Image Ready Semaphore**: Signals when swapchain image is available
- **Render Finish Semaphore**: Signals when rendering is complete (for present)
- **Fence**: Ensures CPU doesn't get too far ahead of GPU

```
CPU:  Frame N     Frame N+1   Frame N+2
       │           │           │
GPU:   │  Frame N-1   Frame N    Frame N+1
       │     │          │           │
       Wait  Acquire   Render    Present
       Fence Img       Submit     Queue
```

---

### 10. Vertex System

**Location**: `src/App/VulkanContext/Vertex/`

**Role**: Defines vertex layouts and manages vertex input state.

#### Vertex Layout

Describes the structure of vertex data:

```cpp
struct VertexAttribute {
    uint32_t location;           // Shader location
    vk::Format format;           // Data format (R32G32B32_SFLOAT, etc.)
    uint32_t offset;             // Offset in vertex struct
};

class VertexLayout {
    vector<VertexAttribute> attributes;
    uint32_t stride;             // Size of one vertex
    uint64_t layoutId;           // Unique ID for this layout
};
```

#### Common Vertex Formats

```cpp
// Position Only (shadows)
struct VertexP {
    vec3 position;
};

// Position + Normal + UV (standard PBR)
struct VertexPNU {
    vec3 position;
    vec3 normal;
    vec2 uv;
};

// Position + Normal + UV + Tangent (normal mapping)
struct VertexPNUT {
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec4 tangent;
};
```

#### VertexLayoutManager

Manages and caches vertex layouts:

```cpp
class VertexLayoutManager {
    VertexLayout* GetOrCreateLayout(const vector<VertexAttribute>& attributes);
    VertexLayout* GetLayoutById(uint64_t layoutId);
};
```

Layouts are cached by their attribute configuration to avoid duplicate definitions.

---

### 11. Shader System

**Location**: `src/App/Systems/RenderSystem/Shader/`

**Role**: Loads, compiles, and manages shader modules.

#### ShaderLibrary

Central repository for all shaders:

```cpp
class ShaderLibrary {
    // Load shader from SPIR-V file
    Shader* LoadShader(const string& path, ShaderStage stage);
    
    // Get shader by path
    Shader* GetShader(const string& path);
    
    // Get shaders by stage
    vector<Shader*> GetShadersByStage(ShaderStage stage);
};
```

#### Shader Stages

```cpp
enum class ShaderStage {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    TessellationControl,
    TessellationEvaluation
};
```

#### Usage

```cpp
// Load shaders
auto vertShader = shaderLibrary->LoadShader("shaders/pbr.vert.spv", ShaderStage::Vertex);
auto fragShader = shaderLibrary->LoadShader("shaders/pbr.frag.spv", ShaderStage::Fragment);

// Create pipeline with shaders
pipeline = pipelineManager->GetOrCreatePipeline(
    renderPassType,
    materialId,
    vertexLayoutId,
    {vertShader, fragShader},
    uniformSets
);
```

---

## Rendering Flow: Complete Walkthrough

### Frame Rendering Sequence

```
VulkanContext::RenderFrame()
│
├─ 1. Wait for frame fence (N-2 frames ago)
│      Ensures this frame's resources are free
│
├─ 2. Acquire swapchain image
│      imageIndex = swapChain->AcquireNextImage(imageReadySemaphore)
│
├─ 3. Reset frame descriptor allocator
│      Recycles descriptors from last time this frame was used
│
├─ 4. Begin primary command buffer
│      cmdBuffer->Begin()
│
├─ 5. Execute secondary command buffers (if any)
│      cmdBuffer->ExecuteCommands(secondaryBuffers)
│
├─ 6. Call RenderSystem callback
│      RenderSystem::RenderScene(cmdBuffer, imageIndex)
│      │
│      ├─ A. RenderPrepare()
│      │     └─ Update camera view-projection matrix
│      │
│      ├─ B. CollectRenderingContext()
│      │     ├─ Iterate all entities with mesh render components
│      │     ├─ Filter by render pass (Forward, Shadow, etc.)
│      │     └─ Sort: Material → MaterialInstance → VertexLayout
│      │
│      ├─ C. UpdateGlobalUniformBuffer()
│      │     ├─ Set global uniforms (camera, lights)
│      │     └─ Allocate global descriptor set (cached)
│      │
│      ├─ D. UpdateMaterialInstanceUniformBuffer()
│      │     ├─ For each unique material instance
│      │     ├─ Update material uniforms
│      │     ├─ Bind textures
│      │     └─ Allocate material descriptor set (cached)
│      │
│      └─ E. RenderPass(Forward)
│            │
│            ├─ i. BeginRendering()
│            │     ├─ Set color attachment (swapchain or MSAA)
│            │     ├─ Set depth attachment
│            │     ├─ Set resolve attachment (if MSAA)
│            │     ├─ Clear color/depth
│            │     └─ Set viewport and scissor
│            │
│            ├─ ii. For each RenderingMesh (sorted):
│            │      │
│            │      ├─ If material changed:
│            │      │   └─ Get/Create pipeline (cached by material+layout)
│            │      │
│            │      ├─ If material instance changed:
│            │      │   └─ Bind material descriptor set
│            │      │
│            │      ├─ If vertex layout changed:
│            │      │   ├─ Bind pipeline
│            │      │   └─ Bind global descriptor set
│            │      │
│            │      ├─ Bind vertex buffer (mesh.vertexBuffer)
│            │      ├─ Bind index buffer (mesh.indexBuffer)
│            │      ├─ Push model matrix (push constant)
│            │      └─ DrawIndexed(indexCount)
│            │
│            └─ iii. EndRendering()
│                   └─ Transition image: ATTACHMENT → PRESENT_SRC
│
├─ 7. End primary command buffer
│      cmdBuffer->End()
│
├─ 8. Submit to graphics queue
│      queue.submit(
│          cmdBuffer,
│          waitSemaphore: imageReadySemaphore,      // Wait for image
│          signalSemaphore: renderFinishSemaphore,  // Signal when done
│          fence: renderFinishFence                 // CPU notification
│      )
│
└─ 9. Present to display
       swapChain->Present(imageIndex, renderFinishSemaphore)
```

### State Change Optimization

The sorted mesh list minimizes expensive state changes:

```
Mesh 1: Material A, Instance A1, Layout L1
    → Bind Pipeline (Material A + Layout L1)
    → Bind Global Descriptors
    → Bind Material Instance A1 Descriptors
    → Draw

Mesh 2: Material A, Instance A1, Layout L1
    → (No state change needed!)
    → Draw

Mesh 3: Material A, Instance A2, Layout L1
    → Bind Material Instance A2 Descriptors
    → Draw

Mesh 4: Material A, Instance A2, Layout L2
    → Bind Pipeline (Material A + Layout L2)
    → Bind Global Descriptors
    → Bind Material Instance A2 Descriptors
    → Draw

Mesh 5: Material B, Instance B1, Layout L1
    → Bind Pipeline (Material B + Layout L1)
    → Bind Global Descriptors
    → Bind Material Instance B1 Descriptors
    → Draw
```

This sorting can reduce state changes by 10-100x compared to unsorted rendering.

---

## Design Patterns

### 1. Singleton Pattern

**VulkanContext** is a singleton managing global Vulkan state:

```cpp
class VulkanContext {
    static VulkanContext* Instance();
private:
    static VulkanContext* _instance;
};
```

**Rationale**: Only one Vulkan context should exist per application.

### 2. Manager Pattern

Resource managers centralize creation and caching:

- `PipelineManager`: Caches pipelines
- `ResourceManager`: Manages GPU resources
- `VertexLayoutManager`: Caches vertex layouts
- `RenderTargetManager`: Manages render targets

**Benefits**: 
- Centralized lifecycle management
- Prevents duplicate resource creation
- Simplifies resource destruction

### 3. Object Pool Pattern

Reusable resources to avoid allocation overhead:

- **Descriptor Pools**: Descriptors are allocated from pools and reset each frame
- **Command Buffers**: Secondary command buffers are pooled and reused
- **Frame Contexts**: Fixed number of contexts rotated each frame

### 4. Cache Key Pattern

Composite keys for efficient resource lookup:

```cpp
// Pipeline cache key
struct VulkanPipelineEntry {
    RenderPassType renderPass;
    uint32_t materialAssetId;
    uint64_t vertexLayoutId;
};

// Descriptor cache key
struct CacheKey {
    vk::DescriptorSetLayout layout;
    size_t bindingHash;
};
```

**Benefits**:
- Fast resource lookup (hash table)
- Automatic reuse of identical resources
- Reduced CPU overhead

### 5. Frame Parallelism Pattern

Multiple frames in-flight pattern (double/triple buffering):

```cpp
FrameContext frameContexts[FRAME_COUNT];  // 2 or 3 frames
uint32_t currentFrame = 0;

// Each frame:
auto& ctx = frameContexts[currentFrame];
WaitForFence(ctx.fence);        // Wait for this frame to complete
// ... use ctx resources ...
currentFrame = (currentFrame + 1) % FRAME_COUNT;
```

**Benefits**:
- Hides GPU latency
- CPU and GPU work in parallel
- Improved frame pacing

### 6. Deferred Destruction Pattern

Resources are not immediately destroyed:

```cpp
class VulkanResourceManager {
    void GarbageCollect() {
        // Destroy resources marked for deletion
        // Only after GPU has finished using them
    }
};
```

**Rationale**: Resources may still be in use by the GPU when marked for deletion.

### 7. Visitor Pattern

The rendering system "visits" sorted meshes with minimal state changes:

```cpp
Material* currentMaterial = nullptr;
MaterialInstance* currentInstance = nullptr;

for (const auto& mesh : sortedMeshes) {
    if (mesh.material != currentMaterial) {
        BindPipeline(mesh.material);
        currentMaterial = mesh.material;
    }
    if (mesh.instance != currentInstance) {
        BindDescriptors(mesh.instance);
        currentInstance = mesh.instance;
    }
    Draw(mesh);
}
```

---

## Performance Optimizations

### 1. Pipeline Caching

**Problem**: Pipeline creation is expensive (milliseconds per pipeline)

**Solution**: Hash-based cache using `VulkanPipelineEntry`

**Impact**: Pipeline creation happens once per unique material+layout combination

### 2. Descriptor Caching

**Problem**: Descriptor allocation and updates are costly

**Solution**: Content-based caching with hash comparison

**Impact**: Repeated materials reuse cached descriptor sets

### 3. Mesh Sorting

**Problem**: Random draw order causes many state changes

**Solution**: Sort by Material → MaterialInstance → VertexLayout

**Impact**: Reduces pipeline binds by 10-100x, descriptor binds by 2-10x

### 4. Frame Buffering

**Problem**: GPU stalls waiting for CPU, or vice versa

**Solution**: 2-3 frames in-flight with independent resources

**Impact**: 30-50% higher throughput, smoother frame times

### 5. MSAA Optimization

**Problem**: MSAA increases memory bandwidth significantly

**Solution**: Transient MSAA targets with resolve

**Impact**: Reduced memory usage, automatic resolve to final target

### 6. Dynamic Descriptor Allocation

**Problem**: Descriptor sets can't be reused across frames without complex tracking

**Solution**: Per-frame descriptor allocator, reset each frame

**Impact**: Simple memory management, no fragmentation

### 7. Push Constants

**Problem**: Uniform updates for per-object data (model matrix) are expensive

**Solution**: Push constants for frequently changing small data

**Impact**: Faster per-object updates without descriptor/buffer overhead

---

## System Integration

### ImGui Integration

**Location**: `src/App/Systems/ImGuiSystem/`

The ImGui system integrates with the renderer:

1. **Initialization**: Creates ImGui Vulkan backend with dedicated descriptor pool
2. **Rendering**: Executes after main Forward pass
3. **Resources**: Uses separate command buffer recording
4. **Synchronization**: Renders to the same frame as game content

### Input System Integration

**Location**: `src/App/Systems/InputSystem/`

Input affects rendering through:
- Camera control (view matrix updates)
- Object selection (highlights in rendering)
- UI interaction (ImGui events)

### Scene System Integration

**Location**: `src/App/Systems/SceneSystem/`

Scene entities provide rendering data:

```cpp
struct CompStaticMeshRender {
    Mesh* mesh;                      // Geometry data
    MaterialInstance* material;      // Rendering material
    Transform* transform;            // World position/rotation/scale
};
```

The RenderSystem queries these components each frame for rendering.

### Asset System Integration

**Location**: `src/App/Systems/AssetsSystem/`

Assets loaded and managed:
- **Meshes**: Vertex and index data
- **Materials**: Shader and uniform definitions
- **Textures**: Image data and sampling parameters
- **Shaders**: Compiled SPIR-V modules

---

## Render Pass Types

The renderer supports multiple render pass types:

### Forward Rendering

**Default pass for opaque and transparent objects**

- Single-pass rendering
- Direct lighting calculations in fragment shader
- Outputs directly to swapchain (with optional MSAA)

### Shadow Pass (Future)

**Depth-only rendering for shadow mapping**

- Renders from light's perspective
- Outputs to depth texture
- Used as shadow map in Forward pass

### Deferred Pass (Future)

**Multi-pass rendering with G-buffer**

- Geometry pass: Output position, normal, albedo, etc. to G-buffer
- Lighting pass: Full-screen quad with lighting calculations
- Benefits: Many lights, consistent shading cost

---

## Configuration & Flexibility

### Render Configuration

Key configuration points:

```cpp
struct RenderConfig {
    bool enableMSAA;                 // Enable multi-sample anti-aliasing
    vk::SampleCountFlagBits msaaLevel;  // 1x, 2x, 4x, 8x
    bool enableVSync;                // Enable vertical synchronization
    vk::PresentModeKHR presentMode;  // Immediate, FIFO, Mailbox, FIFO_Relaxed
};
```

### Shader Flexibility

Materials can use different shaders:
- Standard PBR (Physically-Based Rendering)
- Unlit materials
- Custom shaders with arbitrary uniforms

### Vertex Format Flexibility

Vertex layouts are dynamic:
- Position-only for shadows
- Position + Normal + UV for standard rendering
- Extended attributes (tangent, color, etc.) as needed

---

## Memory Management Strategy

### GPU Memory Allocation

- **Device Local**: Used for vertex, index, and texture data (high performance)
- **Host Visible**: Used for staging and dynamic uniform buffers (CPU access)
- **Host Cached**: Read-back operations (rare)

### Allocation Strategy

```
Large Static Data (meshes, textures):
    CPU → Staging Buffer (host-visible)
        → GPU Buffer (device-local) via transfer command
        
Dynamic Data (uniforms):
    CPU → Uniform Buffer (host-visible + coherent)
        → Direct GPU read (no copy needed)
        
Transient Data (MSAA targets):
    GPU-only (lazily allocated)
        → Automatically managed by Vulkan
```

### Garbage Collection

Resources are collected when:
1. Marked for deletion by application
2. GPU has finished using them (fence check)
3. Garbage collection pass runs (periodic)

This prevents use-after-free bugs with in-flight GPU commands.

---

## Error Handling & Validation

### Vulkan Validation Layers

During development:
- Enabled via environment variable or code
- Catches API misuse, synchronization errors, memory issues
- Performance impact: use only in debug builds

### Swapchain Rebuilding

Handles window resize and other surface changes:

```cpp
if (swapchainOutOfDate || windowResized) {
    swapChain->Rebuild(newWidth, newHeight);
    renderTarget->Rebuild(newWidth, newHeight);
    // Recreate resolution-dependent resources
}
```

### Fence Timeouts

Fences have timeouts to detect GPU hangs:

```cpp
if (!fence->Wait(timeout)) {
    // GPU timeout - likely infinite loop or crash
    HandleGpuTimeout();
}
```

---

## Future Enhancements

Potential areas for extension:

1. **Deferred Rendering**: G-buffer based rendering for many lights
2. **Shadow Mapping**: Directional, point, and spot light shadows
3. **Post-Processing**: Bloom, tone mapping, color grading
4. **Compute Shaders**: GPU particles, culling, animation
5. **Indirect Rendering**: Multi-draw indirect for large scenes
6. **Ray Tracing**: Reflections, ambient occlusion, global illumination
7. **Async Compute**: Parallel compute workloads during rendering

---

## Conclusion

The Ailurus renderer is a well-architected, modern Vulkan-based rendering system that balances performance with flexibility. Key design principles include:

- **Layered Architecture**: Clear separation between high-level logic and low-level API
- **Resource Caching**: Extensive use of caching to minimize redundant operations
- **Frame Parallelism**: Multiple frames in-flight for optimal GPU utilization
- **State Optimization**: Sorting and minimizing state changes for maximum throughput
- **Type Safety**: Uniform system provides type-safe shader data access
- **Extensibility**: Designed to support future rendering techniques and optimizations

The architecture demonstrates best practices in modern graphics programming and provides a solid foundation for complex 3D applications.
