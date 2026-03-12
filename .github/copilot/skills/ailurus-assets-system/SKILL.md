# Ailurus Assets System

## Scope
Asset lifecycle management, reference counting, model loading (Assimp), material system (JSON), texture handling, and mesh GPU upload.

## Key Files
- `include/Ailurus/Systems/AssetsSystem/AssetsSystem.h` — Central registry
- `include/Ailurus/Systems/AssetsSystem/Asset.h` — Base asset class
- `include/Ailurus/Systems/AssetsSystem/AssetRef.h` — Smart reference
- `include/Ailurus/Systems/AssetsSystem/AssetType.h` — Type enum
- `include/Ailurus/Systems/AssetsSystem/Material/Material.h` — Material definition
- `include/Ailurus/Systems/AssetsSystem/Material/MaterialInstance.h` — Material instance
- `include/Ailurus/Systems/AssetsSystem/Mesh/Mesh.h` — GPU mesh geometry
- `include/Ailurus/Systems/AssetsSystem/Model/Model.h` — Model asset (multiple meshes)
- `include/Ailurus/Systems/AssetsSystem/Texture/Texture.h` — Texture asset
- `src/Systems/AssetsSystem/` — All implementations

## Architecture

### Asset Base Class
Non-copyable, non-movable. Has `_assetId` (uint64_t) and `_refCount` (int32_t).

**TypedAsset<AssetType>:** CRTP template providing `GetAssetType()` and `StaticAssetType()`.

### AssetRef<T> (Smart Reference)
RAII reference handle with automatic ref counting.
- Constructor: `AddRef()`, Destructor: `RemoveRef()`
- Copy: increments ref, Move: transfers ownership
- `Get()` → `T*`, `operator->()`, `operator bool()`

### AssetType Enum
`REFLECTION_ENUM(AssetType, Model, Material, MaterialInstance, Texture)`

### AssetsSystem (Central Registry)
**Members:**
- `atomic<uint64_t> _globalAssetIdCounter` — Thread-safe ID generator
- `unordered_map<string, uint64_t> _fileAssetToIdMap` — Path cache
- `unordered_map<uint64_t, unique_ptr<Asset>> _assetsMap` — Asset storage

**API:**
- `LoadModel(path)` → `AssetRef<Model>` (cached by path)
- `LoadMaterial(path)` → `AssetRef<MaterialInstance>` (cached by path)
- `CopyMaterialInstance(ref)` → new `AssetRef<MaterialInstance>`
- `GetAsset<T>(assetId)` → type-safe retrieval
- `GetAssetPath(assetId)` → reverse lookup

### Material System

**Material:** Defines shaders, uniform structures, and textures per render pass.
```cpp
struct MaterialRenderPassInfo {
    StageShaderArray shaders;
    unique_ptr<UniformSet> pUniformSet;
    unordered_map<string, AssetRef<Texture>> textures;
};
unordered_map<RenderPassType, MaterialRenderPassInfo> _renderPassInfoMap;
```
- `HasRenderPass(pass)`, `GetPassShaderArray(pass)`, `GetUniformSet(pass)`, `GetTextures(pass)`

**MaterialInstance:** Runtime instantiation with per-instance uniform data.
- References base Material
- Per-pass `UniformSetMemory` for uniform overrides
- `SetUniformValue(pass, bindingId, "access.path", value)` — Override uniforms
- `GetTextures(pass)` — Delegates to base Material

### Material JSON Format
```json
[{
  "pass": "Forward",
  "shader": [{"stage": "Vertex", "source": "path.vert"}, {"stage": "Fragment", "source": "path.frag"}],
  "uniforms": [{"binding": 0, "shaderStage": ["Fragment"], "name": "material",
    "variable": {"type": "Structure", "members": [
      {"name": "albedo", "variable": {"type": "Numeric", "value": {"type": "Vector3", "x":1,"y":1,"z":1}}}
    ]}}],
  "textures": [{"binding": 1, "uniformVarName": "albedoTexture", "path": "./texture.jpg"}]
}]
```
Variable types: Numeric (Int/Float/Vec2/Vec3/Vec4/Mat4), Structure (named members), Array (homogeneous).

### Model System
`Model : TypedAsset<Model>` — Owns vector of Meshes with merged AABB.

**Loading Pipeline (Assimp):**
1. Cache check by path
2. `Assimp::ReadFile(path, Triangulate | FlipUVs | SortByPType)`
3. Detect vertex layout (Position, Color, Normal, TexCoord, Tangent, Bitangent)
4. Pack interleaved vertex data
5. Create index buffer (UInt16 if possible, else UInt32)
6. Compute per-mesh local AABB
7. Recursively process all nodes (hierarchy flattened)
8. Register asset with path-based caching

### Mesh Class (Not an Asset)
GPU-resident geometry owned by Model.
- `VulkanVertexBuffer`, `VulkanIndexBuffer` (optional), vertex layout ID, local AABB
- Created via staging upload pattern

### Texture System
`Texture : TypedAsset<Texture>` — Wraps `VulkanImage*` + `VulkanSampler*` + binding ID.
- Destructor marks image/sampler for deferred deletion
- Created during material loading from image file paths
