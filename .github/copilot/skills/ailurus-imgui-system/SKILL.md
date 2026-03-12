# Ailurus ImGui System

## Scope
ImGui integration with Vulkan dynamic rendering backend, SDL3 frontend, embedded font management, and Adobe Spectrum theme support.

## Key Files
- `include/Ailurus/Systems/ImGuiSystem/ImGuiSystem.h` — ImGui context & font API
- `src/Systems/ImGuiSystem/ImGuiSystem.cpp` — SDL3 frontend, HiDPI, lifecycle
- `src/Systems/ImGuiSystem/ImGuiVulkanBackEnd.h` — Vulkan backend header
- `src/Systems/ImGuiSystem/ImGuiVulkanBackEnd.cpp` — Dynamic rendering backend
- `include/Ailurus/Systems/ImGuiSystem/Theme/Spectrum.h` — Adobe Spectrum colors
- `src/Systems/ImGuiSystem/Theme/Spectrum.cpp` — Theme application
- `include/Ailurus/Systems/ImGuiSystem/Font/` — Embedded TTF arrays

## ImGuiSystem Class

**Members:**
- `unique_ptr<ImGuiVulkanBackEnd> _pVkImpl` — Vulkan backend
- `static constexpr float DEFAULT_FONT_SIZE = 16`

**Initialization:**
1. Creates ImGui context (`ImGui::CreateContext()`)
2. Sets config flags (keyboard/gamepad navigation)
3. Initializes Vulkan backend (descriptor pool, dynamic rendering)
4. Initializes SDL3 frontend (`ImGui_ImplSDL3_InitForVulkan()`)
5. Loads default font (SourceSansProRegular at 16pt)
6. Applies Spectrum light theme

**Per-Frame Lifecycle:**
1. `NewFrame()` — Calls Vulkan/SDL3 NewFrame, applies HiDPI scaling
2. Application draws ImGui widgets
3. `Render(VulkanCommandBuffer*)` — `ImGui::Render()` then Vulkan draw

**Font API:**
- `CreateImGuiFont(void* data, int size, int fontSize, bool ownsData, ImWchar* glyphRanges)` → `ImFont*`
- `CreateImGuiFont(string ttfPath, int fontSize, ImWchar* glyphRanges)` → `ImFont*`
- Font size auto-scaled by window DPI scale factor

**HiDPI Support:**
- `NewFrame()` queries SDL window size vs pixel size
- Computes scale factor (e.g., 2.0 for Retina displays)
- Updates `ImGuiIO::DisplaySize` and `DisplayFramebufferScale`

## ImGuiVulkanBackEnd

**Members:**
- `vk::DescriptorPool _descriptorPool` — For ImGui texture descriptors

**Initialization:**
- Creates descriptor pool (`eCombinedImageSampler`, `IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE`)
- Configures `ImGui_ImplVulkan_InitInfo` with:
  - Vulkan instance, physical device, device, graphics queue
  - **Dynamic rendering** (`UseDynamicRendering = true`, no VkRenderPass)
  - Surface format from swapchain
  - MSAA sample count, image count
  - `PipelineRenderingCreateInfo` with color attachment format

**Swapchain Rebuild Handling:**
- Registers pre/post rebuild callbacks with RenderSystem
- Pre-rebuild: `ImGui_ImplVulkan_Shutdown()`
- Post-rebuild: re-creates `ImGui_ImplVulkan_InitInfo` and `ImGui_ImplVulkan_Init()`

## Embedded Fonts

Four TTF fonts embedded as C++ `std::array<unsigned char, N>`:

| Font | Variable | Size |
|------|----------|------|
| Source Sans Pro Regular | `SourceSansProRegular` | ~119 KB |
| Source Sans Pro Bold | `SourceSansProBold` | ~119 KB |
| JetBrains Mono Regular | `SourceJetBrainsMonoRegular` | ~274 KB |
| JetBrains Mono Bold | `SourceJetBrainsMonoBold` | ~278 KB |

Loaded via `ImGui::GetIO().Fonts->AddFontFromMemoryTTF()`.

## Spectrum Theme (Adobe Design System)

**Namespace:** `Ailurus::Spectrum`

**Color Palettes:**
- `StaticColor` — Base: WHITE, BLACK, GRAY200–900, BLUE/RED/ORANGE/GREEN 400–700
- `LightColor` — 68 colors: GRAY50–900, 12 color families × 4 shades
- `DarkColor` — 68 colors: Same structure, adjusted contrast

Color families: Blue, Red, Orange, Green, Indigo, Celery, Magenta, Yellow, Fuchsia, Seafoam, Chartreuse, Purple.

**API:** `LoadStyle(bool isDark = false)` — Applies full ImGui style (all ImGuiCol_* colors, rounding, borders).

**Constants:** `CHECKBOX_BORDER_SIZE = 2.0f`, `CHECKBOX_ROUNDING = 2.0f`
