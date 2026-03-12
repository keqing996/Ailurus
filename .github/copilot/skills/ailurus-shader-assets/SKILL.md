# Ailurus Shader Assets & Build Pipeline

## Scope
GLSL shader sources, SPIR-V compilation pipeline, material JSON format, post-processing shaders, and build/asset utility scripts.

## Key Files

### Shaders
- `example/Graphics/Assets/Shader/pbr.vert` / `pbr.frag` — PBR forward rendering (Cook-Torrance + CSM)
- `example/Graphics/Assets/Shader/shadow.vert` / `shadow.frag` — Shadow map depth-only pass
- `example/Graphics/Assets/Shader/triangle.vert` / `triangle.frag` — Simple textured geometry
- `example/Graphics/Assets/Shader/PostProcess/fullscreen.vert` — Full-screen triangle generation
- `example/Graphics/Assets/Shader/PostProcess/bloom_threshold.frag` — Brightness extraction
- `example/Graphics/Assets/Shader/PostProcess/bloom_downsample.frag` — 13-tap downsample
- `example/Graphics/Assets/Shader/PostProcess/bloom_upsample.frag` — 9-tap tent upsample
- `example/Graphics/Assets/Shader/PostProcess/bloom_composite.frag` — Additive bloom composite
- `example/Graphics/Assets/Shader/PostProcess/bloom_kawase_down.frag` — Dual Kawase downsample
- `example/Graphics/Assets/Shader/PostProcess/bloom_kawase_up.frag` — Dual Kawase upsample
- `example/Graphics/Assets/Shader/PostProcess/tonemapping.frag` — ACES tonemapping + gamma

### Materials
- `example/Graphics/Assets/Material/PBRMaterial.json` — PBR material definition
- `example/Graphics/Assets/Material/DefaultMaterial.json` — Simple textured material

### Build Scripts
- `example/Graphics/Scripts/shader_gen.py` — GLSL → SPIR-V compilation
- `example/Graphics/Scripts/assets_copy.py` — Asset staging to build output
- `example/Graphics/Scripts/fix_icd_paths.py` — macOS Vulkan ICD path fixing
- `example/Graphics/CMakeLists.txt` — Build integration & post-build commands

## Shader Architecture

All shaders are **GLSL 450**. Compiled to SPIR-V via `glslc`.

### Descriptor Set Layout Convention
- **Set 0**: Global uniforms (shared across all objects per frame)
- **Set 1**: Per-material uniforms and textures
- **Push Constants**: Per-object data (model matrix, cascade index)

### GlobalUniform (Set 0, Binding 0, std140)
```glsl
layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    int numDirectionalLights;      // max 4
    int numPointLights;            // max 8
    int numSpotLights;             // max 4
    vec4 dirLightDirections[4];
    vec4 dirLightColors[4];
    vec4 pointLightPositions[8];
    vec4 pointLightColors[8];
    vec4 pointLightAttenuations[8];
    vec4 spotLightPositions[4];
    vec4 spotLightDirections[4];
    vec4 spotLightColors[4];
    vec4 spotLightAttenuations[4];
    vec4 spotLightCutoffs[4];
    mat4 cascadeViewProjMatrices[4];
    float cascadeSplitDistances[4];
};
```

### PBR Shader (pbr.vert / pbr.frag)
**Vertex:** Push constant `mat4 modelMatrix`. Attributes: position (loc 0), normal (loc 1), UV (loc 2). Outputs world pos, transformed normal (transpose-inverse), UV.

**Fragment:** Cook-Torrance BRDF:
- **NDF**: GGX/Trowbridge-Reitz (`DistributionGGX`)
- **Geometry**: Smith's method with Schlick-GGX (`GeometrySmith`)
- **Fresnel**: Schlick approximation (`fresnelSchlick`)
- Diffuse: Lambert (energy-conserving with metallic ratio)
- Processes directional, point, spot lights
- **CSM Shadows**: Cascade selection by view-depth, 3×3 PCF sampling
- Material binding (set 1, binding 0): `albedo (vec3)`, `metallic`, `roughness`, `ao`
- Texture (set 1, binding 1): `albedoTexture`
- Shadow maps (set 0, bindings 1–4): `shadowMap0–3`

### Shadow Shader (shadow.vert / shadow.frag)
Push constant: `mat4 modelMatrix`, `uint cascadeIndex`. Depth-only output. Empty fragment shader.

### Triangle Shader (triangle.vert / triangle.frag)
Simple MVP transform + texture sample. Material: `vec3 u_SelfColor`. Texture: `mainTexture`.

### Fullscreen Vertex Shader (fullscreen.vert)
Generates full-screen triangle from `gl_VertexIndex` bit manipulation. No vertex buffer needed. Output: `vec2 fragUV` in [0,1].

### Bloom Pipeline

**Stage 1 — Threshold** (`bloom_threshold.frag`):
Push constants: `float threshold`, `float softKnee`.
Brightness = `max(R, max(G, B))`. Soft knee for smooth transition.

**Stage 2 — Downsample** (`bloom_downsample.frag`):
Push constants: `vec2 texelSize`. 13-tap weighted filter (Jimenez 2014 / CoD:AW).

**Stage 3 — Upsample** (`bloom_upsample.frag`):
Push constants: `vec2 texelSize`, `float blendFactor`. 9-tap tent filter. Blends with current mip level.

**Stage 4 — Composite** (`bloom_composite.frag`):
Push constants: `float bloomIntensity`. Additive: `original + bloom * intensity`.

**Alternative Kawase Filters:**
- `bloom_kawase_down.frag`: Center + 4 diagonal taps, weight 4:1:1:1:1, /8
- `bloom_kawase_up.frag`: 4 diagonal + 4 axis taps, weight 1:1:1:1:2:2:2:2, /12

### Tonemapping (`tonemapping.frag`)
Push constants: `float exposure`, `float gamma`.
1. Apply exposure multiplication
2. ACES filmic tonemapping (Stephen Hill fit): `(x*(2.51x+0.03))/(x*(2.43x+0.59)+0.14)`
3. Clamp [0,1]
4. Gamma correction: `pow(mapped, 1/gamma)`

## Material JSON Format

Material files define render pass configurations as a JSON array:
```json
[{
  "pass": "Forward",
  "shader": [
    {"stage": "Vertex", "source": "Assets/Shader/pbr.vert"},
    {"stage": "Fragment", "source": "Assets/Shader/pbr.frag"}
  ],
  "uniforms": [{
    "binding": 0,
    "shaderStage": ["Fragment"],
    "name": "material",
    "variable": {
      "type": "Structure",
      "members": [
        {"name": "albedo", "variable": {"type": "Numeric", "value": {"type": "Vector3", "x":1,"y":1,"z":1}}},
        {"name": "metallic", "variable": {"type": "Numeric", "value": {"type": "Float", "value": 0.0}}},
        {"name": "roughness", "variable": {"type": "Numeric", "value": {"type": "Float", "value": 0.5}}},
        {"name": "ao", "variable": {"type": "Numeric", "value": {"type": "Float", "value": 1.0}}}
      ]
    }
  }],
  "textures": [{
    "binding": 1,
    "uniformVarName": "albedoTexture",
    "path": "./Assets/Texture/wall.jpg"
  }]
}]
```

### Variable Types
- **Numeric**: `{"type":"Numeric", "value": {"type":"Float|Int|Vector2|Vector3|Vector4|Mat4", ...}}`
- **Structure**: `{"type":"Structure", "members": [{"name":"...", "variable":{...}}, ...]}`
- **Array**: `{"type":"Array", "members": [{...}, ...]}`

## Build Pipeline

### Shader Compilation (`shader_gen.py`)
```
python shader_gen.py --vulkan_sdk <SDK_PATH> --src_directory <GLSL_DIR> --dst_directory <SPV_DIR>
```
Recursively finds `.vert`/`.frag` → compiles via `glslc` → outputs `.spv`.

### Asset Staging (`assets_copy.py`)
```
python assets_copy.py --src_directory <SRC> --dst_directory <DST>
```
Cleans destination, copies entire asset tree.

### macOS ICD Fix (`fix_icd_paths.py`)
```
python fix_icd_paths.py --icd_dir <DIR> [--framework_path <REL_PATH>]
```
Rewrites Vulkan ICD JSON `library_path` to `@executable_path/../Frameworks/` for standalone bundles. Sets `api_version: 1.3.0`.

### CMake Post-Build Flow
1. Compile GLSL → SPIR-V
2. Copy assets (including compiled shaders) to build output
3. (macOS) Copy Vulkan runtime libs (libvulkan, MoltenVK) to Frameworks/
4. (macOS) Copy & fix ICD JSON configurations
