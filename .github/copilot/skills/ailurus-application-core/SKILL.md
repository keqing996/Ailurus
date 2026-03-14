# Ailurus Application & Core Framework

## Scope
Application lifecycle, window management, SDL3 integration, subsystem orchestration, and build system for the Ailurus renderer.

## Key Files
- `include/Ailurus/Application.h` — Static Application facade (public API)
- `src/Application.cpp` — Application implementation
- `src/SDLMain.cpp` — SDL3 entry point and Vulkan library pre-loading
- `include/Ailurus/Assert.h` — Assertion macros
- `include/Ailurus/PlatformDefine.h` — Platform/chip detection macros
- `CMakeLists.txt` — Root build system
- `example/Graphics/CMakeLists.txt` — Example app build (macOS bundle support)
- `example/Graphics/Main.cpp` — Example usage of the framework

## Architecture

### Application Class (Static Singleton)
All members and methods are static. Non-copyable. Manages the entire framework lifecycle.

**Window Lifecycle:**
- `Create(width, height, title, Style)` — SDL window + Vulkan init + all subsystems
- `Loop(loopFunction)` — Main loop with frame limiting
- `Destroy()` — Reverse-order cleanup

**Style Configuration:**
```cpp
struct Style {
    bool canResize;           // SDL_WINDOW_RESIZABLE
    bool haveBorder;          // !SDL_WINDOW_BORDERLESS
    bool enableRender3D;      // Optional 3D rendering
    std::string skyboxHDRTexturePath;
};
```

**Subsystem Access (templated):**
```cpp
Application::Get<TimeSystem>()
Application::Get<InputSystem>()
Application::Get<RenderSystem>()
Application::Get<AssetsSystem>()
Application::Get<SceneSystem>()
```

### Subsystem Initialization Order (Critical)
```
TimeSystem → InputSystem → RenderSystem → AssetsSystem → SceneSystem
```
Destruction is reverse order.

### Main Loop Flow
```
1. TimeSystem::Update() — delta time
2. EventLoop() — SDL events → Input + window handling
3. RenderSystem::CheckRebuildSwapChain()
4. SceneSystem::UpdateAllComponents(deltaTime)
5. User loop function (lambda)
6. RenderSystem::RenderScene()
7. Frame rate limiting (SDL_DelayNS)
```

### SDL3 Integration
- Vulkan library loaded via `VulkanFunctionLoader` before SDL init
- SDL_SetHint tells SDL where to find pre-loaded Vulkan library
- Window created hidden, shown after init
- Window flags: VULKAN, HIGH_PIXEL_DENSITY, HIDDEN initially
- Vulkan surface created via `SDL_Vulkan_CreateSurface()`

### Window Callbacks
```cpp
SetCallbackOnWindowCreated(void())
SetCallbackOnWindowTryToClose(bool())  // Return false to veto
SetCallbackOnWindowClosed(void())
SetCallbackOnWindowPreDestroyed(void())
SetCallbackOnWindowPostDestroyed(void())
SetCallbackOnWindowMoved(void(Vector2i))
SetCallbackOnWindowResize(void(Vector2i))
SetCallbackOnWindowFocusChanged(void(bool))
SetCallbackOnWindowCursorEnteredOrLeaved(void(bool))
```

### Build System (CMake)
- C++20 required
- Main target: `ailurus` (static library)
- Public deps: nlohmann_json, assimp, stb_image, spdlog, glm
- Private deps: SDL3-static, vulkan_custom
- PCH: `src/VulkanContext/VulkanPch.h`
- Options: `AILURUS_ENABLE_TEST`, `AILURUS_ENABLE_EXAMPLE`
- macOS: App bundle with Info.plist, Frameworks, Vulkan ICD deployment
- Shader compilation via Python script (glslc)

### Example App Pattern
```cpp
int Main(int argc, char* argv[]) {
    Application::Create(800, 800, "Test", Style{
        .canResize = true,
        .haveBorder = true,
        .enableRender3D = true,
    });
    
    auto model = Application::Get<AssetsSystem>()->LoadModel("path");
    auto material = Application::Get<AssetsSystem>()->LoadMaterial("path");
    
    auto entity = Application::Get<SceneSystem>()->CreateEntity();
    // entity.lock()->AddComponent<CompStaticMeshRender>(model, material);
    // Setup camera, lights...
    
    Application::Loop([&]() { /* per-frame logic */ });
    Application::Destroy();
}
```
