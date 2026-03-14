# Phase 6: 高级渲染特性（Skybox / IBL / SSAO / 阴影改进 / Gamma 校正）

## 目标

将引擎的渲染品质从当前的"单色背景 + 常量环境光"提升到接近 Unity/Unreal 默认场景水准。包括天空盒、基于图像的光照 (IBL)、屏幕空间环境光遮蔽 (SSAO)、阴影偏移改进、Gamma 正确纹理采样等。

## 当前引擎状态

### 渲染管线流程

```
RenderSystem::RenderScene()
 ├─ RenderPrepare()                → viewProjMatrix, frustum
 ├─ CollectRenderingContext()      → frustum culling, 按材质排序
 ├─ CollectLights()                → dir/point/spot 灯光数据
 ├─ CalculateCascadeShadows()      → 4 级 CSM 计算
 ├─ UpdateGlobalUniformBuffer()    → camera, lights, CSM, ambient → UBO
 ├─ UpdateMaterialInstanceUniformBuffer() → 每材质 uniform
 ├─ RenderShadowPass()             → 深度渲染到 4 张 2048² shadow map
 ├─ RenderPass(Forward)            → PBR 前向渲染到 offscreen HDR RT (R16G16B16A16Sfloat)
 │   └─ BeginRendering() → 清屏, 渲染所有 mesh → RenderSkybox(深度≤1.0, 仅填充空像素)
 └─ PostProcessChain::Execute()    → Bloom(mip chain) → ToneMapping(ACES) → swapchain
```

### 关键架构约束

| 组件 | 现状 | 限制 |
|------|------|------|
| VulkanImage | ✅ 支持 `CreateFromConfig()`: cubemap、HDR 格式、mipmap、自定义 flags/usage | 原有 `Create(const Image&)` 仍可用 |
| RenderTarget | ✅ 支持 cubemap RT (mipLevels, arrayLayers, flags, viewType) + `CreateSingleLayerMipView()` | 原有默认行为不变 |
| VulkanSampler | ✅ 支持 `CreateFromConfig()`: ClampToEdge、mipmap LOD、shadow compare 等 | 原有无参 `Create()` 仍可用 |
| PostProcessEffect | 接口完善：Init/Render/OnResize/Shutdown | ✅ 可直接扩展 |
| PostProcessResourcePool | RTHandle 稳定指针，自动 resize | ✅ 可注册新 RT |
| PostProcessPipelineFactory | 全屏三角形 + 自定义 frag shader | ✅ 可直接创建新 pipeline |
| GlobalUniform | std140，末尾是 `vec4 ambientColor` | 可追加新字段 |
| Material JSON | 支持多 pass (Shadow/Forward)、textures 数组 | ✅ 可扩展新 binding |
| Shadow bias | 硬编码 `0.005` 在 `pbr.frag sampleShadowMap()` | 需改为动态参数 |

---

## 任务清单

| # | 任务 | 优先级 | 难度 | 前置依赖 | 状态 |
|---|------|--------|------|----------|------|
| 1 | 扩展 VulkanImage 支持 Cubemap 和 HDR | 🔴 高 | 高 | 无 | ✅ 已完成 |
| 2 | 扩展 VulkanSampler 支持可配置参数 | 🔴 高 | 中 | 无 | ✅ 已完成 |
| 3 | 扩展 RenderTarget 支持 Cubemap 和 Mipmap | 🔴 高 | 高 | 无 | ✅ 已完成 |
| 4 | Skybox 渲染 | 🔴 高 | 高 | 任务 1, 2 | ✅ 已完成 |
| 5 | IBL — 辐照度(Irradiance)和预滤波环境贴图 | 🔴 高 | 很高 | 任务 1, 2, 3, 4 | ✅ 已完成 |
| 6 | 阴影偏移改进 | 🟡 中 | 中 | 无 | ✅ 已完成 |
| 7 | Gamma 正确纹理采样 | 🟡 中 | 中 | 无 | ✅ 已完成 |
| 8 | SSAO（屏幕空间环境光遮蔽） | 🟡 中 | 高 | 无 | ✅ 已完成 |

---

## 任务 1: 扩展 VulkanImage 支持 Cubemap 和 HDR

**优先级**: 🔴 高（Skybox、IBL 的前提）  
**难度**: 高  
**涉及文件**:

| 文件 | 作用 |
|------|------|
| `src/VulkanContext/Resource/Image/VulkanImage.h` | VulkanImage 类定义 |
| `src/VulkanContext/Resource/Image/VulkanImage.cpp` | `VulkanImage::Create()` 图像创建逻辑 |
| `src/VulkanContext/Resource/VulkanResourceManager.h` | `CreateImage()` 公共接口 |
| `src/VulkanContext/Resource/VulkanResourceManager.cpp` | `CreateImage()` 实现 |
| `include/Ailurus/Utility/Image.h` | 像素数据加载类 |
| `src/Utility/Image.cpp` | Image 加载实现（使用 stb_image） |

**当前问题**:
`VulkanImage::Create(const Image& image)` 硬编码了以下参数：
- 格式: `vk::Format::eR8G8B8A8Srgb`（8bit LDR）
- 类型: `vk::ImageType::e2D` / `vk::ImageViewType::e2D`
- MipLevels: `1`
- ArrayLayers: `1`
- 无 cubemap flag

Skybox 需要 cubemap（6 面、`eCube` view type），IBL 需要 HDR 和 mipmap。

**具体步骤**:

### 1.1 定义 ImageCreateConfig 结构体

**文件**: `src/VulkanContext/Resource/Image/VulkanImage.h`

在 VulkanImage 类之前添加配置结构体：

```cpp
struct VulkanImageCreateConfig
{
    uint32_t width = 0;
    uint32_t height = 0;
    vk::Format format = vk::Format::eR8G8B8A8Srgb;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    vk::ImageCreateFlags flags = {};           // 加 eCubeCompatible 用于 cubemap
    vk::ImageViewType viewType = vk::ImageViewType::e2D;
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor;
};
```

### 1.2 扩展 VulkanImage 存储信息

**文件**: `src/VulkanContext/Resource/Image/VulkanImage.h`

给 VulkanImage 增加成员：

```cpp
uint32_t _mipLevels = 1;
uint32_t _arrayLayers = 1;
vk::ImageViewType _viewType = vk::ImageViewType::e2D;
```

更新构造函数以接收这些参数。

### 1.3 新增 CreateImageFromConfig() 静态方法

**文件**: `src/VulkanContext/Resource/Image/VulkanImage.cpp`

新增方法 `VulkanImage::CreateFromConfig(const VulkanImageCreateConfig& config, const void* pixelData, size_t dataSize)`。

核心修改（对比原 `Create(const Image&)` 方法）：
```cpp
// 原代码（硬编码）
imageInfo.setFormat(vk::Format::eR8G8B8A8Srgb)
    .setMipLevels(1)
    .setArrayLayers(1);
// ...
viewInfo.setViewType(vk::ImageViewType::e2D)
    .setSubresourceRange(...setLevelCount(1).setLayerCount(1));

// 新代码（参数化）
imageInfo.setFormat(config.format)
    .setMipLevels(config.mipLevels)
    .setArrayLayers(config.arrayLayers)
    .setFlags(config.flags)              // eCubeCompatible for cubemaps
    .setUsage(config.usage);
// ...
viewInfo.setViewType(config.viewType)    // eCube for cubemaps
    .setSubresourceRange(
        ...setLevelCount(config.mipLevels)
           .setLayerCount(config.arrayLayers));
```

**关键**: 对于 cubemap，需要将 6 张面的像素数据按顺序排列在一个 buffer 中，然后用 6 个 `vk::BufferImageCopy` region 分别上传到 `baseArrayLayer` 0-5。

Cubemap 上传逻辑：
```cpp
if (config.arrayLayers > 1)
{
    size_t faceSize = dataSize / config.arrayLayers;
    std::vector<vk::BufferImageCopy> regions(config.arrayLayers);
    for (uint32_t i = 0; i < config.arrayLayers; i++)
    {
        regions[i].setBufferOffset(i * faceSize)
            .setImageSubresource(vk::ImageSubresourceLayers(
                vk::ImageAspectFlagBits::eColor, 0, i, 1))
            .setImageExtent(vk::Extent3D(config.width, config.height, 1));
    }
    commandBuffer.copyBufferToImage(stagingBuffer->buffer, vkImage,
        vk::ImageLayout::eTransferDstOptimal, regions);
}
```

### 1.4 扩展 Image 类支持 HDR 加载

**文件**: `include/Ailurus/Utility/Image.h` 和 `src/Utility/Image.cpp`

stb_image 已经在 submodule 中。添加 HDR 支持：

```cpp
// Image.h — 新增
class Image
{
public:
    // 现有 constructors...
    
    bool IsHDR() const;
    const float* GetHDRData() const;   // 返回 float* (RGB 3 通道或 RGBA 4 通道)
    
private:
    std::vector<float> _hdrData;       // HDR 像素数据
    bool _isHDR = false;
    int _channels = 4;
};
```

```cpp
// Image.cpp — 在文件加载构造函数中
#include <stb_image.h>

Image::Image(const std::string& filePath)
{
    if (stbi_is_hdr(filePath.c_str()))
    {
        _isHDR = true;
        int w, h, channels;
        float* data = stbi_loadf(filePath.c_str(), &w, &h, &channels, 4); // 强制 RGBA
        if (data)
        {
            _width = w; _height = h; _channels = 4;
            _hdrData.assign(data, data + w * h * 4);
            stbi_image_free(data);
        }
    }
    else
    {
        // 现有 LDR 加载逻辑不变
    }
}
```

### 1.5 在 VulkanResourceManager 中暴露新接口

**文件**: `src/VulkanContext/Resource/VulkanResourceManager.h`

```cpp
VulkanImage* CreateImageFromConfig(const VulkanImageCreateConfig& config,
                                    const void* pixelData, size_t dataSize);
```

### 1.6 保持原有 `CreateImage(const Image&)` 不变

原有 `CreateImage(const Image& image)` 方法保持不变，这样现有的材质加载流程完全不受影响。新的 `CreateImageFromConfig` 仅供 Skybox、IBL 等高级功能使用。

**验证**:
1. 编译通过，现有场景不受影响
2. 单元测试：创建一个 1x1 的 `eR16G16B16A16Sfloat` 图像，确认 VkImage 和 VkImageView 正确创建
3. 用 `config.flags = eCubeCompatible, arrayLayers = 6, viewType = eCube` 创建 cubemap 图像验证

---

## 任务 2: 扩展 VulkanSampler 支持可配置参数

**优先级**: 🔴 高（Skybox ClampToEdge、IBL mipmap 的前提）  
**难度**: 中  
**涉及文件**:

| 文件 | 作用 |
|------|------|
| `src/VulkanContext/Resource/Image/VulkanSampler.h` | VulkanSampler 类定义 |
| `src/VulkanContext/Resource/Image/VulkanSampler.cpp` | `VulkanSampler::Create()` 固定参数采样器 |
| `src/VulkanContext/Resource/VulkanResourceManager.h` | 公共接口 |
| `src/VulkanContext/Resource/VulkanResourceManager.cpp` | 调用 Create |

**当前问题**:
`VulkanSampler::Create()` 所有参数硬编码：
- Filter: Linear
- AddressMode: Repeat（Skybox 需要 ClampToEdge）
- MaxLod: 0.0f（IBL 预滤波需要 mipmap LOD 范围）
- CompareEnable: false（用于阴影 PCF 时可改为 shadow sampler）

**具体步骤**:

### 2.1 定义 SamplerCreateConfig

**文件**: `src/VulkanContext/Resource/Image/VulkanSampler.h`

```cpp
struct VulkanSamplerCreateConfig
{
    vk::Filter magFilter = vk::Filter::eLinear;
    vk::Filter minFilter = vk::Filter::eLinear;
    vk::SamplerMipmapMode mipmapMode = vk::SamplerMipmapMode::eLinear;
    vk::SamplerAddressMode addressModeU = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode addressModeV = vk::SamplerAddressMode::eRepeat;
    vk::SamplerAddressMode addressModeW = vk::SamplerAddressMode::eRepeat;
    float maxAnisotropy = 16.0f;
    bool anisotropyEnable = true;
    float minLod = 0.0f;
    float maxLod = 0.0f;            // 设为 VK_LOD_CLAMP_NONE (1000.0f) 启用 mipmap
    vk::BorderColor borderColor = vk::BorderColor::eIntOpaqueBlack;
    bool compareEnable = false;
    vk::CompareOp compareOp = vk::CompareOp::eAlways;
};
```

### 2.2 新增 CreateFromConfig() 静态方法

**文件**: `src/VulkanContext/Resource/Image/VulkanSampler.cpp`

```cpp
VulkanResourcePtr VulkanSampler::CreateFromConfig(const VulkanSamplerCreateConfig& config)
{
    const auto device = VulkanContext::GetDevice();
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.setMagFilter(config.magFilter)
        .setMinFilter(config.minFilter)
        .setMipmapMode(config.mipmapMode)
        .setAddressModeU(config.addressModeU)
        .setAddressModeV(config.addressModeV)
        .setAddressModeW(config.addressModeW)
        .setAnisotropyEnable(config.anisotropyEnable ? VK_TRUE : VK_FALSE)
        .setMaxAnisotropy(config.maxAnisotropy)
        .setCompareEnable(config.compareEnable ? VK_TRUE : VK_FALSE)
        .setCompareOp(config.compareOp)
        .setMinLod(config.minLod)
        .setMaxLod(config.maxLod)
        .setBorderColor(config.borderColor)
        .setUnnormalizedCoordinates(VK_FALSE);
    
    vk::Sampler sampler = device.createSampler(samplerInfo);
    VulkanSampler* p = new VulkanSampler(sampler);
    return VulkanResourcePtr(p, &SamplerDeleter);
}
```

### 2.3 在 VulkanResourceManager 中暴露

**文件**: `src/VulkanContext/Resource/VulkanResourceManager.h`

```cpp
VulkanSampler* CreateSampler(const VulkanSamplerCreateConfig& config);
```

保持原有的无参 `CreateSampler()` 不变（默认行为）。

**验证**: 编译通过，现有代码不受影响。

---

## 任务 3: 扩展 RenderTarget 支持 Cubemap 和 Mipmap

**优先级**: 🔴 高（IBL 预计算的前提）  
**难度**: 高  
**涉及文件**:

| 文件 | 作用 |
|------|------|
| `src/VulkanContext/RenderTarget/RenderTarget.h` | RenderTarget 类和 RenderTargetConfig |
| `src/VulkanContext/RenderTarget/RenderTarget.cpp` | 图像/视图创建 |

**当前问题**:
`RenderTarget` 固定 `e2D`、1 mip、1 layer。IBL 预计算需要渲染到 cubemap 各面和各 mip 级别。

**具体步骤**:

### 3.1 扩展 RenderTargetConfig

**文件**: `src/VulkanContext/RenderTarget/RenderTarget.h`

在 `RenderTargetConfig` 结构体中添加：

```cpp
struct RenderTargetConfig
{
    // 现有字段...
    uint32_t width = 0;
    uint32_t height = 0;
    vk::Format format;
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
    vk::ImageUsageFlags usage;
    vk::ImageAspectFlags aspectMask;
    bool transient = false;
    
    // 新增字段
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    vk::ImageCreateFlags flags = {};          // eCubeCompatible for cubemap
    vk::ImageViewType viewType = vk::ImageViewType::e2D;
};
```

### 3.2 修改 CreateImage()

**文件**: `src/VulkanContext/RenderTarget/RenderTarget.cpp`

```cpp
void RenderTarget::CreateImage()
{
    // ... 现有 usage 处理不变 ...
    
    vk::ImageCreateInfo imageInfo;
    imageInfo.setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(_config.width, _config.height, 1))
        .setMipLevels(_config.mipLevels)          // 原来是 1
        .setArrayLayers(_config.arrayLayers)       // 原来是 1
        .setFormat(_config.format)
        .setFlags(_config.flags)                   // 新增
        // ... 其余不变 ...
}
```

### 3.3 修改 CreateImageView()

```cpp
void RenderTarget::CreateImageView()
{
    // ...
    viewInfo.setViewType(_config.viewType)          // 原来是 e2D
        .setSubresourceRange(vk::ImageSubresourceRange()
            .setAspectMask(_config.aspectMask)
            .setBaseMipLevel(0)
            .setLevelCount(_config.mipLevels)       // 原来是 1
            .setBaseArrayLayer(0)
            .setLayerCount(_config.arrayLayers));    // 原来是 1
}
```

### 3.4 新增获取单面/单 mip 视图的方法

IBL 预计算需要分面、分 mip 渲染，需要为特定的 (face, mip) 创建独立的 ImageView：

**文件**: `src/VulkanContext/RenderTarget/RenderTarget.h`

```cpp
// 创建指定 face+mip 的 ImageView（用于渲染到 cubemap 的某一面/某一mip）
vk::ImageView CreateSingleLayerMipView(uint32_t arrayLayer, uint32_t mipLevel) const;
```

**文件**: `src/VulkanContext/RenderTarget/RenderTarget.cpp`

```cpp
vk::ImageView RenderTarget::CreateSingleLayerMipView(uint32_t arrayLayer, uint32_t mipLevel) const
{
    vk::ImageViewCreateInfo viewInfo;
    viewInfo.setImage(_image)
        .setViewType(vk::ImageViewType::e2D)  // 单面视为 2D
        .setFormat(_config.format)
        .setSubresourceRange(vk::ImageSubresourceRange()
            .setAspectMask(_config.aspectMask)
            .setBaseMipLevel(mipLevel)
            .setLevelCount(1)
            .setBaseArrayLayer(arrayLayer)
            .setLayerCount(1));
    return VulkanContext::GetDevice().createImageView(viewInfo);
}
```

> **重要**: 调用者负责销毁返回的 ImageView。建议在使用完毕后立即 `device.destroyImageView(view)`。

**验证**:
1. 编译通过，原有 RenderTargetManager 行为不变（新字段默认值与旧行为一致）
2. 创建 cubemap RT: `config.flags = eCubeCompatible, arrayLayers = 6, viewType = eCube`

---

## 任务 4: Skybox 渲染

**优先级**: 🔴 高  
**难度**: 高  
**前置依赖**: 任务 1（VulkanImage cubemap）, 任务 2（Sampler ClampToEdge）  
**涉及文件**:

| 文件 | 作用 | 操作 |
|------|------|------|
| `example/Graphics/Assets/Shader/skybox.vert` | 天空盒顶点着色器 | 新建 |
| `example/Graphics/Assets/Shader/skybox.frag` | 天空盒片元着色器 | 新建 |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | Skybox API | 修改 |
| `src/Systems/RenderSystem/RenderSystem.cpp` | Skybox 初始化 | 修改 |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | 天空盒渲染 pass | 修改 |
| `src/VulkanContext/Pipeline/VulkanPipeline.h` | 可能需要新的 pipeline 构造 | 可能修改 |

**当前问题**:
没有天空盒，背景是纯色 clear color。缺少环境感和深度感。

### 设计方案

天空盒使用**全屏三角形 + inverse VP 矩阵**方式（而非传统的 cube mesh），简单高效：
- 顶点着色器生成全屏三角形（与后处理共用 `fullscreen.vert` 同样的技巧）
- 片元着色器用 inverse(VP) 将屏幕坐标转换为世界空间方向，采样 cubemap
- 深度设为 1.0（最远），在 Forward pass **之后** 渲染，仅填充未被场景覆盖的像素

### 具体步骤

#### 4.1 准备 Skybox Cubemap 资源

放置 6 张天空图到 `example/Graphics/Assets/Texture/Skybox/` 目录：
```
right.jpg   (+X)
left.jpg    (-X)
top.jpg     (+Y)
bottom.jpg  (-Y)
front.jpg   (+Z)
back.jpg    (-Z)
```

或者使用单张 equirectangular HDR `.hdr` 文件，后续通过计算转换为 cubemap（见任务 5 IBL）。

> 简化方案：先使用 6 张 LDR 图片，后续 IBL 任务再支持 HDR equirectangular 转换。

#### 4.2 创建 Skybox 着色器

**文件**: `example/Graphics/Assets/Shader/skybox.vert`（新建）

```glsl
#version 450

layout(push_constant) uniform PushConstants {
    mat4 inverseVP;    // inverse(projection * view)，去掉位移的 view 矩阵
} pushConstants;

layout(location = 0) out vec3 fragWorldDir;

void main()
{
    // 全屏三角形（覆盖整个 NDC）
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 clipPos = vec4(uv * 2.0 - 1.0, 1.0, 1.0);  // z=1.0 = 最远深度
    
    // 从裁剪空间转换为世界空间方向
    vec4 worldPos = pushConstants.inverseVP * clipPos;
    fragWorldDir = worldPos.xyz / worldPos.w;
    
    gl_Position = vec4(uv * 2.0 - 1.0, 1.0, 1.0);   // z=1.0 确保在最远处
}
```

**文件**: `example/Graphics/Assets/Shader/skybox.frag`（新建）

```glsl
#version 450

layout(set = 0, binding = 0) uniform samplerCube skyboxTexture;

layout(location = 0) in vec3 fragWorldDir;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 dir = normalize(fragWorldDir);
    outColor = texture(skyboxTexture, dir);
}
```

#### 4.3 构建 SPIR-V 编译规则

**文件**: `example/Graphics/CMakeLists.txt` 或 shader 编译脚本

确保新增的 `skybox.vert` 和 `skybox.frag` 被编译为 SPIR-V：
```
skybox.vert → ShaderBin/skybox.vert.spv
skybox.frag → ShaderBin/skybox.frag.spv
```

查找现有着色器的 CMake 编译规则（搜索 `glslangValidator` 或 `glslc` 或 `compile_shader` 在 CMakeLists.txt 中），按相同模式添加新文件。

#### 4.4 实现 Skybox 管理类

**文件**: `include/Ailurus/Systems/RenderSystem/Skybox/Skybox.h`（新建）

```cpp
#pragma once
#include <string>
#include <array>
#include <memory>

namespace Ailurus
{
    class VulkanImage;
    class VulkanSampler;
    class VulkanPipeline;
    class VulkanDescriptorSetLayout;
    class VulkanCommandBuffer;
    class VulkanDescriptorAllocator;
    class ShaderLibrary;

    class Skybox
    {
    public:
        Skybox();
        ~Skybox();

        // 从 6 张面图加载 (路径按 +X, -X, +Y, -Y, +Z, -Z 顺序)
        void LoadFromFaces(const std::array<std::string, 6>& facePaths);
        
        // 初始化管线和描述符布局
        void InitPipeline(ShaderLibrary* pShaderLibrary, vk::Format colorFormat);
        
        // 渲染天空盒（在 Forward pass 之后调用）
        void Render(VulkanCommandBuffer* pCmdBuffer, VulkanDescriptorAllocator* pAllocator,
                    const glm::mat4& inverseVP, vk::Extent2D extent);
        
        void Shutdown();
        
        bool IsLoaded() const { return _cubemapImage != nullptr; }

        // 获取 cubemap 的 ImageView（IBL 需要）
        vk::ImageView GetCubemapImageView() const;

    private:
        VulkanImage* _cubemapImage = nullptr;
        VulkanSampler* _cubemapSampler = nullptr;
        std::unique_ptr<VulkanPipeline> _pipeline;
        std::unique_ptr<VulkanDescriptorSetLayout> _descriptorSetLayout;
    };
}
```

**文件**: `src/Systems/RenderSystem/Skybox/Skybox.cpp`（新建）

关键实现点：

```cpp
void Skybox::LoadFromFaces(const std::array<std::string, 6>& facePaths)
{
    // 1. 加载 6 张图 → Image 对象
    // 2. 将 6 张 RGBA 数据拼接到一个连续 buffer
    // 3. 构造 VulkanImageCreateConfig:
    //    config.width/height = 单面尺寸
    //    config.format = eR8G8B8A8Srgb
    //    config.arrayLayers = 6
    //    config.flags = vk::ImageCreateFlagBits::eCubeCompatible
    //    config.viewType = vk::ImageViewType::eCube
    // 4. 调用 VulkanResourceManager::CreateImageFromConfig()
    // 5. 创建 sampler（ClampToEdge）:
    //    VulkanSamplerCreateConfig samplerConfig;
    //    samplerConfig.addressModeU/V/W = eClampToEdge;
    //    _cubemapSampler = resourceManager->CreateSampler(samplerConfig);
}

void Skybox::InitPipeline(ShaderLibrary* pShaderLibrary, vk::Format colorFormat)
{
    // 1. 创建 DescriptorSetLayout: binding 0 = eCombinedImageSampler (fragment)
    // 2. 创建 pipeline:
    //    - 使用 VulkanPipeline 的 post-process 构造函数（全屏三角形, 无顶点输入）
    //    - colorFormat = offscreenColorTarget 的格式 (R16G16B16A16Sfloat)
    //    - pushConstantSize = sizeof(glm::mat4) = 64 bytes
    //    - 关键区别：需要深度测试但不写深度
    //      depthTestEnable = true, depthWriteEnable = false, 
    //      depthCompareOp = eLessOrEqual
    //    ⚠ 注意：现有 post-process pipeline 构造函数**禁用了深度测试**。
    //    需要新增 pipeline 构造或在现有构造中添加参数控制深度行为。
    //    参见下方 4.5 节。
}

void Skybox::Render(VulkanCommandBuffer* pCmdBuffer, VulkanDescriptorAllocator* pAllocator,
                    const glm::mat4& inverseVP, vk::Extent2D extent)
{
    // 1. 分配 descriptor set 并写入 cubemap image+sampler
    // 2. Bind pipeline
    // 3. Push constants: inverseVP (64 bytes)
    // 4. Bind descriptor set
    // 5. Draw 3 vertices (全屏三角形)
}
```

#### 4.5 修改 VulkanPipeline 或新增 Skybox Pipeline 构造

现有 post-process pipeline 构造函数禁用了深度测试。天空盒需要**深度测试开启（pass depth == 1.0）但深度写入关闭**。

**方案 A（推荐）**: 给 post-process 构造函数加可选深度参数

**文件**: `src/VulkanContext/Pipeline/VulkanPipeline.h`

```cpp
// 现有 post-process 构造函数签名
VulkanPipeline(vk::Format colorFormat,
               const StageShaderArray& shaderArray,
               const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
               uint32_t pushConstantSize = 0,
               bool blendEnabled = false);

// 新增深度参数
VulkanPipeline(vk::Format colorFormat,
               const StageShaderArray& shaderArray,
               const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
               uint32_t pushConstantSize,
               bool blendEnabled,
               vk::Format depthFormat,            // eD32Sfloat for skybox, eUndefined for post-process
               bool depthTestEnable = false,
               bool depthWriteEnable = false,
               vk::CompareOp depthCompareOp = vk::CompareOp::eLessOrEqual);
```

**方案 B**: 单独创建 Skybox pipeline（混合场景 pipeline 和 post-process pipeline 的特性）

#### 4.6 在 RenderSystem 中集成天空盒

**文件**: `include/Ailurus/Systems/RenderSystem/RenderSystem.h`

```cpp
// Public
void SetSkybox(const std::array<std::string, 6>& facePaths);
bool HasSkybox() const;

// Private
std::unique_ptr<Skybox> _skybox;
```

**文件**: `src/Systems/RenderSystem/RenderSystem.Render.cpp`

在 `RenderScene()` 的 `RenderPass(Forward)` 之后、PostProcess 之前插入天空盒渲染：

```cpp
// Forward pass (renders to offscreen HDR RT)
RenderPass(RenderPassType::Forward, pCommandBuffer);

// ★ Skybox pass (渲染到同一个 offscreen HDR RT)
// 不需要新的 BeginRendering，因为 Forward pass 已经 EndRendering 了
// 需要重新 BeginRendering 但不清屏 (clearColor = false)
if (_skybox && _skybox->IsLoaded())
{
    auto depthView = pRenderTargetManager->GetDepthImageView();
    pCommandBuffer->BeginRendering(
        offscreenColorView, depthView, /*resolveView*/ nullptr, extent,
        /*clearColor*/ false, /*useDepth*/ true, _clearColor);
    
    // 计算 inverseVP (去掉 view 矩阵的位移部分)
    glm::mat4 view = ...; // camera view matrix with translation zeroed
    glm::mat4 proj = ...; // camera projection matrix
    glm::mat4 inverseVP = glm::inverse(proj * viewRotOnly);
    
    _skybox->Render(pCommandBuffer, pDescriptorAllocator, inverseVP, extent);
    pCommandBuffer->EndRendering();
}

// Transition offscreen → ShaderReadOnly for post-process
```

> **关键**: BeginRendering 时 `clearColor = false`，这样不会覆盖 Forward pass 已经渲染的内容。深度缓冲也不清除，利用 Forward pass 写入的深度来遮挡天空盒。

#### 4.7 在 GlobalUniform 中添加 inverseVP 矩阵

可选方案：也可以通过 push constants 传递 inverseVP（不修改 GlobalUniform），这样更简洁。上述方案已采用 push constants。

**验证**:
1. 在 Main.cpp 中调用 `SetSkybox({"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"})`
2. 运行后背景应显示天空盒而非纯色
3. 旋转相机，天空盒应正确跟随方向变化

---

## 任务 5: IBL — 辐照度和预滤波环境贴图

**优先级**: 🔴 高  
**难度**: 很高  
**前置依赖**: 任务 1, 2, 3, 4  
**涉及文件**:

| 文件 | 作用 | 操作 |
|------|------|------|
| `example/Graphics/Assets/Shader/ibl_irradiance.frag` | 辐照度卷积着色器 | 新建 |
| `example/Graphics/Assets/Shader/ibl_prefilter.frag` | 预滤波环境贴图着色器 | 新建 |
| `example/Graphics/Assets/Shader/ibl_brdf_lut.frag` | BRDF LUT 生成着色器 | 新建 |
| `example/Graphics/Assets/Shader/pbr.frag` | PBR 着色器（IBL 采样） | 修改 |
| `include/Ailurus/Systems/RenderSystem/IBL/IBLManager.h` | IBL 管理类 | 新建 |
| `src/Systems/RenderSystem/IBL/IBLManager.cpp` | IBL 预计算实现 | 新建 |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | IBL 集成 | 修改 |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | descriptor set 绑定 | 修改 |

**当前问题**:
环境光仅是 `ambientColor * ambientStrength * albedo * ao`（常量乘以 albedo），完全没有环境反射、金属高光反射、或基于环境的漫反射。这是当前场景看起来"平"和"假"的核心原因。

### IBL 三要素

| 组件 | 用途 | 分辨率 | 格式 | Mip |
|------|------|--------|------|-----|
| Irradiance Map | 漫反射环境光 | 32×32(每面) | R16G16B16A16Sfloat | 1 |
| Pre-filtered Env Map | 镜面反射（不同粗糙度） | 128×128(每面) | R16G16B16A16Sfloat | 5 级 |
| BRDF LUT | Split-sum 第二部分 | 512×512 | R16G16Sfloat | 1 |

### 具体步骤

#### 5.1 Irradiance Map 卷积

**文件**: `example/Graphics/Assets/Shader/ibl_irradiance.frag`（新建）

```glsl
#version 450

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

layout(push_constant) uniform PushConstants {
    mat4 faceVP;      // 当前面的 VP 矩阵
} pushConstants;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

void main()
{
    // 从 UV 计算世界方向
    vec4 clipPos = vec4(fragUV * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldDir = inverse(pushConstants.faceVP) * clipPos;
    vec3 N = normalize(worldDir.xyz / worldDir.w);
    
    vec3 irradiance = vec3(0.0);
    
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
    
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);
    
    outColor = vec4(irradiance, 1.0);
}
```

#### 5.2 Pre-filtered Environment Map

**文件**: `example/Graphics/Assets/Shader/ibl_prefilter.frag`（新建）

```glsl
#version 450

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

layout(push_constant) uniform PushConstants {
    mat4 faceVP;
    float roughness;
} pushConstants;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

// Hammersley 低差异序列
float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

// GGX 重要性采样
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    return tangent * H.x + bitangent * H.y + N * H.z;
}

void main()
{
    vec4 clipPos = vec4(fragUV * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldDir = inverse(pushConstants.faceVP) * clipPos;
    vec3 N = normalize(worldDir.xyz / worldDir.w);
    vec3 R = N;
    vec3 V = R;

    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    
    for (uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, pushConstants.roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            prefilteredColor += texture(environmentMap, L).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    
    outColor = vec4(prefilteredColor, 1.0);
}
```

#### 5.3 BRDF LUT 生成

**文件**: `example/Graphics/Assets/Shader/ibl_brdf_lut.frag`（新建）

```glsl
#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

float RadicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N) {
    return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return tangent * H.x + bitangent * H.y + N * H.z;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float a = roughness;
    float k = (a * a) / 2.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

void main()
{
    float NdotV = fragUV.x;
    float roughness = fragUV.y;
    
    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;
    
    float A = 0.0;
    float B = 0.0;
    
    vec3 N = vec3(0.0, 0.0, 1.0);
    
    for (uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        
        if (NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    
    outColor = vec4(A, B, 0.0, 1.0);
}
```

#### 5.4 IBLManager 实现

**文件**: `include/Ailurus/Systems/RenderSystem/IBL/IBLManager.h`（新建）

```cpp
#pragma once
#include <memory>

namespace Ailurus
{
    class Skybox;
    class RenderTarget;
    class VulkanImage;
    class VulkanSampler;
    class VulkanPipeline;
    class VulkanDescriptorSetLayout;
    class VulkanCommandBuffer;
    class VulkanDescriptorAllocator;
    class ShaderLibrary;
    class PostProcessPipelineFactory;

    class IBLManager
    {
    public:
        IBLManager();
        ~IBLManager();

        // 调用时机：Skybox 加载之后
        // 从 environment cubemap 预计算 irradiance + prefiltered + BRDF LUT
        void Precompute(const Skybox& skybox, ShaderLibrary* pShaderLibrary);

        // 获取预计算结果 (用于 PBR 着色器绑定)
        vk::ImageView GetIrradianceMapView() const;
        vk::ImageView GetPrefilteredMapView() const;
        vk::ImageView GetBRDFLUTView() const;
        VulkanSampler* GetIrradianceSampler() const;
        VulkanSampler* GetPrefilteredSampler() const;
        VulkanSampler* GetBRDFLUTSampler() const;
        
        bool IsReady() const;
        void Shutdown();

    private:
        void ComputeIrradianceMap(vk::ImageView envCubemapView, VulkanSampler* envSampler,
                                   ShaderLibrary* pShaderLibrary);
        void ComputePrefilteredMap(vk::ImageView envCubemapView, VulkanSampler* envSampler,
                                    ShaderLibrary* pShaderLibrary);
        void ComputeBRDFLUT(ShaderLibrary* pShaderLibrary);

        // Irradiance cubemap (32x32 per face, 6 layers, 1 mip)
        std::unique_ptr<RenderTarget> _irradianceMap;
        VulkanSampler* _irradianceSampler = nullptr;

        // Pre-filtered env cubemap (128x128 per face, 6 layers, 5 mips)
        std::unique_ptr<RenderTarget> _prefilteredMap;
        VulkanSampler* _prefilteredSampler = nullptr;

        // BRDF LUT (512x512, 2D, R16G16Sfloat)
        std::unique_ptr<RenderTarget> _brdfLUT;
        VulkanSampler* _brdfLUTSampler = nullptr;

        bool _ready = false;
    };
}
```

**预计算流程** (`IBLManager::Precompute`):

```
1. ComputeIrradianceMap():
   - 创建 cubemap RenderTarget (32x32, 6 layers, R16G16B16A16Sfloat, eCubeCompatible)
   - 对每面 (0-5):
     a. CreateSingleLayerMipView(face, 0) → 2D ImageView
     b. 构造面 VP 矩阵 (lookAt from origin)
     c. BeginRendering → 全屏三角形 + ibl_irradiance.frag → EndRendering
     d. destroyImageView(临时 view)
   - 创建 ClampToEdge sampler

2. ComputePrefilteredMap():
   - 创建 cubemap RenderTarget (128x128, 6 layers, 5 mips, R16G16B16A16Sfloat, eCubeCompatible)
   - 对每 mip (0-4), roughness = mip / 4.0:
     对每面 (0-5):
       a. CreateSingleLayerMipView(face, mip) → 2D ImageView
       b. 构造面 VP + roughness push constant
       c. BeginRendering(mip 分辨率) → 全屏三角形 + ibl_prefilter.frag → EndRendering
       d. destroyImageView(临时 view)
   - 创建 ClampToEdge sampler, maxLod = 4.0

3. ComputeBRDFLUT():
   - 创建 2D RenderTarget (512x512, R16G16Sfloat)
   - BeginRendering → 全屏三角形 + ibl_brdf_lut.frag → EndRendering
   - 创建 ClampToEdge sampler
```

**6 面 VP 矩阵**:

```cpp
// Cubemap face view matrices (looking from origin)
const glm::mat4 captureViews[] = {
    glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)), // +X
    glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)), // -X
    glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)), // +Y
    glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)), // -Y
    glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)), // +Z
    glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0)), // -Z
};
glm::mat4 captureProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
```

#### 5.5 修改 PBR 着色器集成 IBL

**文件**: `example/Graphics/Assets/Shader/pbr.frag`

新增 IBL 纹理 binding（放在 set=0，因为 IBL 是全局资源，与场景灯光同级）：

```glsl
// 在 shadow map 之后添加（set=0 的新 binding）
layout(set = 0, binding = 5) uniform samplerCube irradianceMap;
layout(set = 0, binding = 6) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 7) uniform sampler2D brdfLUT;
```

> **⚠ binding 编号**: 现有 set=0 的 binding 分配是 0=GlobalUniform, 1-4=shadowMap0-3。新增从 binding 5 开始。

新增 Fresnel 粗糙度版本：
```glsl
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
```

修改 main() 中的环境光计算（替换现有的 `vec3 ambient = ...` 部分）：
```glsl
// IBL 漫反射
vec3 F_ibl = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
vec3 kS_ibl = F_ibl;
vec3 kD_ibl = (1.0 - kS_ibl) * (1.0 - metallic);
vec3 irradiance = texture(irradianceMap, N).rgb;
vec3 diffuseIBL = kD_ibl * irradiance * albedo;

// IBL 镜面反射
const float MAX_REFLECTION_LOD = 4.0;
vec3 R = reflect(-V, N);
vec3 prefilteredColor = textureLod(prefilteredMap, R, roughness * MAX_REFLECTION_LOD).rgb;
vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
vec3 specularIBL = prefilteredColor * (F_ibl * brdf.x + brdf.y);

// 最终环境光（替换原来的常量环境光）
vec3 ambient = (diffuseIBL + specularIBL) * ao;
// 保留乘以 ambientColor 的选项（用于全局调色）
ambient *= globalUniform.ambientColor.rgb * globalUniform.ambientColor.w;
```

#### 5.6 修改 GlobalUniform set=0 的 descriptor set layout

**文件**: `src/Systems/RenderSystem/RenderSystem.cpp`

在构建 globalUniformSet 时，添加 IBL 纹理的 binding：

```cpp
// 现有的 texture bindings (binding 1-4 = shadow maps)
std::vector<TextureBindingInfo> textureBindings = {
    {1, vk::ShaderStageFlagBits::eFragment}, // shadowMap0
    {2, vk::ShaderStageFlagBits::eFragment}, // shadowMap1
    {3, vk::ShaderStageFlagBits::eFragment}, // shadowMap2
    {4, vk::ShaderStageFlagBits::eFragment}, // shadowMap3
    // 新增 IBL
    {5, vk::ShaderStageFlagBits::eFragment}, // irradianceMap
    {6, vk::ShaderStageFlagBits::eFragment}, // prefilteredMap
    {7, vk::ShaderStageFlagBits::eFragment}, // brdfLUT
};
```

#### 5.7 在渲染时绑定 IBL 纹理到 descriptor set

**文件**: `src/Systems/RenderSystem/RenderSystem.Render.cpp`

在 `UpdateGlobalUniformBuffer()` 中，写入 IBL 纹理到 descriptor set：

```cpp
if (_iblManager && _iblManager->IsReady())
{
    descriptorWriter.WriteImage(5, _iblManager->GetIrradianceMapView(),
                                _iblManager->GetIrradianceSampler()->GetSampler());
    descriptorWriter.WriteImage(6, _iblManager->GetPrefilteredMapView(),
                                _iblManager->GetPrefilteredSampler()->GetSampler());
    descriptorWriter.WriteImage(7, _iblManager->GetBRDFLUTView(),
                                _iblManager->GetBRDFLUTSampler()->GetSampler());
}
```

> **⚠ 注意**: 如果 IBL 未加载，需要绑定一个默认的 1x1 cubemap/2D 纹理（与 default_normal.png 类似的思路），否则着色器可能采样到未初始化的内存。

**验证**:
1. 加载 Skybox → 自动触发 IBL 预计算
2. 金属表面应该反射环境；粗糙表面有模糊反射
3. 关闭所有直接光源，场景仅靠 IBL 照亮，物体应有正确的明暗关系

---

## 任务 6: 阴影偏移改进

**优先级**: 🟡 中  
**难度**: 中  
**前置依赖**: 无  
**涉及文件**:

| 文件 | 作用 | 操作 |
|------|------|------|
| `example/Graphics/Assets/Shader/pbr.frag` | 阴影采样和偏移 | 修改 |
| `example/Graphics/Assets/Shader/shadow.vert` | 阴影深度渲染 | 可能修改 |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | bias API | 修改 |
| `src/Systems/RenderSystem/RenderSystem.cpp` | BuildGlobalUniform | 修改 |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | bias 上传 | 修改 |

**当前问题**:
- `pbr.frag` 中 `sampleShadowMap()` 使用硬编码 bias `0.005`
- 远处级联因为分辨率低、同一 bias 值会产生阴影条纹 (shadow acne)
- 近处级联同一 bias 值又可能造成 peter-panning（阴影脱离物体）
- 没有根据光照角度做斜率缩放 (slope-scale bias)

### 具体步骤

#### 6.1 在 GlobalUniform 中添加 shadow bias 参数

**文件**: 三个着色器 `pbr.frag`, `pbr.vert`, `shadow.vert`

在 GlobalUniform 末尾 (`vec4 ambientColor` 之后) 添加：

```glsl
vec4 shadowBiasParams;  // x = constantBias, y = slopeBias, z = normalOffsetScale, w = unused
```

> **⚠ 重要**: 三个着色器中的 GlobalUniform 必须完全一致。

#### 6.2 C++ 侧添加 bias 成员和 API

**文件**: `include/Ailurus/Systems/RenderSystem/RenderSystem.h`

```cpp
// Public
void SetShadowBias(float constantBias, float slopeBias);
void SetShadowNormalOffset(float normalOffsetScale);

// Private
float _shadowConstantBias = 0.002f;
float _shadowSlopeBias = 0.005f;
float _shadowNormalOffset = 0.02f;
```

**文件**: `src/Systems/RenderSystem/RenderSystem.cpp`

- `BuildGlobalUniform()` 添加 `shadowBiasParams` Vector4 成员
- 实现 `SetShadowBias()` 和 `SetShadowNormalOffset()`

**文件**: `src/Systems/RenderSystem/RenderSystem.Render.cpp`

- `UpdateGlobalUniformBuffer()` 上传 `Vector4f(_shadowConstantBias, _shadowSlopeBias, _shadowNormalOffset, 0.0f)`

#### 6.3 修改 pbr.frag 阴影采样

**文件**: `example/Graphics/Assets/Shader/pbr.frag`

替换 `sampleShadowMap()` 中的硬编码 bias：

```glsl
float sampleShadowMap(sampler2D shadowMap, vec2 uv, float compareDepth, float bias) {
    // ... 现有 UV 边界检查不变 ...
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float depth = texture(shadowMap, uv + offset).r;
            shadow += (compareDepth - bias > depth) ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}
```

修改 `calculateShadow()` 函数：

```glsl
float calculateShadow(vec3 worldPos, vec3 viewPos, vec3 N, vec3 L) {
    float viewDepth = length(viewPos);
    int cascadeIndex = selectCascade(viewDepth);
    
    vec4 lightSpacePos = globalUniform.cascadeViewProjMatrices[cascadeIndex] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    
    // ★ 动态 bias
    float constantBias = globalUniform.shadowBiasParams.x;
    float slopeBias = globalUniform.shadowBiasParams.y;
    float cosTheta = max(dot(N, L), 0.0);
    float slopeScale = sqrt(1.0 - cosTheta * cosTheta) / max(cosTheta, 0.001);
    
    // 每级联距离越远, 需要的 bias 越大
    float cascadeScale = float(cascadeIndex + 1);
    float bias = constantBias * cascadeScale + slopeBias * slopeScale;
    
    // ... 现有的 shadow map 选择分支, 改用新的 bias 参数 ...
    if (cascadeIndex == 0) shadow = sampleShadowMap(shadowMap0, projCoords.xy, currentDepth, bias);
    // ... 其余同理 ...
    
    return shadow;
}
```

#### 6.4 法线偏移 (Normal Offset)

在 `calculateShadow()` 中添加法线偏移，在计算光空间位置之前偏移世界坐标：

```glsl
// 法线偏移 — 沿法线方向偏移采样位置, 减少自阴影
float normalOffsetScale = globalUniform.shadowBiasParams.z;
vec3 offsetPos = worldPos + N * normalOffsetScale * (1.0 - max(dot(N, L), 0.0));
vec4 lightSpacePos = globalUniform.cascadeViewProjMatrices[cascadeIndex] * vec4(offsetPos, 1.0);
```

#### 6.5 更新 main() 中的 calculateShadow 调用

当前：
```glsl
float shadow = calculateShadow(fragWorldPos, viewPos);
```

改为：
```glsl
vec3 mainLightDir = normalize(-globalUniform.dirLightDirections[0].xyz);
float shadow = calculateShadow(fragWorldPos, viewPos, N, mainLightDir);
```

**验证**:
1. 默认参数下阴影无明显条纹
2. 在 Main.cpp 中调用 `SetShadowBias(0.001f, 0.003f)` 微调
3. 掠射角度下（光线几乎平行于表面）无严重 shadow acne

---

## 任务 7: Gamma 正确纹理采样

**优先级**: 🟡 中  
**难度**: 中  
**前置依赖**: 无  
**涉及文件**:

| 文件 | 作用 | 操作 |
|------|------|------|
| `src/VulkanContext/Resource/Image/VulkanImage.cpp` | 图像创建，格式选择 | 修改 |
| `include/Ailurus/Utility/Image.h` | Image 类 | 可能修改 |
| `src/Systems/AssetsSystem/AssetsSystem.Material.cpp` | 材质贴图加载 | 修改 |
| `example/Graphics/Assets/Material/PBRMaterial.json` | 贴图类型标注 | 修改 |

**当前问题**:
`VulkanImage::Create()` 对**所有**纹理都用 `eR8G8B8A8Srgb` 格式。这对颜色贴图 (albedo) 是正确的（sRGB → 线性空间自动转换），但对数据纹理（法线贴图、roughness 贴图、AO 贴图）是**错误的**，因为：
- 法线贴图存储的是线性数据（方向向量），sRGB 解码会扭曲数值
- 粗糙度/金属度/AO 贴图同理，是线性数值

### 具体步骤

#### 7.1 区分颜色纹理和数据纹理

**方案 A（JSON 标注，推荐）**: 在材质 JSON 中标注纹理类型

**文件**: `example/Graphics/Assets/Material/PBRMaterial.json`

```json
{
    "binding": 1,
    "uniformVarName": "albedoTexture",
    "path": "./Assets/Texture/wall.jpg",
    "srgb": true                          // ← 新增字段, 默认 true
},
{
    "binding": 2,
    "uniformVarName": "normalTexture",
    "path": "./Assets/Texture/default_normal.png",
    "srgb": false                         // ← 法线贴图用线性格式
}
```

**方案 B（命名约定）**: 根据 uniformVarName 自动判断
- 包含 "normal"、"roughness"、"metallic"、"ao"、"height" → 线性 (`eR8G8B8A8Unorm`)
- 其他 → sRGB (`eR8G8B8A8Srgb`)

#### 7.2 扩展纹理加载传递格式信息

**文件**: `src/Systems/AssetsSystem/AssetsSystem.Material.cpp`

在 `JsonReadTextures()` 中：

```cpp
// 解析 srgb 字段 (默认 true)
bool isSrgb = true;
if (textureConfig.contains("srgb"))
    isSrgb = textureConfig["srgb"].get<bool>();

// 选择格式
vk::Format format = isSrgb ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm;
```

#### 7.3 VulkanImage::Create() 支持指定格式

**方案 A（推荐）**: 使用任务 1 中已经实现的 `CreateImageFromConfig()`

```cpp
VulkanImageCreateConfig config;
config.width = image.GetPixelSize().first;
config.height = image.GetPixelSize().second;
config.format = isSrgb ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm;
auto* pImage = resourceManager->CreateImageFromConfig(config, image.GetBytesData(), imageSize);
```

**方案 B**: 如果不想依赖任务 1，给原有 `CreateImage` 加一个 format 参数：

```cpp
VulkanImage* CreateImage(const Image& image, vk::Format format = vk::Format::eR8G8B8A8Srgb);
```

只需在 `VulkanImage::Create()` 中将硬编码的 `eR8G8B8A8Srgb` 替换为参数。

#### 7.4 验证

**重要检查**: 修改前后对比
- albedo 纹理：应保持不变（已经正确使用 sRGB）
- normal 纹理：改为 `eR8G8B8A8Unorm` 后，法线方向应更准确（之前 sRGB 解码会导致法线偏差）
- 新增的 roughness/AO 纹理（如有）：应使用 `Unorm`

**验证步骤**:
1. 修改 `PBRMaterial.json` 中 `normalTexture` 的 `srgb` 为 `false`
2. 编译运行，法线贴图效果应更准确（高光位置更正确）
3. albedo 效果无变化

---

## 任务 8: SSAO（屏幕空间环境光遮蔽）

**优先级**: 🟡 中  
**难度**: 高  
**前置依赖**: 无（独立于 Skybox/IBL）  
**涉及文件**:

| 文件 | 作用 | 操作 |
|------|------|------|
| `example/Graphics/Assets/Shader/PostProcess/ssao.frag` | SSAO 着色器 | 新建 |
| `example/Graphics/Assets/Shader/PostProcess/ssao_blur.frag` | SSAO 模糊着色器 | 新建 |
| `include/Ailurus/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.h` | SSAO 特效类 | 新建 |
| `src/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.cpp` | SSAO 实现 | 新建 |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | SSAO 开关 API | 修改 |
| `src/Systems/RenderSystem/RenderSystem.cpp` | 注册 SSAO 效果 | 修改 |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | 传递深度纹理给 SSAO | 修改 |
| `example/Graphics/Assets/Shader/pbr.frag` | AO 混合 | 修改 |

**当前问题**:
材质的 `ao` 值是标量常量，没有屏幕空间 AO。接触面、角落、凹陷处应该更暗，但目前完全没有。

### 设计方案

SSAO 有两种集成方式：

**方式 A（推荐 — 简单）: 作为 PostProcess 在 Forward pass 之后、Bloom 之前执行**
- SSAO 输出一张灰度 AO 纹理
- 在 Forward pass 输出上混合（乘以 AO）
- 优点：不需要修改 Forward pass 着色器
- 缺点：SSAO 在 tone mapping 之前，AO 影响了高光

**方式 B（更正确 — 复杂）: 在 Forward pass 中采样 SSAO 纹理**
- 需要预先渲染 SSAO（在 Forward pass 之前需要深度）
- 需要 depth pre-pass 或上一帧的深度
- Forward pass 的 pbr.frag 中混合 AO

**选择方式 A**（起步简单，后续可升级到方式 B）。

### 具体步骤

#### 8.1 SSAO 着色器

**文件**: `example/Graphics/Assets/Shader/PostProcess/ssao.frag`（新建）

```glsl
#version 450

layout(set = 0, binding = 0) uniform sampler2D sceneDepth;     // 深度纹理
layout(set = 0, binding = 1) uniform sampler2D noiseTexture;    // 4x4 随机旋转向量

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 inverseProjection;
    vec4 samples[64];    // 随机采样核
    vec2 noiseScale;     // viewport / noise_size
    float radius;        // 采样半径
    float bias;          // 深度偏移
} pc;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

// 从深度重建视空间位置
vec3 reconstructViewPos(vec2 uv) {
    float depth = texture(sceneDepth, uv).r;
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = pc.inverseProjection * clipPos;
    return viewPos.xyz / viewPos.w;
}

void main()
{
    vec3 fragPos = reconstructViewPos(fragUV);
    
    // 通过深度差分估算法线
    vec3 dPosX = reconstructViewPos(fragUV + vec2(1.0 / textureSize(sceneDepth, 0).x, 0)) - fragPos;
    vec3 dPosY = reconstructViewPos(fragUV + vec2(0, 1.0 / textureSize(sceneDepth, 0).y)) - fragPos;
    vec3 normal = normalize(cross(dPosX, dPosY));
    
    // TBN 从噪声纹理
    vec3 randomVec = texture(noiseTexture, fragUV * pc.noiseScale).xyz;
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    float occlusion = 0.0;
    int sampleCount = 64;
    
    for (int i = 0; i < sampleCount; i++)
    {
        // 采样点（视空间）
        vec3 samplePos = fragPos + TBN * pc.samples[i].xyz * pc.radius;
        
        // 投影到屏幕
        vec4 offset = pc.projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        // 比较深度
        float sampleDepth = reconstructViewPos(offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, pc.radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + pc.bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1.0 - (occlusion / float(sampleCount));
    outColor = vec4(vec3(occlusion), 1.0);
}
```

> **⚠ Push constant 大小限制**: Vulkan 保证至少 128 bytes push constants。上面的 shader 使用 64 个 vec4 samples + 其他参数会超出限制。**应改为 UBO 传递 samples**，push constants 只放 matrices + params。
>
> 修改方案：
> ```glsl
> layout(set = 0, binding = 2) uniform SSAOParams {
>     vec4 samples[64];
> } ssaoParams;
> 
> layout(push_constant) uniform PushConstants {
>     mat4 projection;
>     mat4 inverseProjection;
>     vec2 noiseScale;
>     float radius;
>     float bias;
> } pc;
> ```

#### 8.2 SSAO 模糊着色器

**文件**: `example/Graphics/Assets/Shader/PostProcess/ssao_blur.frag`（新建）

```glsl
#version 450

layout(set = 0, binding = 0) uniform sampler2D ssaoInput;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    
    // 4x4 box blur
    for (int x = -2; x < 2; x++)
    {
        for (int y = -2; y < 2; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, fragUV + offset).r;
        }
    }
    result /= 16.0;
    
    outColor = vec4(vec3(result), 1.0);
}
```

#### 8.3 SSAOEffect 类

**文件**: `include/Ailurus/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.h`（新建）

```cpp
#pragma once
#include "Ailurus/Systems/RenderSystem/PostProcess/PostProcessEffect.h"
#include "Ailurus/Systems/RenderSystem/PostProcess/RTHandle.h"
#include <memory>
#include <glm/glm.hpp>

namespace Ailurus
{
    class SSAOEffect : public PostProcessEffect
    {
    public:
        SSAOEffect();
        ~SSAOEffect() override;
        
        const std::string& GetName() const override;
        void Init(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
                  uint32_t width, uint32_t height, vk::Format format) override;
        void Render(VulkanCommandBuffer* pCmdBuffer, vk::ImageView inputImageView,
                    vk::ImageView outputImageView, vk::Extent2D extent,
                    VulkanDescriptorAllocator* pDescriptorAllocator) override;
        void OnResize(PostProcessResourcePool& pool, PostProcessPipelineFactory& factory,
                      uint32_t width, uint32_t height, vk::Format format) override;
        void Shutdown() override;
        
        void SetRadius(float radius) { _radius = radius; }
        void SetBias(float bias) { _bias = bias; }
        void SetIntensity(float intensity) { _intensity = intensity; }
        
        // 必须在每帧渲染前设置
        void SetDepthImageView(vk::ImageView depthView) { _depthImageView = depthView; }
        void SetProjectionMatrix(const glm::mat4& proj) { _projection = proj; }

    private:
        static const std::string EFFECT_NAME;
        
        float _radius = 0.5f;
        float _bias = 0.025f;
        float _intensity = 1.0f;
        
        // SSAO pass resources
        std::unique_ptr<VulkanPipeline> _ssaoPipeline;
        std::unique_ptr<VulkanDescriptorSetLayout> _ssaoDescriptorLayout;
        RTHandle* _ssaoRT = nullptr;          // 半分辨率 AO 纹理
        
        // Blur pass resources
        std::unique_ptr<VulkanPipeline> _blurPipeline;
        std::unique_ptr<VulkanDescriptorSetLayout> _blurDescriptorLayout;
        RTHandle* _blurRT = nullptr;          // 模糊后的 AO 纹理
        
        // Noise texture (4x4 random rotations)
        VulkanImage* _noiseTexture = nullptr;
        VulkanSampler* _noiseSampler = nullptr;
        
        // Sample kernel (vec3 hemisphere samples)
        std::vector<glm::vec4> _sampleKernel;
        
        // Per-frame state
        vk::ImageView _depthImageView;
        glm::mat4 _projection;
        
        void GenerateSampleKernel();
        void GenerateNoiseTexture();
    };
}
```

#### 8.4 SSAOEffect 集成

**文件**: `src/Systems/RenderSystem/RenderSystem.cpp`

SSAO 需要在 Bloom/ToneMapping **之前** 执行：

```cpp
// 现有效果注册顺序
_postProcessChain->AddEffect<SSAOEffect>();       // 新增: SSAO 先执行
_postProcessChain->AddEffect<BloomMipChainEffect>();
_postProcessChain->AddEffect<ToneMappingEffect>();
```

SSAO 效果的 `Render()` 流程：
```
1. 渲染 SSAO pass → _ssaoRT (半分辨率灰度)
   input: depth texture + noise texture + samples UBO
   output: raw AO 纹理

2. 渲染 Blur pass → _blurRT
   input: _ssaoRT
   output: 模糊 AO 纹理

3. 渲染 Composite pass → outputImageView
   input: inputImageView (场景 HDR) + _blurRT (AO)
   output: scene * AO 混合结果
```

> **⚠ 每帧传递深度**: SSAO 需要深度纹理。在 `RenderSystem::RenderScene()` 中，Forward pass 结束后、PostProcess 执行前，设置：
> ```cpp
> if (auto* ssao = dynamic_cast<SSAOEffect*>(_postProcessChain->GetEffect("SSAO")))
> {
>     ssao->SetDepthImageView(pRenderTargetManager->GetDepthImageView());
>     ssao->SetProjectionMatrix(projectionMatrix);
> }
> ```

#### 8.5 Composite 着色器（SSAO 混合到场景）

**文件**: `example/Graphics/Assets/Shader/PostProcess/ssao_composite.frag`（新建）

```glsl
#version 450

layout(set = 0, binding = 0) uniform sampler2D sceneColor;  // 输入场景
layout(set = 0, binding = 1) uniform sampler2D ssaoTexture;  // AO 纹理

layout(push_constant) uniform PushConstants {
    float intensity;
} pc;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 color = texture(sceneColor, fragUV).rgb;
    float ao = texture(ssaoTexture, fragUV).r;
    ao = mix(1.0, ao, pc.intensity);
    outColor = vec4(color * ao, 1.0);
}
```

**验证**:
1. SSAO 效果开启后，角落和接触面应有明暗变化
2. `SetIntensity(0.0f)` 应完全禁用 AO 效果
3. `SetRadius(1.0f)` 增大采样范围，AO 更柔和
4. 性能检查：半分辨率 SSAO + 4x4 blur 应在中等 GPU 上 < 1ms

---

## 实施建议

### 推荐执行顺序

```
阶段 A (基础设施): 任务 1 + 2 + 3 → 同时进行, 互不依赖
                    ⇩
阶段 B (天空渲染): 任务 4 (Skybox)
                    ⇩
阶段 C (IBL):     任务 5 (IBL 预计算 + PBR 集成)
                    ⇩
阶段 D (独立改进): 任务 6 + 7 + 8 → 互不依赖, 可按任意顺序
```

### 每个任务完成后的验证清单

1. **编译**: C++ 编译零错误零警告
2. **着色器**: 所有 GLSL → SPIR-V 编译通过
3. **运行**: 场景正常渲染，无崩溃
4. **视觉**: 效果符合预期（对比截图）
5. **回归**: 现有功能不受影响
6. **性能**: 无明显帧率下降（目标 60fps）

### GlobalUniform 修改检查表

每次修改 GlobalUniform **必须**同步以下文件：
- [ ] `example/Graphics/Assets/Shader/pbr.frag`
- [ ] `example/Graphics/Assets/Shader/pbr.vert`
- [ ] `example/Graphics/Assets/Shader/shadow.vert`
- [ ] `src/Systems/RenderSystem/RenderSystem.cpp` → `BuildGlobalUniform()`
- [ ] `src/Systems/RenderSystem/RenderSystem.Render.cpp` → `UpdateGlobalUniformBuffer()`
- [ ] `include/Ailurus/Systems/RenderSystem/RenderSystem.h` → 静态常量和成员

---

## 实施记录

> 完成后在此记录实际做了什么、遇到的问题和解决方案。

### 任务 1 实施: 扩展 VulkanImage
✅ **已完成**

**修改文件**:
| 文件 | 修改内容 |
|------|----------|
| `src/VulkanContext/Resource/Image/VulkanImage.h` | 新增 `VulkanImageCreateConfig` 结构体（width, height, format, mipLevels, arrayLayers, flags, viewType, usage, aspectMask）；VulkanImage 类新增 `_mipLevels`、`_arrayLayers`、`_viewType` 成员及对应 getter；扩展构造函数接收新参数；声明 `CreateFromConfig()` 静态方法 |
| `src/VulkanContext/Resource/Image/VulkanImage.cpp` | 实现 `CreateFromConfig(const VulkanImageCreateConfig&, const void*, size_t)`，支持 cubemap 多层上传（当 arrayLayers > 1 时使用 6 个 `vk::BufferImageCopy` region 分别上传）；支持任意格式、flags、usage |
| `include/Ailurus/Utility/Image.h` | 新增 `_hdrData`（vector\<float\>）、`_isHDR`、`_channels` 成员；新增 `IsHDR()`、`GetHDRData()`、`GetChannels()` 方法 |
| `src/Utility/Image.cpp` | 在构造函数中通过 `stbi_is_hdr()` 检测 HDR 文件，使用 `stbi_loadf()` 加载为 float RGBA 数据（强制 4 通道） |
| `src/VulkanContext/Resource/VulkanResourceManager.h` | 新增 `#include` VulkanImage.h；声明 `CreateImageFromConfig(config, pixelData, dataSize)` |
| `src/VulkanContext/Resource/VulkanResourceManager.cpp` | 实现 `CreateImageFromConfig()`，调用 `VulkanImage::CreateFromConfig()` 并存入 `_resources` |

**设计决策**:
- 保留原有 `VulkanImage::Create(const Image&)` 和 `CreateImage(const Image&)` 不变，确保现有材质加载流程零影响
- `CreateFromConfig` 仅供 Skybox、IBL 等高级功能使用
- Cubemap 上传采用 6 个 BufferImageCopy region 按 baseArrayLayer 0-5 分别上传

---

### 任务 2 实施: 扩展 VulkanSampler
✅ **已完成**

**修改文件**:
| 文件 | 修改内容 |
|------|----------|
| `src/VulkanContext/Resource/Image/VulkanSampler.h` | 新增 `VulkanSamplerCreateConfig` 结构体（magFilter, minFilter, mipmapMode, addressModeU/V/W, maxAnisotropy, anisotropyEnable, minLod, maxLod, borderColor, compareEnable, compareOp）；声明 `CreateFromConfig()` 静态方法 |
| `src/VulkanContext/Resource/Image/VulkanSampler.cpp` | 实现 `CreateFromConfig(const VulkanSamplerCreateConfig&)`，从 config 创建 vk::Sampler |
| `src/VulkanContext/Resource/VulkanResourceManager.h` | 新增 `#include` VulkanSampler.h；声明 `CreateSampler(const VulkanSamplerCreateConfig& config)` |
| `src/VulkanContext/Resource/VulkanResourceManager.cpp` | 实现 `CreateSampler(config)`，调用 `VulkanSampler::CreateFromConfig()` 并存入 `_resources` |

**设计决策**:
- 保留原有无参 `CreateSampler()` 不变（默认 Linear/Repeat/maxAniso=16/maxLod=0 行为）
- 新接口支持 ClampToEdge（Skybox 需要）、mipmap LOD 范围（IBL 需要）、shadow sampler（compareEnable）等场景

---

### 任务 3 实施: 扩展 RenderTarget
✅ **已完成**

**修改文件**:
| 文件 | 修改内容 |
|------|----------|
| `src/VulkanContext/RenderTarget/RenderTarget.h` | `RenderTargetConfig` 新增 `mipLevels = 1`、`arrayLayers = 1`、`flags = {}`、`viewType = e2D` 字段；RenderTarget 类声明 `CreateSingleLayerMipView(arrayLayer, mipLevel)` 方法 |
| `src/VulkanContext/RenderTarget/RenderTarget.cpp` | `CreateImage()` 使用 config 的 mipLevels/arrayLayers/flags 创建 VkImage；`CreateImageView()` 使用 config 的 viewType/mipLevels/arrayLayers；新增 `CreateSingleLayerMipView()` 实现——为指定 face+mip 创建独立的 2D ImageView |

**设计决策**:
- 所有新增字段均有与旧行为一致的默认值，确保现有 RenderTargetManager 行为不变
- `CreateSingleLayerMipView()` 返回的 ImageView 需由调用者负责销毁（IBL 预计算用，渲染后立即 `device.destroyImageView(view)`）
- 支持 cubemap RT：`config.flags = eCubeCompatible, arrayLayers = 6, viewType = eCube`

---

### 任务 4 实施: Skybox 渲染
✅ **已完成**

**新建文件**:
| 文件 | 内容 |
|------|------|
| `example/Graphics/Assets/Shader/skybox.vert` | 天空盒顶点着色器——使用 gl_VertexIndex 生成全屏三角形，通过 `inverseViewProjection` push constant 将裁剪空间坐标转换为世界空间方向，输出 z=1.0（最远深度） |
| `example/Graphics/Assets/Shader/skybox.frag` | 天空盒片元着色器——使用 `samplerCube`（set=0, binding=0）采样归一化方向 |
| `src/Systems/RenderSystem/Skybox/Skybox.h` | Skybox 类头文件，声明 Init/Render/RebuildPipeline/Shutdown 公共方法及私有 GenerateGradientCubemap/CreateDescriptorResources/CreatePipeline 辅助方法 |
| `src/Systems/RenderSystem/Skybox/Skybox.cpp` | Skybox 完整实现 |

**修改文件**:
| 文件 | 修改内容 |
|------|----------|
| `src/VulkanContext/Pipeline/VulkanPipeline.h` | 新增第三个构造函数：`VulkanPipeline(colorFormat, depthFormat, shaderArray, descriptorSetLayouts, pushConstantSize)`，面向 Skybox 用途——无顶点输入、深度测试只读（LessOrEqual + 写入禁用）、MSAA 感知、顶点阶段 push constants |
| `src/VulkanContext/Pipeline/VulkanPipeline.cpp` | 实现第三个构造函数：无顶点输入、无背面剔除、MSAA 从 `VulkanContext::GetMSAASamples()` 获取、深度测试启用（LessOrEqual）/ 写入禁用、dynamic rendering 附带深度附件 |
| `include/Ailurus/Systems/RenderSystem/RenderSystem.h` | 新增 `class Skybox` 前向声明；新增 `SetSkyboxEnabled(bool)` / `IsSkyboxEnabled()` 公共方法；新增 `RenderSkybox(VulkanCommandBuffer*)` 私有方法；新增 `std::unique_ptr<Skybox> _pSkybox` 和 `bool _skyboxEnabled = true` 成员 |
| `src/Systems/RenderSystem/RenderSystem.cpp` | 添加 `#include "Skybox/Skybox.h"`；构造函数中初始化 Skybox（enable3D 时创建并调用 Init，传入 R16G16B16A16Sfloat + D32Sfloat 格式）；析构函数中 Skybox 在 PostProcess 之前 Shutdown；实现 getter/setter 方法；`RebuildSwapChain()` 中重建 Skybox pipeline |
| `src/Systems/RenderSystem/RenderSystem.Render.cpp` | 添加 `#include "Skybox/Skybox.h"`；在 `RenderPass()` 的 Forward pass 中，网格循环结束后、`EndRendering()` 之前调用 `RenderSkybox()`；实现 `RenderSkybox()`——从摄像机获取 view 矩阵（去掉平移分量）和 projection 矩阵，计算 `inverse(proj * viewRotOnly)` 作为 push constant |

**Skybox 渲染方案**:
- 采用全屏三角形 + inverse VP 矩阵方式（而非传统 cube mesh），简单高效
- 渲染时机：在 Forward pass **内部**、所有网格之后渲染（共享同一个 BeginRendering/EndRendering 块），利用已有深度缓冲遮挡天空盒
- 深度行为：测试启用（LessOrEqual）+ 写入禁用，顶点着色器输出 z=1.0（NDC 最远深度），仅填充未被场景覆盖的像素

**Skybox 纹理**:
- 使用程序化生成的渐变色 cubemap 作为占位纹理（64×64 每面）
- 渐变配色：天顶蓝 (0.1, 0.2, 0.5) → 地平线淡蓝 (0.5, 0.7, 1.0) → 地面棕 (0.3, 0.2, 0.1)
- 通过 `VulkanResourceManager::CreateImageFromConfig()` 创建（flags = eCubeCompatible, arrayLayers = 6, viewType = eCube）
- Sampler 使用 Linear filter + ClampToEdge 地址模式

**Pipeline 设计**:
- 新增第三类 VulkanPipeline 构造（与场景 pipeline、后处理 pipeline 并列）
- 无顶点输入（全屏三角形仅用 gl_VertexIndex）
- 使用 `PushConstantModelMatrix`（已有的 64 bytes mat4 结构体）复用传递 inverseVP 矩阵
- descriptor set layout：1 个 combined image sampler（cubemap）
- descriptor pool/set 在 Skybox 类内部管理（非每帧分配）

**遇到的问题与解决方案**:
1. **unique_ptr\<VulkanPipeline\> 不完整类型错误**：Skybox.h 中前向声明 VulkanPipeline 导致 unique_ptr 析构时类型不完整。解决方案：改为 `#include <VulkanContext/Pipeline/VulkanPipeline.h>`
2. **CMake 未发现新源文件**：`GLOB_RECURSE ./src/*.cpp` 使用缓存的文件列表，新增 Skybox.cpp 后需要重新运行 `cmake -S . -B build` 刷新
3. **Shader SPIR-V 编译**：skybox.vert 和 skybox.frag 由 `shader_gen.py` 自动发现并编译为 skybox.vert.spv / skybox.frag.spv，无需手动配置

### 任务 5 实施: IBL

**新建文件**:
- `example/Graphics/Assets/Shader/ibl_irradiance.frag` — 辐照度卷积着色器（半球采样累积环境漫反射）
- `example/Graphics/Assets/Shader/ibl_prefilter.frag` — 预滤波环境贴图着色器（GGX 重要性采样，5 级 mip 对应不同粗糙度）
- `example/Graphics/Assets/Shader/ibl_brdf_lut.frag` — BRDF LUT 生成着色器（Split-sum 近似的第二部分）
- `include/Ailurus/Systems/RenderSystem/IBL/IBLManager.h` — IBL 管理类声明
- `src/Systems/RenderSystem/IBL/IBLManager.cpp` — IBL 预计算实现（one-shot command buffer 模式）

**修改文件**:
- `example/Graphics/Assets/Shader/pbr.frag` — 新增 IBL bindings（set=0, binding 5=irradianceMap, binding 6=prefilteredMap, binding 7=brdfLUT），添加 `fresnelSchlickRoughness()` 函数，IBL 环境光替代原有常量环境光（漫反射 irradiance + 镜面反射 prefiltered + BRDF LUT）
- `src/Systems/RenderSystem/Skybox/Skybox.h` — 新增 `GetCubemapImageView()` 和 `GetCubemapSampler()` 接口供 IBL 使用
- `src/Systems/RenderSystem/Skybox/Skybox.cpp` — 实现上述 getter
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h` — 新增 `_pIBLManager` 成员
- `src/Systems/RenderSystem/RenderSystem.cpp` — 构造函数中创建 IBLManager 并调用 `Precompute()`，析构函数中 Shutdown
- `src/Systems/RenderSystem/RenderSystem.Render.cpp` — `UpdateGlobalUniformBuffer()` 中写入 IBL descriptor（bindings 5-7）

**IBL 预计算参数**:
- 辐照度贴图：32×32 每面，R16G16B16A16Sfloat
- 预滤波环境贴图：128×128 每面，5 级 mip，R16G16B16A16Sfloat
- BRDF LUT：512×512，R16G16Sfloat

---

### 任务 6 实施: 阴影偏移改进

**修改文件**:
- `example/Graphics/Assets/Shader/pbr.frag` — GlobalUniform 新增 `vec4 shadowBiasParams`（x=constant bias, y=slope scale, z=normal offset, w=unused）；`sampleShadowMap()` 改为接收 bias 参数；`calculateShadow()` 实现法线偏移 + 坡度缩放偏移
- `example/Graphics/Assets/Shader/pbr.vert` — GlobalUniform 同步新增 `vec4 shadowBiasParams`
- `example/Graphics/Assets/Shader/shadow.vert` — GlobalUniform 同步新增 `vec4 shadowBiasParams`
- `include/Ailurus/Systems/RenderSystem/RenderSystem.h` — 新增 `_shadowConstantBias`(0.005f)、`_shadowSlopeScale`(0.01f)、`_shadowNormalOffset`(0.02f) 成员及 setter 方法，新增 `GetGlobalUniformAccessNameShadowBiasParams()`
- `src/Systems/RenderSystem/RenderSystem.cpp` — `BuildGlobalUniform()` 新增 `shadowBiasParams` vec4 成员
- `src/Systems/RenderSystem/RenderSystem.Render.cpp` — `UpdateGlobalUniformBuffer()` 上传 shadowBiasData 到 UBO

**设计要点**:
- 三种偏移方式组合使用：常量偏移、坡度缩放偏移（基于光线入射角）、法线偏移（沿法线方向偏移采样位置）
- 所有参数可在运行时通过 RenderSystem 的 setter 方法调整

---

### 任务 7 实施: Gamma 正确纹理采样

**修改文件**:
- `example/Graphics/Assets/Material/PBRMaterial.json` — 纹理配置新增 `"colorSpace"` 字段：albedo 纹理使用 `"srgb"`，法线贴图使用 `"linear"`
- `src/Systems/AssetsSystem/AssetsSystem.Material.cpp` — `JsonReadTextures()` 读取 `colorSpace` 字段，根据值选择 `eR8G8B8A8Srgb`（srgb）或 `eR8G8B8A8Unorm`（linear）格式，调用 `CreateImageFromConfig()` 创建纹理

**设计要点**:
- 通过 JSON 枚举（`"srgb"` / `"linear"`）控制颜色空间，默认 srgb
- albedo 等颜色纹理使用 sRGB 格式（硬件自动 gamma 解码）
- 法线贴图、金属度/粗糙度等数据纹理使用 linear 格式（避免错误的 gamma 解码）

---

### 任务 8 实施: SSAO

**新建文件**:
- `example/Graphics/Assets/Shader/PostProcess/ssao_generate.frag` — SSAO 生成着色器（从深度重建视空间位置，半球采样计算遮蔽，hash 随机化）
- `example/Graphics/Assets/Shader/PostProcess/ssao_blur.frag` — 4×4 box blur 着色器
- `example/Graphics/Assets/Shader/PostProcess/ssao_composite.frag` — 合成着色器（场景颜色 × AO，支持强度控制）
- `include/Ailurus/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.h` — SSAOEffect 类声明
- `src/Systems/RenderSystem/PostProcess/Effects/SSAOEffect.cpp` — SSAOEffect 三 pass 实现

**修改文件**:
- `src/Systems/RenderSystem/RenderSystem.cpp` — 在 PostProcessChain 中 `InsertEffect<SSAOEffect>(0)`（插入到 Bloom 之前）
- `src/Systems/RenderSystem/RenderSystem.Render.cpp` — 每帧调用 `ssaoEffect->SetProjectionMatrix()` 传递相机投影矩阵

**三 pass 渲染流程**:
1. **SSAO 生成**（深度缓冲 → R8Unorm AO 贴图）：从深度重建视空间位置和法线，半球采样 + hash 随机化 + 范围检查
2. **模糊**（AO 贴图 → 模糊 AO 贴图）：4×4 box blur 降噪
3. **合成**（场景颜色 + 模糊 AO → 最终输出）：场景颜色乘以 AO 值，strength 参数控制强度

**可配置参数**（均有默认值，可运行时调整）:
- `radius` = 0.5f — 采样半径
- `bias` = 0.025f — 深度偏移
- `power` = 2.0f — AO 强度指数
- `kernelSize` = 32 — 采样数量
- `strength` = 1.0f — 最终合成强度

**后处理链顺序**: SSAO(0) → Bloom(1) → ToneMapping(2)
