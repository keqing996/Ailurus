# Ailurus Shader & Uniform System

## Scope
Shader loading/caching, uniform buffer layout (std140), uniform value storage, descriptor set schema, and uniform memory management.

## Key Files
- `include/Ailurus/Systems/RenderSystem/Shader/Shader.h` — Shader wrapper
- `include/Ailurus/Systems/RenderSystem/Shader/ShaderLibrary.h` — Shader cache
- `include/Ailurus/Systems/RenderSystem/Shader/ShaderStage.h` — Stage enum
- `include/Ailurus/Systems/RenderSystem/Shader/StageShaderArray.h` — Multi-stage container
- `include/Ailurus/Systems/RenderSystem/Uniform/UniformValue.h` — Runtime value storage
- `include/Ailurus/Systems/RenderSystem/Uniform/UniformVariable.h` — Schema definition
- `include/Ailurus/Systems/RenderSystem/Uniform/UniformBindingPoint.h` — Binding + offset
- `include/Ailurus/Systems/RenderSystem/Uniform/UniformSet.h` — Descriptor set schema
- `include/Ailurus/Systems/RenderSystem/Uniform/UniformSetMemory.h` — GPU data store
- `include/Ailurus/Systems/RenderSystem/Uniform/UniformLayoutHelper.h` — std140 calculator

## Architecture

### Shader System

**Shader:** Lightweight wrapper over VulkanShader.
- `Shader(stage, name)` — Load SPIR-V binary
- `GetStage()` → `ShaderStage::Vertex` or `Fragment`
- `GetImpl()` → `VulkanShader*`

**ShaderStage Enum:** `REFLECTION_ENUM(ShaderStage, Vertex, Fragment)`

**StageShaderArray:** Fixed-size array `[Vertex, Fragment]` with stage-indexed access.

**ShaderLibrary:** Thread-safe shader cache.
- `GetShader(stage, path)` — Load or retrieve cached
- Path resolution via `Path::ResolvePath()`
- Two-level map: `_library[ShaderStage][path]`

### Uniform Value System

**UniformValue:** Type-erased container for GPU data.
```cpp
union Data { int32_t; float; Vector2f; Vector3f; Vector4f; Matrix4x4f; };
enum UniformValueType { Int(4B), Float(4B), Vector2(8B), Vector3(12B), Vector4(16B), Mat4(64B) };
```
- `GetDataPointer()` — Raw pointer for GPU upload
- Typed getters/setters: `GetInt()`, `SetFloat()`, etc.

### Uniform Variable Hierarchy (Schema)

**UniformVariable (base):** `VariableType()` → Numeric | Structure | Array

**UniformVariableNumeric:** Single scalar/vector/matrix.
**UniformVariableStructure:** Named members: `AddMember(name, variable)`.
**UniformVariableArray:** Homogeneous fixed-size elements.

Example schema tree:
```
GlobalUniform (Structure)
├─ viewProjectionMatrix (Numeric: Mat4)
├─ cameraPosition (Numeric: Vec3)
├─ numDirectionalLights (Numeric: Int)
├─ dirLightDirections (Array[4]: Vec4)
└─ ...
```

### UniformBindingPoint
Maps a uniform variable to a Vulkan descriptor set binding with std140 offset calculation.

**Members:**
- `_bindingPoint` — VK descriptor binding ID
- `_usingStages` — Shader stages
- `_totalSize` — Allocated buffer size
- `_accessNameToBufferOffsetMap` — `"path.member[index]" → byte offset`

**Std140 Rules Applied:**
- Scalars: 4-byte alignment
- Vec2: 8-byte; Vec3/Vec4: 16-byte
- Mat4: 16-byte column alignment
- Arrays: stride always ≥ 16 bytes

### UniformSet (Descriptor Set Schema)
```cpp
enum UniformSetUsage { General(set=0), MaterialCustom(set=1) };
```
- `AddBindingPoint(bindingPoint)` — Register binding
- `InitUniformBufferInfo()` — Calculate total UBO size
- `InitDescriptorSetLayout(textureBindings)` — Create VkDescriptorSetLayout
- `GetUniformBufferSize()` — Total allocation needed

### UniformSetMemory (GPU Data Store)
Per-instance uniform data with GPU backing.
- `SetUniformValue(bindingId, "access.path", value)` — Update value
- `UpdateToDescriptorSet(cmdBuffer, descriptorSet, materialInstance)` — Upload to GPU
- Owns `VulkanUniformBuffer` for CPU→GPU transfer

### UniformLayoutHelper (std140 Calculator)
Static utility for layout calculations:
- `AlignOffset(offset, alignment)` — Round up
- `GetStd140BaseAlignment(type)` — Type alignment
- `GetStd140ArrayStride(type)` — Array element stride (≥16)
- `CalculateStructureLayout(types, offsets)` — Multi-member layout
- `CalculateArrayLayout(type, count, offsets)` — Array layout
