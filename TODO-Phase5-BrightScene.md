# Phase 5: 构建明亮场景（Unreal Engine 默认场景风格）

## 目标

将引擎的渲染效果从当前的"黑暗背景 + 微弱环境光"提升到类似 Unreal Engine 初始场景的明亮、自然光照效果。包括可配置的 clear color、可调环境光、后处理参数暴露、法线贴图支持等。

## 前置知识

### 当前渲染管线流程

```
Application::Loop(callback)
 └─ RenderSystem::RenderScene()
     ├─ RenderPrepare()              → 计算 viewProjMatrix，提取相机 frustum
     ├─ CollectRenderingContext()     → 遍历场景实体，做 frustum culling，按材质排序
     ├─ CollectLights()              → 收集灯光数据 (最多4方向光 + 8点光 + 4聚光灯)
     ├─ CalculateCascadeShadows()    → CSM: 4级级联阴影计算
     ├─ UpdateGlobalUniformBuffer()  → 上传 viewProjMat、cameraPos、灯光数据、CSM 到 GPU
     ├─ UpdateMaterialInstanceUniformBuffer()  → 上传每材质 shader uniforms
     ├─ RenderShadowPass()           → 阴影深度渲染
     ├─ RenderPass(Forward)          → PBR 前向渲染到 offscreen HDR RT
     │   └─ VulkanCommandBuffer::BeginRendering()  → 清屏 + 开始渲染
     ├─ PostProcess Chain            → Bloom → ToneMapping → 输出到 swapchain
     └─ RenderImGuiPass()            → ImGui 叠加渲染
```

### 关键文件清单

| 文件 | 作用 |
|------|------|
| `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.h/.cpp` | Vulkan 命令缓冲，BeginRendering 函数中硬编码 clear color |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | RenderSystem 公共 API |
| `src/Systems/RenderSystem/RenderSystem.cpp` | BuildGlobalUniform() 构建全局 UBO 结构 |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | 渲染流程主文件，UpdateGlobalUniformBuffer(), RenderPass(), CollectLights() |
| `src/Systems/RenderSystem/Detail/RenderIntermediateVariable.h` | 每帧渲染中间数据结构 |
| `example/Graphics/Assets/Shader/pbr.frag` | PBR 片元着色器，包含光照计算和环境光 |
| `example/Graphics/Assets/Shader/pbr.vert` | PBR 顶点着色器 |
| `example/Graphics/Assets/Shader/shadow.vert` | 阴影顶点着色器 |
| `include/Ailurus/Systems/RenderSystem/PostProcess/PostProcessChain.h` | 后处理链管理 |
| `include/Ailurus/Systems/RenderSystem/PostProcess/Effects/ToneMappingEffect.h` | Tone mapping 效果 |
| `include/Ailurus/Systems/RenderSystem/PostProcess/Effects/BloomMipChainEffect.h` | Bloom 效果 |
| `example/Graphics/Assets/Material/PBRMaterial.json` | PBR 材质定义 (JSON) |

### GlobalUniform UBO 布局 (std140)

GLSL 端 (`pbr.frag`, `pbr.vert`, `shadow.vert` 三处同步定义):
```glsl
layout(std140, set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    int numDirectionalLights;
    int numPointLights;
    int numSpotLights;
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
} globalUniform;
```

C++ 端在 `RenderSystem::BuildGlobalUniform()` 中用 `UniformVariableStructure` 动态构建，通过 `UniformSetMemory::SetUniformValue()` 上传。

---

## 任务清单

---

### 任务 1: 可配置 Clear Color（背景色）

**优先级**: 🔴 关键阻塞  
**难度**: 简单  
**涉及文件**:
- `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.h`
- `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.cpp`
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h`
- `src/Systems/RenderSystem/RenderSystem.Render.cpp`

**当前问题**:  
`VulkanCommandBuffer::BeginRendering()` 在第 202 行硬编码了 clear color 为 `{0.0f, 0.0f, 0.0f, 1.0f}`（纯黑），没有暴露给上层任何配置接口。

**具体步骤**:

#### 1.1 修改 `VulkanCommandBuffer::BeginRendering()` 增加 clear color 参数

**文件**: `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.h`

当前签名（约第 77 行）:
```cpp
void BeginRendering(vk::ImageView colorImageView, vk::ImageView depthImageView, 
    vk::ImageView resolveImageView, vk::Extent2D extent, 
    bool clearColor = true, bool useDepth = true);
```

修改为:
```cpp
void BeginRendering(vk::ImageView colorImageView, vk::ImageView depthImageView, 
    vk::ImageView resolveImageView, vk::Extent2D extent, 
    bool clearColor = true, bool useDepth = true,
    std::array<float, 4> clearColorValue = {0.0f, 0.0f, 0.0f, 1.0f});
```

**文件**: `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.cpp`

在 `BeginRendering` 函数（约第 202 行），将:
```cpp
.setClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
```
改为:
```cpp
.setClearValue(vk::ClearColorValue(clearColorValue));
```

#### 1.2 在 RenderSystem 中添加 SetClearColor API

**文件**: `include/Ailurus/Systems/RenderSystem/RenderSystem.h`

在 public 区域的 Camera 部分后面添加:
```cpp
// Clear color
void SetClearColor(float r, float g, float b, float a = 1.0f);
std::array<float, 4> GetClearColor() const;
```

在 private 区域添加成员变量:
```cpp
std::array<float, 4> _clearColor = {0.1f, 0.1f, 0.1f, 1.0f};
```

**文件**: `src/Systems/RenderSystem/RenderSystem.cpp` (或独立实现文件)

添加实现:
```cpp
void RenderSystem::SetClearColor(float r, float g, float b, float a)
{
    _clearColor = {r, g, b, a};
}

std::array<float, 4> RenderSystem::GetClearColor() const
{
    return _clearColor;
}
```

#### 1.3 在 RenderPass 调用中传递 clear color

**文件**: `src/Systems/RenderSystem/RenderSystem.Render.cpp`

在 `RenderSystem::RenderPass()` 函数中（约第 797-806 行），所有 `pCommandBuffer->BeginRendering(...)` 调用都需要追加 `_clearColor` 参数:
```cpp
pCommandBuffer->BeginRendering(msaaColorView, msaaDepthView, offscreenColorView, extent, clearColor, true, _clearColor);
```

**验证**: 在 Main.cpp 中调用 `Application::Get<RenderSystem>()->SetClearColor(0.5f, 0.6f, 0.7f);` 后运行，背景应变为蓝灰色。

---

### 任务 2: 可配置环境光强度

**优先级**: 🔴 关键阻塞  
**难度**: 中等  
**涉及文件**:
- `example/Graphics/Assets/Shader/pbr.frag` (着色器)
- `example/Graphics/Assets/Shader/pbr.vert` (着色器，需同步 UBO 布局)
- `example/Graphics/Assets/Shader/shadow.vert` (着色器，需同步 UBO 布局)
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h` (C++ API)
- `src/Systems/RenderSystem/RenderSystem.cpp` (BuildGlobalUniform)
- `src/Systems/RenderSystem/RenderSystem.Render.cpp` (UpdateGlobalUniformBuffer)
- `src/Systems/RenderSystem/Detail/RenderIntermediateVariable.h` (中间数据)

**当前问题**:  
PBR 着色器中环境光硬编码为 `vec3(0.03) * albedo * ao`（仅 3% 亮度），非常暗。无法从 C++ 侧调节。

**具体步骤**:

#### 2.1 在 GlobalUniform 中添加 ambientStrength 和 ambientColor

**重要**: GlobalUniform 布局必须在 3 个着色器文件中**完全同步**修改。

**文件**: `example/Graphics/Assets/Shader/pbr.frag`、`pbr.vert`、`shadow.vert`

在 GlobalUniform 的末尾（`float cascadeSplitDistances[4];` 之后）添加:
```glsl
layout(std140, set = 0, binding = 0) uniform GlobalUniform {
    // ... 现有字段不变 ...
    mat4 cascadeViewProjMatrices[4];
    float cascadeSplitDistances[4];
    // 新增环境光参数
    vec4 ambientColor;              // xyz = ambient color, w = ambient strength
} globalUniform;
```

> **注意 std140 对齐**: `vec4` 是 16 字节对齐，放在 `float[4]` 之后没有对齐问题。`float cascadeSplitDistances[4]` 每个元素占 16 字节（std140 数组元素对齐到 vec4），所以共 64 字节，新 vec4 会紧跟其后。

#### 2.2 修改 PBR 着色器 main() 中的环境光计算

**文件**: `example/Graphics/Assets/Shader/pbr.frag`

找到（约第 262 行）:
```glsl
vec3 ambient = vec3(0.03) * albedo * ao;
```

改为:
```glsl
vec3 ambient = globalUniform.ambientColor.rgb * globalUniform.ambientColor.w * albedo * ao;
```

#### 2.3 C++ 侧添加 ambient 到 GlobalUniform 结构

**文件**: `src/Systems/RenderSystem/RenderSystem.cpp` → `BuildGlobalUniform()`

在 cascadeSplitDistances 添加之后，添加:
```cpp
pGlobalUniformStructure->AddMember(
    "ambientColor",
    std::make_unique<UniformVariableNumeric>(UniformValueType::Vector4));
```

同时在 RenderSystem.h 中添加静态常量:
```cpp
static const char* GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR;
```

在 .cpp 中给出定义值:
```cpp
const char* RenderSystem::GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR = "ambientColor";
```

#### 2.4 添加 RenderSystem API

**文件**: `include/Ailurus/Systems/RenderSystem/RenderSystem.h`

Public 区域：
```cpp
// Ambient light
void SetAmbientColor(float r, float g, float b);
void SetAmbientStrength(float strength);
```

Private 成员：
```cpp
Vector3f _ambientColor = {1.0f, 1.0f, 1.0f};
float _ambientStrength = 0.3f;
```

#### 2.5 在 UpdateGlobalUniformBuffer() 中上传 ambient 数据

**文件**: `src/Systems/RenderSystem/RenderSystem.Render.cpp` → `UpdateGlobalUniformBuffer()`

在 CSM 数据上传之后添加:
```cpp
Vector4f ambientData(_ambientColor.x, _ambientColor.y, _ambientColor.z, _ambientStrength);
_pGlobalUniformMemory->SetUniformValue(
    { 0, GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR },
    ambientData);
```

**验证**: 在 Main.cpp 中调用 `Application::Get<RenderSystem>()->SetAmbientStrength(0.4f);` 后运行，暗面应明显变亮。

---

### 任务 3: 暴露后处理链 API

**优先级**: 🟡 中等  
**难度**: 简单  
**涉及文件**:
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h`

**当前问题**:  
`PostProcessChain` 是 RenderSystem 的 private 成员，用户无法在 Main.cpp 中修改 Bloom 阈值、Tone Mapping 曝光等参数。

**具体步骤**:

#### 3.1 添加 PostProcessChain 公开访问

**文件**: `include/Ailurus/Systems/RenderSystem/RenderSystem.h`

在 public 区域添加:
```cpp
// Post-process access
PostProcessChain* GetPostProcessChain() const { return _postProcessChain.get(); }
```

**用法示例** (在 Main.cpp 中):
```cpp
auto* pPostProcess = Application::Get<RenderSystem>()->GetPostProcessChain();
if (auto* pBloom = dynamic_cast<BloomMipChainEffect*>(pPostProcess->GetEffect("BloomMipChain")))
{
    pBloom->SetThreshold(0.4f);
    pBloom->SetBloomIntensity(0.8f);
}
if (auto* pToneMap = dynamic_cast<ToneMappingEffect*>(pPostProcess->GetEffect("ToneMapping")))
{
    pToneMap->SetExposure(1.2f);
}
```

> **注意**: 需要确认 BloomMipChainEffect 和 ToneMappingEffect 注册时使用的名称字符串。查看 `PostProcessChain` 的构造函数或者 `RenderSystem` 构造函数中添加 effect 的代码。

**验证**: 在 Main.cpp 中修改 bloom threshold 为 0.4，可以看到明显的辉光效果。

---

### 任务 4: 法线贴图支持

**优先级**: 🟡 中等  
**难度**: 较高  
**涉及文件**:
- `example/Graphics/Assets/Shader/pbr.frag` (片元着色器)
- `example/Graphics/Assets/Shader/pbr.vert` (顶点着色器)
- `example/Graphics/Assets/Material/PBRMaterial.json` (材质定义)

**当前问题**:  
PBR 着色器只使用顶点法线 `fragNormal`，没有法线贴图采样。表面看起来非常平坦，缺少细节。

**具体步骤**:

#### 4.1 顶点着色器 — 计算 TBN 矩阵

**文件**: `example/Graphics/Assets/Shader/pbr.vert`

需要添加切线 (tangent) 输入和 TBN 矩阵输出。

**前提**: 检查模型加载时是否已经计算了切线 (tangent)。查看 Assimp 导入设置中是否有 `aiProcess_CalcTangentSpace`。如果没有，需要在模型加载代码中添加此 flag，并在顶点布局中添加 tangent 属性。

涉及的模型加载文件:
- 搜索 `aiProcess_` 或 `Assimp` 在 `src/Systems/AssetsSystem/` 中的使用
- 检查 Vertex 结构定义（搜索 `struct Vertex` 或顶点布局）

添加到 pbr.vert:
```glsl
layout(location = 3) in vec3 inTangent;

layout(location = 3) out mat3 fragTBN;  // 注意: mat3 占用 3 个 location (3,4,5)
```

main() 中计算 TBN:
```glsl
vec3 T = normalize(normalMatrix * inTangent);
vec3 N = normalize(normalMatrix * inNormal);
// Re-orthogonalize (Gram-Schmidt)
T = normalize(T - dot(T, N) * N);
vec3 B = cross(N, T);
fragTBN = mat3(T, B, N);
```

#### 4.2 片元着色器 — 采样法线贴图

**文件**: `example/Graphics/Assets/Shader/pbr.frag`

添加法线贴图采样器:
```glsl
layout(set = 1, binding = 2) uniform sampler2D normalTexture;
```

添加输入:
```glsl
layout(location = 3) in mat3 fragTBN;
```

修改 main() 中法线获取逻辑:
```glsl
// 采样法线贴图并从 [0,1] 转换到 [-1,1]
vec3 N = texture(normalTexture, fragUV).rgb;
N = N * 2.0 - 1.0;
N = normalize(fragTBN * N);
```

#### 4.3 更新材质 JSON

**文件**: `example/Graphics/Assets/Material/PBRMaterial.json`

在 Forward pass 的 textures 数组中添加:
```json
{
    "binding": 2,
    "uniformVarName": "normalTexture",
    "path": "./Assets/Texture/wall_normal.jpg"
}
```

> **注意**: 需要准备一张法线贴图文件 `wall_normal.jpg`，或者使用默认的平面法线贴图 (平坦 RGB=(128,128,255)) 作为 fallback。

#### 4.4 顶点布局兼容性检查

需要确认引擎的以下部分是否需要更新:
1. **Vertex 结构体**: 需添加 `tangent` 字段
2. **VertexInputDescription**: Vulkan vertex input binding 需要包含 tangent attribute
3. **Mesh 加载**: Assimp 导入需要 `aiProcess_CalcTangentSpace`
4. **VkPipelineVertexInputStateCreateInfo**: 管线创建需要匹配新的顶点格式

---

## 实施记录

> **完成日期**: 2026-03-13  
> **编译状态**: ✅ 全部通过（C++ 编译 + SPIR-V 着色器编译，零错误零警告）

---

### 任务 1 实施: 可配置 Clear Color ✅

**做了什么**:
- `VulkanCommandBuffer::BeginRendering()` 增加了 `std::array<float, 4> clearColorValue` 参数（默认 `{0,0,0,1}`），替换原来硬编码的纯黑色。
- `RenderSystem` 新增 `SetClearColor(r,g,b,a)` / `GetClearColor()` 公共 API 和 `_clearColor` 私有成员（默认 `{0.1, 0.1, 0.1, 1.0}` 深灰色）。
- `RenderPass()` 中 MSAA / non-MSAA 两条路径的 `BeginRendering` 调用均传递 `_clearColor`。

**修改的文件**:
| 文件 | 改动 |
|------|------|
| `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.h` | 增加 `<array>` include；`BeginRendering` 签名增加 `clearColorValue` 参数 |
| `src/VulkanContext/CommandBuffer/VulkanCommandBuffer.cpp` | `setClearValue` 改用参数 `clearColorValue` |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | 增加 `<array>` include；public 区添加 `SetClearColor`/`GetClearColor`；private 区添加 `_clearColor` |
| `src/Systems/RenderSystem/RenderSystem.cpp` | 添加 `SetClearColor`/`GetClearColor` 实现 |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | `RenderPass()` 两处 `BeginRendering` 调用追加 `_clearColor` |

**遇到的问题**: 无。

**使用方法**:
```cpp
Application::Get<RenderSystem>()->SetClearColor(0.5f, 0.6f, 0.7f); // 蓝灰色背景
```

---

### 任务 2 实施: 可配置环境光强度 ✅

**做了什么**:
- GlobalUniform UBO 布局末尾新增 `vec4 ambientColor`（xyz = 颜色，w = 强度），三个着色器文件同步修改。
- PBR 片元着色器中环境光从硬编码 `vec3(0.03)` 改为 `globalUniform.ambientColor.rgb * globalUniform.ambientColor.w`。
- C++ 侧 `BuildGlobalUniform()` 新增 `ambientColor` Vector4 成员；`UpdateGlobalUniformBuffer()` 每帧上传。
- `RenderSystem` 新增 `SetAmbientColor(r,g,b)` / `SetAmbientStrength(strength)` API，默认白色 × 0.3 强度（比原来 0.03 提升 10 倍）。

**修改的文件**:
| 文件 | 改动 |
|------|------|
| `example/Graphics/Assets/Shader/pbr.frag` | GlobalUniform 添加 `vec4 ambientColor`；环境光计算改用该 uniform |
| `example/Graphics/Assets/Shader/pbr.vert` | GlobalUniform 添加 `vec4 ambientColor`（仅布局同步） |
| `example/Graphics/Assets/Shader/shadow.vert` | GlobalUniform 添加 `vec4 ambientColor`（仅布局同步） |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | 新增 `SetAmbientColor`/`SetAmbientStrength` API；`_ambientColor`/`_ambientStrength` 成员；`GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR` 静态常量；`GetGlobalUniformAccessNameAmbientColor()` helper |
| `src/Systems/RenderSystem/RenderSystem.cpp` | `GLOBAL_UNIFORM_ACCESS_AMBIENT_COLOR` 定义；`SetAmbientColor`/`SetAmbientStrength` 实现；`BuildGlobalUniform()` 添加 `ambientColor` 成员；access name helper |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | `UpdateGlobalUniformBuffer()` 添加 ambient 数据上传 |

**遇到的问题**: 无。

**使用方法**:
```cpp
Application::Get<RenderSystem>()->SetAmbientStrength(0.4f);  // 提高环境光
Application::Get<RenderSystem>()->SetAmbientColor(1.0f, 0.95f, 0.9f); // 暖白环境光
```

---

### 任务 3 实施: 暴露后处理链 API ✅

**做了什么**:
- `RenderSystem` public 区新增 inline getter: `PostProcessChain* GetPostProcessChain() const`。

**修改的文件**:
| 文件 | 改动 |
|------|------|
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | 添加 `GetPostProcessChain()` |

**遇到的问题**: 无。

**已确认的效果名称**: `"BloomMipChain"`（`BloomMipChainEffect`）和 `"ToneMapping"`（`ToneMappingEffect`）。

**使用方法**:
```cpp
auto* pPostProcess = Application::Get<RenderSystem>()->GetPostProcessChain();
if (auto* pBloom = dynamic_cast<BloomMipChainEffect*>(pPostProcess->GetEffect("BloomMipChain")))
{
    pBloom->SetThreshold(0.4f);
    pBloom->SetBloomIntensity(0.8f);
}
if (auto* pToneMap = dynamic_cast<ToneMappingEffect*>(pPostProcess->GetEffect("ToneMapping")))
{
    pToneMap->SetExposure(1.2f);
}
```

---

### 任务 4 实施: 法线贴图支持 ✅

**做了什么**:
- **Assimp 导入**: 取消注释 `aiProcess_CalcTangentSpace`，启用自动切线/副切线计算。引擎已有的动态顶点布局 `ReadLayout()` 会在检测到 tangent/bitangent 时自动将它们加入顶点属性（无需额外修改顶点结构体或管线）。
- **pbr.vert**: 新增 `layout(location = 3) in vec3 inTangent` 输入、`layout(location = 3) out mat3 fragTBN` 输出（mat3 占用 location 3,4,5）；main() 中用 Gram-Schmidt 正交化计算 TBN 矩阵。
- **shadow.vert**: 新增 `layout(location = 3) in vec3 inTangent` 输入声明（保持与 pbr.vert 顶点布局一致，但不使用）。
- **pbr.frag**: 新增 `sampler2D normalTexture`（set=1, binding=2）和 `in mat3 fragTBN` 输入；main() 中采样法线贴图并用 TBN 矩阵变换到世界空间。
- **PBRMaterial.json**: Forward pass textures 新增 binding 2 `normalTexture`，默认指向 `default_normal.png`。
- **默认法线贴图**: 生成 2×2 像素 `default_normal.png`（RGB=128,128,255 = 切线空间 +Z 法线），放置在 `example/Graphics/Assets/Texture/`。

**修改的文件**:
| 文件 | 改动 |
|------|------|
| `src/Systems/AssetsSystem/AssetsSystem.Model.cpp` | 取消注释 `aiProcess_CalcTangentSpace` |
| `example/Graphics/Assets/Shader/pbr.vert` | 新增 tangent 输入、TBN 矩阵输出和计算 |
| `example/Graphics/Assets/Shader/shadow.vert` | 新增 tangent 输入声明 |
| `example/Graphics/Assets/Shader/pbr.frag` | 新增 normalTexture 采样器和 TBN 输入；法线改为从贴图采样 |
| `example/Graphics/Assets/Material/PBRMaterial.json` | 新增 normalTexture 绑定 |
| `example/Graphics/Assets/Texture/default_normal.png` | 新建：2×2 默认平面法线贴图 |

**遇到的问题与解决**:

1. **Assimp 的 `ReadLayout()` 会同时添加 Tangent 和 Bitangent**  
   启用 `aiProcess_CalcTangentSpace` 后，顶点布局变为 Position(0), Normal(1), TexCoord(2), Tangent(3), Bitangent(4)。着色器只声明到 location 3 (inTangent)，Bitangent (location 4) 的数据存在但着色器不读取。Vulkan 允许这种"多余属性"的情况——VkVertexInputAttributeDescription 会包含 Bitangent 的描述（由动态顶点布局生成），但着色器未使用的属性会被忽略，不影响正确性。  

2. **顶点布局与管线创建无需额外修改**  
   引擎采用动态顶点布局系统：`ReadLayout()` → `VulkanVertexLayout` → `VkVertexInputAttributeDescription` 全部自动适配。管线 (`VkPipelineVertexInputStateCreateInfo`) 从 `VulkanVertexLayout` 获取描述，Vertex 数据也是 `ReadLayout()` 驱动的 interleaved buffer，所以启用 tangent 后整个管线自动兼容。

3. **Bitangent 在着色器中通过叉积计算**  
   片元着色器中不需要从顶点传入 bitangent，而是在 `pbr.vert` 中 `B = cross(N, T)` 计算得到。这避免了需要额外的 `layout(location = 4) in vec3 inBitangent`，减少了 varying 数量。

**使用方法**:
用真实法线贴图替换 `PBRMaterial.json` 中的 `default_normal.png` 路径即可启用法线贴图效果：
```json
{
    "binding": 2,
    "uniformVarName": "normalTexture",
    "path": "./Assets/Texture/my_normal_map.png"
}
```

搜索关键词: `VertexInput`, `VkVertexInputAttributeDescription`, `struct Vertex`, `aiProcess_`

**验证**: 加载带法线贴图的材质后，方块表面应显示凹凸细节。

---

### 任务 5: 金属度/粗糙度/AO 贴图支持

**优先级**: 🟢 低  
**难度**: 中等  
**涉及文件**:
- `example/Graphics/Assets/Shader/pbr.frag`
- `example/Graphics/Assets/Material/PBRMaterial.json`

**当前问题**:  
metallic、roughness、ao 目前只是标量 uniform 值，不支持纹理贴图。现代 PBR 材质通常使用 MetallicRoughness 贴图。

**具体步骤**:

#### 5.1 添加贴图采样器

**文件**: `example/Graphics/Assets/Shader/pbr.frag`

添加采样器:
```glsl
layout(set = 1, binding = 3) uniform sampler2D metallicRoughnessTexture;  // G=roughness, B=metallic (glTF convention)
layout(set = 1, binding = 4) uniform sampler2D aoTexture;
```

#### 5.2 修改 main() 中读取逻辑

**文件**: `example/Graphics/Assets/Shader/pbr.frag`

将:
```glsl
float metallic = material.metallic;
float roughness = material.roughness;
float ao = material.ao;
```

改为:
```glsl
vec4 mrSample = texture(metallicRoughnessTexture, fragUV);
float metallic = material.metallic * mrSample.b;   // Blue channel = metallic
float roughness = material.roughness * mrSample.g;  // Green channel = roughness
float ao = material.ao * texture(aoTexture, fragUV).r;
```

> 当贴图是全白纹理时，乘以 uniform 标量等同于只用标量值，保持向后兼容。

#### 5.3 更新材质 JSON

 在 PBRMaterial.json 中添加对应 texture binding（可先使用 1x1 白色纹理作为默认值）。

#### 5.4 准备默认纹理

创建一个 1x1 像素的白色 PNG 图片作为默认贴图，当材质不需要某个贴图时使用白色纹理。

**验证**: 使用全白贴图时效果应与之前一致（不引入回归）。使用真实 PBR 贴图时应能看到金属高光和粗糙度变化。

---

### 任务 6: 阴影 Bias 可配置

**优先级**: 🟢 低  
**难度**: 简单  
**涉及文件**:
- `example/Graphics/Assets/Shader/pbr.frag`
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h`
- `src/Systems/RenderSystem/RenderSystem.cpp`
- `src/Systems/RenderSystem/RenderSystem.Render.cpp`

**当前问题**:  
阴影采样中的 bias 硬编码在 PBR 着色器中（第 ~155 行）:
```glsl
shadow += (compareDepth - 0.005 > depth) ? 0.0 : 1.0;
```

**具体步骤**:

#### 6.1 在 GlobalUniform 中添加 shadowBias

在 GlobalUniform 结尾添加（ambientColor 之后）:
```glsl
float shadowBias;     // 阴影偏移值
float shadowNormalBias; // 法线方向偏移
// std140 padding: 需要对齐到 16 字节，如果需要可加 padding
```

> **std140 对齐注意**: 在 `vec4 ambientColor` 之后添加两个 float，需要额外 2 个 float 的 padding 来对齐到 16 字节边界。或者直接用 `vec4 shadowParams`（x=bias, y=normalBias, zw=reserved）。

推荐方案:
```glsl
vec4 shadowParams;  // x = depth bias, y = normal bias, z = reserved, w = reserved
```

#### 6.2 修改阴影采样函数

在 `sampleShadowMap` 函数中，将:
```glsl
shadow += (compareDepth - 0.005 > depth) ? 0.0 : 1.0;
```
改为:
```glsl
shadow += (compareDepth - globalUniform.shadowParams.x > depth) ? 0.0 : 1.0;
```

#### 6.3 C++ 侧

在 RenderSystem 添加:
```cpp
void SetShadowBias(float depthBias, float normalBias = 0.0f);
```

在 BuildGlobalUniform 添加 `shadowParams` 成员定义（类型 Vector4），在 UpdateGlobalUniformBuffer 上传。

**同步修改**: `pbr.vert` 和 `shadow.vert` 的 GlobalUniform 声明需同步更新。

**验证**: 调节 bias 值应能消除阴影痤疮 (shadow acne) 或 peter-panning 现象。

---

### 任务 7: 更新 Example 场景

**优先级**: 🟡 中等（在任务 1-3 完成后进行）  
**难度**: 简单  
**涉及文件**:
- `example/Graphics/Main.cpp`

**目标**: 利用新暴露的 API 构建一个明亮的测试场景。

**具体步骤**:

```cpp
// 1. 设置天蓝色背景
Application::Get<RenderSystem>()->SetClearColor(0.53f, 0.81f, 0.92f);

// 2. 提高环境光到自然水平
Application::Get<RenderSystem>()->SetAmbientColor(0.8f, 0.85f, 0.95f);  // 略带蓝色的天光
Application::Get<RenderSystem>()->SetAmbientStrength(0.4f);

// 3. 调整后处理 (需要任务 3 完成)
auto* pPostProcess = Application::Get<RenderSystem>()->GetPostProcessChain();
// Bloom - 降低阈值让辉光在正常亮度下可见
if (auto* pBloom = dynamic_cast<BloomMipChainEffect*>(pPostProcess->GetEffect("BloomMipChain")))
{
    pBloom->SetThreshold(0.5f);
    pBloom->SetBloomIntensity(0.6f);
}
// Tone mapping - 略微提高曝光
if (auto* pToneMap = dynamic_cast<ToneMappingEffect*>(pPostProcess->GetEffect("ToneMapping")))
{
    pToneMap->SetExposure(1.3f);
}

// 4. 灯光设置 - 模拟室外自然光
// 主方向光 (太阳)
pLightComp->SetDirection({ -0.3f, -0.8f, -0.4f });
pLightComp->SetColor({ 1.0f, 0.98f, 0.95f });
pLightComp->SetIntensity(2.0f);
```

---

## 执行顺序

```
任务 1 (Clear Color)  ─────┐
                            ├→ 任务 7 (更新 Example 场景)
任务 2 (环境光)  ──────────┤
                            │
任务 3 (后处理 API)  ──────┘

任务 4 (法线贴图)  ─→ 独立，可并行

任务 5 (金属度/粗糙度贴图)  ─→ 依赖任务 4 的顶点布局修改

任务 6 (阴影 Bias)  ─→ 独立，可并行
```

**推荐执行顺序**: 1 → 2 → 3 → 7 → 4 → 6 → 5

任务 1-3 完成后就能实现明亮场景的基本效果。任务 4-6 是进一步提升画质的增强项。

---

## 注意事项

1. **Shader UBO 同步**: 修改 GlobalUniform 时，必须同时更新 `pbr.frag`、`pbr.vert`、`shadow.vert` 三个着色器文件中的声明，否则 std140 布局不匹配会导致渲染错误。

2. **Shader 编译**: 引擎使用预编译 SPIR-V 着色器。修改 `.frag`/`.vert` 后需要重新编译为 `.spv`。构建系统（CMake）会自动处理，但要确保 glslc/glslangValidator 工具可用。

3. **std140 对齐规则**:
   - `float` 占 4 字节，对齐到 4
   - `vec3` 占 12 字节，对齐到 16
   - `vec4` 占 16 字节，对齐到 16
   - `mat4` 占 64 字节，对齐到 16
   - 数组元素对齐到 `vec4`（16 字节），即 `float[4]` 实际占 64 字节
   - 新增字段务必在 C++ 和 GLSL 两端同步

4. **向后兼容**: 所有新功能应具有合理的默认值，确保现有代码不修改也能正常运行。

5. **Post-process effect 名称**: 使用 `GetEffect()` 时需传入正确的注册名称。查看 RenderSystem 构造函数中 `_postProcessChain->AddEffect()` 的调用来确认。
