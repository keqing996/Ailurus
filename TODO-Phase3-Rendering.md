# Phase 3: 渲染系统强化

> 目标：补充渲染管线中缺失的关键能力——视锥裁剪、AABB、渲染统计、自定义 RenderPass。
> 这些是引擎从 demo 级别迈向可发布游戏的必要条件。

---

## 3.1 为 Mesh / Model 计算 AABB（前置任务）

### 现状

`CompStaticMeshRender`（`include/App/Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h`）只持有 `AssetRef<Model>` 和 `AssetRef<MaterialInstance>`，没有任何包围盒数据。Mesh/Model 资源在加载时也不计算边界信息。

没有 AABB，视锥裁剪无从实施。**此任务是 3.2 的前置条件**。

### 改造方案

#### 3.1.1 定义 AABB 结构体

**新文件**: `include/Core/Ailurus/Math/AABB.hpp`

```cpp
template <typename T>
struct AABB
{
    Vector3<T> min;
    Vector3<T> max;

    Vector3<T> GetCenter() const { return (min + max) * T(0.5); }
    Vector3<T> GetExtents() const { return (max - min) * T(0.5); }

    // 用 Model Matrix 变换 AABB（结果仍是 AABB，会比原始包围盒大）
    AABB Transform(const Matrix4x4<T>& matrix) const;

    // 合并两个 AABB
    static AABB Merge(const AABB& a, const AABB& b);
};

using AABBf = AABB<float>;
```

#### 3.1.2 Mesh 加载时计算 AABB

**文件**: `include/App/Ailurus/Application/AssetsSystem/Mesh/Mesh.h`

在 `Mesh` 类中新增成员：
```cpp
class Mesh
{
public:
    // 现有接口...
    const AABBf& GetLocalAABB() const;

private:
    AABBf _localAABB;  // 新增：模型空间的 AABB
};
```

**文件**: Mesh 的构造/创建代码（在 assimp 加载路径中）

在加载顶点数据时，遍历所有顶点计算 min/max：
```cpp
Vector3f minPos(FLT_MAX, FLT_MAX, FLT_MAX);
Vector3f maxPos(-FLT_MAX, -FLT_MAX, -FLT_MAX);
for (const auto& vertex : vertices)
{
    minPos = Vector3f::Min(minPos, vertex.position);
    maxPos = Vector3f::Max(maxPos, vertex.position);
}
_localAABB = { minPos, maxPos };
```

#### 3.1.3 Model 合并所有 Mesh 的 AABB

**文件**: `include/App/Ailurus/Application/AssetsSystem/Model/Model.h`

```cpp
class Model
{
public:
    const AABBf& GetLocalAABB() const;

private:
    AABBf _localAABB;  // 所有 Mesh 的 AABB 合并结果
};
```

在 Model 加载完所有 Mesh 后，调用 `AABB::Merge` 合并。

#### 3.1.4 CompStaticMeshRender 提供世界空间 AABB

**文件**: `include/App/Ailurus/Application/SceneSystem/Component/CompStaticMeshRender.h`

新增方法：
```cpp
AABBf GetWorldAABB() const
{
    return _modelAsset->GetLocalAABB().Transform(GetEntity()->GetModelMatrix());
}
```

**测试验证**：
- 加载一个已知尺寸的模型（如 unit cube），验证 AABB 为 (-0.5, -0.5, -0.5) ~ (0.5, 0.5, 0.5)
- 设置 Entity scale 为 (2,2,2)，验证世界 AABB 为 (-1,-1,-1) ~ (1,1,1)
- 设置 Entity position 为 (5,0,0)，验证世界 AABB 偏移正确

---

## 3.2 视锥裁剪

### 现状

`RenderSystem::CollectRenderingContext()`（`src/App/Systems/RenderSystem/RenderSystem.Render.cpp`）遍历所有 Entity 并收集所有 `CompStaticMeshRender`，**完全没有裁剪**：

```cpp
const auto allEntities = Application::Get<SceneSystem>()->GetAllRawEntities();
for (const auto pEntity : allEntities)
{
    const auto pMeshRender = pEntity->GetComponent<CompStaticMeshRender>();
    // 直接收集，无论是否在相机视野内
}
```

项目中已有 frustum 计算代码（CSM 阴影中算了 frustum corners），但没有用于前向渲染裁剪。

### 改造方案

#### 3.2.1 定义 Frustum 结构体

**新文件**: `include/Core/Ailurus/Math/Frustum.hpp`

```cpp
struct FrustumPlane
{
    Vector3f normal;
    float distance;

    // 点到平面的有符号距离
    float SignedDistance(const Vector3f& point) const;
};

struct Frustum
{
    FrustumPlane planes[6];  // Near, Far, Left, Right, Top, Bottom

    // 从 View-Projection 矩阵提取 6 个平面（Gribb-Hartmann 方法）
    static Frustum FromViewProjection(const Matrix4x4f& vp);

    // AABB 是否与 Frustum 相交（或完全在内部）
    bool Intersects(const AABBf& aabb) const;
};
```

**`Frustum::FromViewProjection()` 算法**：
从 VP 矩阵的行向量直接提取 6 个裁剪平面（Gribb-Hartmann 方法），无需手动构造 frustum corners。这是最高效的方法。

**`Frustum::Intersects(AABB)` 算法**：
对每个平面，计算 AABB 的「最正」顶点（p-vertex），如果该顶点在平面外侧则整个 AABB 被裁剪。

#### 3.2.2 在 RenderPrepare 中构建 Frustum

**文件**: `src/App/Systems/RenderSystem/RenderSystem.Render.cpp`

在 `RenderPrepare()` 中，计算完 `viewProjectionMatrix` 后：
```cpp
void RenderSystem::RenderPrepare()
{
    const Matrix4x4f projMat = _pMainCamera->GetProjectionMatrix();
    const Matrix4x4f viewMat = _pMainCamera->GetViewMatrix();
    _pIntermediateVariable->viewProjectionMatrix = projMat * viewMat;
    _pIntermediateVariable->cameraFrustum = Frustum::FromViewProjection(
        _pIntermediateVariable->viewProjectionMatrix);  // 新增
    // ...
}
```

需要在 `RenderIntermediateVariable` 结构体（`src/App/Systems/RenderSystem/Detail/RenderIntermediateVariable.h`）中新增：
```cpp
Frustum cameraFrustum;
```

#### 3.2.3 在 CollectRenderingContext 中应用裁剪

**文件**: `src/App/Systems/RenderSystem/RenderSystem.Render.cpp`

修改 `CollectRenderingContext()`：
```cpp
for (const auto pEntity : allEntities)
{
    const auto pMeshRender = pEntity->GetComponent<CompStaticMeshRender>();
    if (pMeshRender == nullptr) continue;

    // 新增：视锥裁剪
    AABBf worldAABB = pMeshRender->GetWorldAABB();
    if (!_pIntermediateVariable->cameraFrustum.Intersects(worldAABB))
        continue;  // 跳过不可见物体

    // 后续收集逻辑不变...
}
```

**测试验证**：
- 放置物体在相机正前方，验证被收集
- 放置物体在相机正后方，验证被裁剪
- 放置物体在相机视野边缘，验证正确判断
- 对比裁剪开关前后的 draw call 数量（配合 3.3 渲染统计）

---

## 3.3 渲染统计

### 现状

没有任何渲染统计 API。无法知道每帧绘制了多少 draw call、多少三角形、裁剪了多少物体。对性能优化和调试工具（ImGui 面板）都是刚需。

### 改造方案

#### 3.3.1 定义 RenderStats 结构体

**新文件**: `include/App/Ailurus/Application/RenderSystem/RenderStats.h`

```cpp
struct RenderStats
{
    // 每帧重置
    uint32_t drawCalls = 0;            // draw call 总数
    uint32_t triangleCount = 0;        // 三角形总数
    uint32_t entityCount = 0;          // 提交渲染的 Entity 数
    uint32_t culledEntityCount = 0;    // 被视锥裁剪掉的 Entity 数
    uint32_t meshCount = 0;            // 提交渲染的 Mesh 数

    // 帧时间
    float frameTimeMs = 0.0f;          // CPU 侧帧时间（毫秒）
    float renderTimeMs = 0.0f;         // GPU 渲染时间（毫秒），需要 GPU timestamp query

    void Reset()
    {
        drawCalls = 0;
        triangleCount = 0;
        entityCount = 0;
        culledEntityCount = 0;
        meshCount = 0;
    }
};
```

#### 3.3.2 在 RenderSystem 中维护统计

**文件**: `include/App/Ailurus/Application/RenderSystem/RenderSystem.h`

新增：
```cpp
class RenderSystem
{
public:
    const RenderStats& GetRenderStats() const;

private:
    RenderStats _renderStats;
};
```

#### 3.3.3 在渲染流程中累加统计

**文件**: `src/App/Systems/RenderSystem/RenderSystem.Render.cpp`

1. `RenderPrepare()` 开头：`_renderStats.Reset();`

2. `CollectRenderingContext()` 中：
   - 每当一个 Entity 被裁剪：`_renderStats.culledEntityCount++;`
   - 每当一个 Entity 被收集：`_renderStats.entityCount++;`
   - 每收集一个 Mesh：`_renderStats.meshCount++;`

3. Forward pass 绘制时：
   - 每次 `vkCmdDraw` / `vkCmdDrawIndexed`：`_renderStats.drawCalls++;`
   - 累加三角形数：`_renderStats.triangleCount += indexCount / 3;`

**使用场景**：
- ImGui 调试面板显示实时统计
- 帧率分析和性能瓶颈定位
- 视锥裁剪效果验证

---

## 3.4 自定义 RenderPass 插槽（可选/低优先级）

### 现状

`RenderPassType` 枚举（`include/App/Ailurus/Application/RenderSystem/RenderPass/RenderPassType.h`）是硬编码的：
```cpp
REFLECTION_ENUM(RenderPassType, Shadow, Forward, PostProcess, ImGui);
```

用户无法插入自定义的 render pass（如 wireframe overlay、outline pass、depth prepass 等）。

### 改造方案

#### 方案：用户注册回调的方式

不改变 RenderPassType 枚举。在 Forward pass 结束后、PostProcess 开始前，提供一个回调插槽：

**文件**: `include/App/Ailurus/Application/RenderSystem/RenderSystem.h`

新增：
```cpp
using CustomRenderCallback = std::function<void(VulkanCommandBuffer*)>;

class RenderSystem
{
public:
    // 注册自定义渲染回调，在 Forward pass 之后、PostProcess 之前执行
    void AddCustomRenderPass(const std::string& name, const CustomRenderCallback& callback);
    void RemoveCustomRenderPass(const std::string& name);

private:
    std::vector<std::pair<std::string, CustomRenderCallback>> _customRenderPasses;
};
```

**文件**: `src/App/Systems/RenderSystem/RenderSystem.cpp`（主渲染循环中）

在 Forward pass 完成后、PostProcess 执行前，插入：
```cpp
// 执行用户自定义 render pass
for (auto& [name, callback] : _customRenderPasses)
    callback(pCommandBuffer);
```

**使用示例**：
```cpp
renderSystem->AddCustomRenderPass("Wireframe", [](VulkanCommandBuffer* cmd) {
    // 绘制线框覆盖层
});
```

**注意事项**：
- 回调拿到的是 raw command buffer，用户需要自行管理 pipeline state
- 后续可扩展为更完善的 RenderGraph 系统，但目前回调方案足够
- 此任务优先级低于 3.1-3.3，可延后实现

---

## 实施顺序

```
3.1 AABB 计算  ←  前置条件
     ↓
3.2 视锥裁剪  ←  依赖 3.1
     ↓
3.3 渲染统计  ←  独立，但可与 3.2 配合验证裁剪效果
     ↓
3.4 自定义 RenderPass  ←  独立，低优先级
```

建议先完成 3.1 → 3.2 → 3.3，这三者紧密关联。3.4 可以后续按需实现。
