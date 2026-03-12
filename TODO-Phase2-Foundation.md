# Phase 2: 基础架构强化

> 目标：修复组件系统性能问题，增加场景序列化，修复 Uniform 类型安全问题。
> 完成这些后，引擎才具备「可用于做游戏」的最低门槛。

---

## 2.1 组件查找从 O(n) 改为 O(1)

### 现状

`Entity` 的组件存储在 `std::vector<std::unique_ptr<Component>>` 中（见 `include/App/Ailurus/Application/SceneSystem/Entity/Entity.h` 第 75 行）。

`GetComponent(ComponentType)` 和 `RemoveComponent(ComponentType)` 都需要线性遍历整个 vector。每帧渲染时 `CollectRenderingContext()` 对所有 Entity 调用 `GetComponent<CompStaticMeshRender>()`，O(N*M) 复杂度（N=实体数, M=每个实体的组件数）。

### 改造方案

**文件**: `include/App/Ailurus/Application/SceneSystem/Entity/Entity.h`

将私有成员：
```cpp
std::vector<std::unique_ptr<Component>> _components;
```

改为：
```cpp
std::unordered_map<ComponentType, std::vector<std::unique_ptr<Component>>> _components;
```

用 `ComponentType` 做 key，value 是 vector（支持 `AllowMultipleInstance` 的情况）。

**需要同步修改的方法**：

1. **`AddComponent<T>(Args...)`**（Entity.h 第 82-93 行）
   - 改为：`_components[T::StaticType].push_back(std::move(pComp));`
   - 单实例组件：先检查 `_components[T::StaticType]` 是否非空，非空则先清除

2. **`GetComponent(ComponentType type)`**（Entity.cpp）
   - 改为：`auto it = _components.find(type);` → O(1)
   - 如果需要支持继承查找（如 `GetComponent<CompRender>()` 能找到 `CompStaticMeshRender`），需利用 `ComponentMeta::Is()` 做 fallback 遍历

3. **`RemoveComponent(ComponentType compType)`**（Entity.cpp）
   - 改为：`_components.erase(compType);` → O(1)

4. **`GetComponent<T>()` 模板**（Entity.h 第 107-110 行）
   - 无需改动，仍然代理到 `GetComponent(T::StaticType)`

**测试验证**：
- 编写测试：创建 Entity，AddComponent 多个不同类型，验证 GetComponent 返回正确
- 验证 AllowMultipleInstance 组件可以添加多个
- 验证继承查找：`GetComponent<CompRender>()` 能否找到 `CompStaticMeshRender`
- 性能测试：1000 个 Entity 各 10 个组件，连续 GetComponent 100000 次，对比改造前后耗时

---

## 2.2 Entity 父子层级

### 现状

Entity 没有父子关系，每个 Entity 的 Transform 都是世界坐标。如果要做一辆车带4个轮子，需要手动计算每个轮子的世界坐标。

### 改造方案

**文件**: `include/App/Ailurus/Application/SceneSystem/Entity/Entity.h`

新增私有成员：
```cpp
Entity* _parent = nullptr;
std::vector<Entity*> _children;
```

新增公共接口：
```cpp
void SetParent(Entity* parent);
Entity* GetParent() const;
const std::vector<Entity*>& GetChildren() const;
void AddChild(Entity* child);
void RemoveChild(Entity* child);
```

**修改 `GetModelMatrix()`**（Entity.cpp）：
```cpp
Matrix4x4f Entity::GetModelMatrix() const
{
    Matrix4x4f localMatrix = /* 当前的 TRS 计算 */;
    if (_parent != nullptr)
        return _parent->GetModelMatrix() * localMatrix;
    return localMatrix;
}
```

**注意事项**：
- `SetParent()` 需要同时更新旧 parent 的 `_children` 和新 parent 的 `_children`
- `DestroyEntity()` 时需要递归处理子 Entity 或断开父子关系
- 避免循环引用：`SetParent()` 中检查新 parent 不是自身的子孙节点
- 考虑缓存世界矩阵 + dirty flag 避免每帧重复计算深层级的矩阵链

**文件**: `include/App/Ailurus/Application/SceneSystem/SceneSystem.h`

`DestroyEntity()` 需要增加逻辑：销毁 Entity 时从 parent 的 children 列表中移除自身。

---

## 2.3 组件生命周期回调

### 现状

Component 基类只有虚析构函数，没有任何生命周期钩子。组件被 `AddComponent` 添加或 `RemoveComponent` 移除时没有通知机制。

### 改造方案

**文件**: `include/App/Ailurus/Application/SceneSystem/Component/Base/Component.h`

在 `Component` 类中新增虚函数：
```cpp
class Component
{
public:
    virtual ~Component() = default;
    virtual ComponentType GetType() const { return ComponentType::Component; }

    // 新增生命周期回调
    virtual void OnAttach() {}    // 组件被添加到 Entity 后调用
    virtual void OnDetach() {}    // 组件从 Entity 移除前调用
    virtual void OnUpdate(float deltaTime) {}  // 每帧更新（可选）

    Entity* GetEntity() const { return _parentEntity; }
    // ...
};
```

**文件**: `include/App/Ailurus/Application/SceneSystem/Entity/Entity.h`

修改 `AddComponent<T>()` 模板（第 82-93 行），在 push_back 后调用：
```cpp
pComp->_parentEntity = this;
_components.push_back(std::move(pComp));
_components.back()->OnAttach();  // 新增
```

修改 `RemoveComponent()`（Entity.cpp），在移除前调用：
```cpp
comp->OnDetach();  // 新增
// 然后从容器中移除
```

**文件**: `src/App/Systems/RenderSystem/RenderSystem.Render.cpp` 或 Application 主循环

在主循环适当位置调用所有 Entity 的所有 Component 的 `OnUpdate(deltaTime)`。可以考虑在 `Application::Loop` 的 loopFunction 调用之前执行。

**注意事项**：
- `OnAttach()` 调用时 `GetEntity()` 已经有效
- `OnDetach()` 调用时 `GetEntity()` 仍然有效（在移除之前调用）
- `OnUpdate()` 不是每个组件都需要，默认空实现即可

---

## 2.4 UniformValue 类型安全

### 现状

`UniformValue`（`include/App/Ailurus/Application/RenderSystem/Uniform/UniformValue.h`）使用裸 `union Data` 存储，`GetData()` 返回 `const Data&`，调用方可以读取任意成员而不做类型检查，类型不匹配时读到垃圾数据。

### 改造方案

**方案 A（推荐）：保留 union，增加类型安全的 getter**

保留现有 `GetData()` 给内部使用（如 memcpy 写 uniform buffer），新增类型安全的访问方法：

```cpp
class UniformValue
{
public:
    // 类型安全 getter — 类型不匹配时 assert 失败
    int32_t GetInt() const;
    float GetFloat() const;
    const Vector2f& GetVector2() const;
    const Vector3f& GetVector3() const;
    const Vector4f& GetVector4() const;
    const Matrix4x4f& GetMatrix4x4() const;

    // 已有的保留
    auto GetDataPointer() const -> const void*;
    auto GetType() const -> UniformValueType;
    auto GetSize() const -> uint32_t;
    auto GetData() const -> const Data&;
    // ...
};
```

**实现**（`src/App/Systems/RenderSystem/Uniform/UniformValue.cpp`）：

```cpp
int32_t UniformValue::GetInt() const
{
    ASSERT_MSG(type == UniformValueType::Int, "UniformValue type mismatch");
    return data.intValue;
}
// 其余类似
```

**方案 B（更彻底）：改用 `std::variant`**

```cpp
using UniformData = std::variant<int32_t, float, Vector2f, Vector3f, Vector4f, Matrix4x4f>;
```

好处是编译器强制类型安全；坏处是 `GetDataPointer()` 需要用 `std::visit` 实现，可能影响性能和 memcpy 逻辑。

**建议选方案 A**，改动最小，向后兼容。

---

## 2.5 UniformVariable 拼写修正

### 现状

`include/App/Ailurus/Application/RenderSystem/Uniform/UniformVariable.h` 中：
- 枚举名 `UniformVaribleType`（缺少字母 'a'）
- 方法名 `VaribleType()`（同样拼写错误）

### 改造方案

全局搜索替换（涉及多个文件）：

1. `UniformVaribleType` → `UniformVariableType`
2. `VaribleType()` → `VariableType()`

**影响的文件**（需全局 grep `UniformVaribleType` 和 `VaribleType` 确认）：
- `include/App/Ailurus/Application/RenderSystem/Uniform/UniformVariable.h` — 枚举定义 + 3 个虚方法声明
- `src/App/Systems/RenderSystem/Uniform/UniformVariable.cpp` — 3 个 override 实现
- `src/App/Systems/RenderSystem/Uniform/UniformLayoutHelper.cpp` — 布局计算中的类型判断
- `src/App/Systems/AssetsSystem/AssetsSystem.Material.cpp` — Material JSON 解析
- `test/Graphics/TestUniformStd140.cpp` — 测试代码
- 其他引用文件（需 grep 确认完整列表）

**注意**：这是纯机械替换，不改变任何逻辑，但需确保所有引用都被更新。

---

## 2.6 场景 JSON 序列化

### 现状

项目已有 `nlohmann/json` 依赖（Material 加载已使用），但 Scene/Entity 没有任何序列化能力。无法保存/加载关卡。

### 改造方案

**新文件**: `src/App/Systems/SceneSystem/SceneSerializer.h` 和 `.cpp`

```cpp
class SceneSerializer
{
public:
    static nlohmann::json SerializeScene(const SceneSystem& scene);
    static void DeserializeScene(SceneSystem& scene, AssetsSystem& assets,
                                 const nlohmann::json& json);

    static nlohmann::json SerializeEntity(const Entity& entity);
    static std::shared_ptr<Entity> DeserializeEntity(const nlohmann::json& json,
                                                      AssetsSystem& assets);
};
```

**JSON 格式设计**：
```json
{
  "version": 1,
  "entities": [
    {
      "guid": 1,
      "name": "MainCamera",
      "parent": null,
      "transform": {
        "position": [0.0, 5.0, -10.0],
        "rotation": [0.0, 0.0, 0.0, 1.0],
        "scale": [1.0, 1.0, 1.0]
      },
      "components": [
        {
          "type": "Camera",
          "isPerspective": true,
          "fovHorizontal": 90.0,
          "aspect": 1.777,
          "near": 0.1,
          "far": 1000.0
        },
        {
          "type": "Light",
          "lightType": "Directional",
          "color": [1.0, 1.0, 1.0],
          "intensity": 1.0,
          "direction": [0.0, -1.0, 0.0]
        },
        {
          "type": "StaticMeshRender",
          "modelPath": "Models/Cube.fbx",
          "materialPath": "Materials/PBRMaterial.json"
        }
      ]
    }
  ]
}
```

**实现步骤**：

1. 每个 Component 子类实现 `virtual nlohmann::json Serialize() const` 和 `static T* Deserialize(const nlohmann::json&)`
   - `CompCamera`：序列化 isPerspective, fov, aspect, near, far
   - `CompLight`：序列化 lightType, color, intensity, direction, attenuation, cutoff
   - `CompStaticMeshRender`：序列化 model 和 material 的文件路径

2. `Entity` 需要新增 `std::string _name` 字段（可选但方便调试和序列化）

3. `SceneSystem` 新增便捷方法：
   ```cpp
   void SaveToFile(const std::string& filePath) const;
   void LoadFromFile(const std::string& filePath, AssetsSystem& assets);
   ```

**依赖关系**：
- 此任务依赖 2.2（父子层级，需要序列化 parent 引用）
- 依赖 2.3（组件生命周期，反序列化时需要触发 OnAttach）
- 建议最后实现

**测试验证**：
- 创建若干 Entity（有 Camera、Light、StaticMeshRender 组件）
- SerializeScene → JSON 字符串
- 清空 Scene → DeserializeScene 重建
- 验证所有 Entity 的 Transform 和 Component 数据与原始一致
