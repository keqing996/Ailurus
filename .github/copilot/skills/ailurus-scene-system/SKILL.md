# Ailurus Scene System

## Scope
Entity-component architecture, scene hierarchy, transform management, component lifecycle, concrete component types, and JSON serialization.

## Key Files
- `include/Ailurus/Systems/SceneSystem/SceneSystem.h` — Scene entity management
- `include/Ailurus/Systems/SceneSystem/Entity.h` — Game entity with transform & hierarchy
- `include/Ailurus/Systems/SceneSystem/Component/Component.h` — Base component + TComponent template
- `include/Ailurus/Systems/SceneSystem/Component/ComponentType.h` — Component type enum
- `include/Ailurus/Systems/SceneSystem/Component/CompCamera.h` — Camera component
- `include/Ailurus/Systems/SceneSystem/Component/CompLight.h` — Light component
- `include/Ailurus/Systems/SceneSystem/Component/CompRender.h` — Render base component
- `include/Ailurus/Systems/SceneSystem/Component/CompStaticMeshRender.h` — Static mesh render component
- `include/Ailurus/Systems/SceneSystem/Serializer/SceneSerializer.h` — JSON scene serializer
- `src/Systems/SceneSystem/` — All implementations

## Architecture

### SceneSystem
Central entity registry, subsystem of Application.

**Members:**
- `unordered_map<uint32_t, shared_ptr<Entity>> _entities` — Entity storage by GUID
- `uint32_t _guidCounter` — Auto-incrementing entity ID

**API:**
- `CreateEntity(name)` → `weak_ptr<Entity>` — Allocates entity with unique GUID
- `DestroyEntity(guid)` — Recursive destruction including all children
- `GetEntity(guid)` → `weak_ptr<Entity>` — Returns weak reference
- `GetAllEntities()` → `const&` whole map

Returns `weak_ptr<Entity>` externally to prevent reference cycles.

### Entity
Game object with transform, hierarchy, and component storage.

**Transform:**
- `Vector3f position` (default 0,0,0)
- `Quaternionf rotation` (default identity)
- `Vector3f scale` (default 1,1,1)
- `GetModelMatrix()` → `Matrix4x4f` (TRS composition)

**Hierarchy:**
- `Entity* parent` — Raw pointer (weak back-reference)
- `vector<Entity*> children` — Raw pointers (non-owning)
- `SetParent(Entity*)` — With circular-reference detection
- `AddChild(Entity*)`, `RemoveChild(Entity*)`

**Component Storage:**
- `unordered_map<ComponentType, vector<unique_ptr<Component>>> _components`
- `AddComponent<T>(args...)` → `T*` — Creates and attaches component, calls `OnAttach()`
- `GetComponent<T>()` → `T*` — Type-safe retrieval with inheritance lookup
- `GetComponents<T>()` → `vector<T*>` — All components of type (or derived)
- `RemoveComponent<T>()` — Detach and destroy, calls `OnDetach()`

### Component System

**Component (Base):**
- Virtual lifecycle: `OnAttach(Entity&)`, `OnDetach()`, `OnUpdate(float dt)`
- Virtual: `GetComponentType()`, `Serialize()`, `Deserialize()`
- `Entity* _entity` — Back-reference to owning entity

**TComponent<ComponentType Type, typename BaseType, bool AllowMultiple>:**
CRTP registration template.
- `static ComponentType StaticComponentType()` → compile-time type
- `GetComponentType() override` → returns Type
- Inner `Registrar` struct triggers `ComponentMeta::Register()` at static init
- `AllowMultiple` flag controls whether entity can hold multiple instances

**ComponentMeta (Registry):**
- `Register(type, parentType)` — Records type hierarchy
- `IsA(type, baseType)` → inheritance check at runtime
- Used by `GetComponent<Base>()` to find derived types

### ComponentType Enum
```
REFLECTION_ENUM(ComponentType, Camera, Light, Render, StaticMeshRender)
```

### Concrete Components

**CompCamera** — `TComponent<Camera, Component, false>`
- Projection: `enum {Perspective, Orthographic}`
- Members: `fov`, `nearPlane`, `farPlane`, `aspect`, `orthoSize`
- `GetViewMatrix()` — From entity transform
- `GetProjectionMatrix()` — Perspective or orthographic with Vulkan NDC (Y-flip, 0–1 depth)
- `GetViewProjectionMatrix()` — Combined VP

**CompLight** — `TComponent<Light, Component, false>`
- Type: `enum {Directional, Point, Spot}`
- Members: `color (Vector3f)`, `intensity`, `range`, `innerConeAngle`, `outerConeAngle`
- Direction derived from entity rotation
- Attenuation factor based on range (for Point/Spot)

**CompRender** — `TComponent<Render, Component, false>`
- Minimal base class for renderable components

**CompStaticMeshRender** — `TComponent<StaticMeshRender, CompRender, false>`
- Members: `AssetRef<Model> model`, `AssetRef<MaterialInstance> materialInstance`
- `GetWorldAABB()` — Model AABB transformed by entity's model matrix (merge of all mesh AABBs)
- `SetModel(AssetRef<Model>)`, `SetMaterialInstance(AssetRef<MaterialInstance>)`

### Scene Serialization

**SceneSerializer:**
- `Serialize(SceneSystem&)` → `nlohmann::json`
- `Deserialize(json, SceneSystem&)` — Two-pass loading

**JSON Schema:**
```json
{
  "entities": [{
    "guid": 1,
    "name": "MyEntity",
    "parentGuid": 0,
    "position": [0, 0, 0],
    "rotation": [0, 0, 0, 1],
    "scale": [1, 1, 1],
    "components": [{
      "type": "StaticMeshRender",
      "model": "path/to/model.obj",
      "material": "path/to/material.json"
    }]
  }]
}
```

**Two-Pass Deserialization:**
1. Pass 1: Create all entities with transforms and components (parentGuid stored)
2. Pass 2: Resolve parent-child relationships using stored GUIDs

## Key Patterns
- **Weak ownership**: SceneSystem owns via `shared_ptr`, exposes `weak_ptr`
- **Auto-registration**: TComponent inner `Registrar` struct registers type hierarchy at static init
- **Inheritance-aware lookup**: `GetComponent<CompRender>()` finds `CompStaticMeshRender` via ComponentMeta
- **Cyclic reference prevention**: `SetParent()` walks ancestor chain to detect loops
