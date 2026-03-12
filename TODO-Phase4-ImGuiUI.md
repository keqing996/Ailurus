# Phase 4: ImGui UI 现代化

> 目标：将 ImGui UI 从 Adobe Spectrum 风格提升到 Material Design 3 级别的视觉体验，
> 对标 Flutter 的现代、美观感。包含主题体系重建、字体升级、高阶组件封装、动画支持和 Docking 布局。

---

## 4.1 启用 ImGui Docking

### 现状

当前 imgui 子模块为 `v1.92.3` main 分支，**不包含 Docking API**。
`ImGuiConfigFlags_DockingEnable`、`ImGui::DockSpace()`、`ImGuiDockNode` 等 API 均不可用。
代码中仅在注释里提到 docking（"in docking branch"），实际功能未编译进来。

### 改造方案

#### 4.1.1 切换 imgui 子模块到 docking 分支

```bash
cd submodule/imgui
git fetch origin docking
git checkout docking
cd ../..
git add submodule/imgui
git commit -m "Switch imgui submodule to docking branch"
```

**验证**：确认 `submodule/imgui/imgui.h` 中存在 `ImGuiConfigFlags_DockingEnable` 定义。

#### 4.1.2 在 ImGuiSystem 初始化中启用 Docking

**文件**: `src/App/Systems/ImGuiSystem/ImGuiSystem.cpp`

在 ImGui context 创建后添加：
```cpp
ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
// 可选：启用多视口（需要额外平台后端支持）
// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
```

#### 4.1.3 提供 DockSpace 便捷 API

**文件**: `include/App/Ailurus/Application/ImGuiSystem/ImGuiSystem.h`

新增方法：
```cpp
class ImGuiSystem
{
public:
    // 创建全屏 DockSpace，应在每帧 ImGui 渲染最开始调用
    void BeginDockSpace(const char* name = "MainDockSpace");
    void EndDockSpace();
};
```

**实现**：
```cpp
void ImGuiSystem::BeginDockSpace(const char* name)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin(name, nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoDocking);
    ImGui::PopStyleVar(3);
    ImGui::DockSpace(ImGui::GetID(name));
}

void ImGuiSystem::EndDockSpace()
{
    ImGui::End();
}
```

**注意事项**：
- Docking 分支可能引入 API 变化，需要检查 Vulkan backend 兼容性
- 如果编译报错，对比 docking 分支的 backend 文件版本与当前自定义 backend 的差异
- 暂时不启用 Multi-Viewport（需要额外的平台窗口创建支持，较复杂）

---

## 4.2 Material Design 3 主题体系

### 现状

当前使用 Adobe Spectrum 主题（`include/App/Ailurus/Application/ImGuiSystem/Theme/Spectrum.h`），
通过 `StaticColor`、`LightColor`、`DarkColor` 结构体定义颜色。

Spectrum 主题风格偏企业工具，要达到 Flutter/Material 3 的现代感，需要重建整套主题。

### 改造方案

#### 4.2.1 定义 Material Design 3 语义色 Token

**新文件**: `include/App/Ailurus/Application/ImGuiSystem/Theme/MaterialTheme.h`

```cpp
namespace Ailurus::MaterialTheme
{
    struct ColorScheme
    {
        // Primary
        ImVec4 primary;           // 主色（按钮、选中态）
        ImVec4 onPrimary;         // 主色上的文字/图标
        ImVec4 primaryContainer;  // 主色容器（卡片背景、选中填充）
        ImVec4 onPrimaryContainer;

        // Secondary
        ImVec4 secondary;
        ImVec4 onSecondary;
        ImVec4 secondaryContainer;
        ImVec4 onSecondaryContainer;

        // Tertiary
        ImVec4 tertiary;
        ImVec4 onTertiary;

        // Error
        ImVec4 error;
        ImVec4 onError;
        ImVec4 errorContainer;

        // Surface（背景层）
        ImVec4 surface;           // 最底层背景
        ImVec4 surfaceVariant;    // 次级背景（输入框、分隔线区域）
        ImVec4 onSurface;         // 背景上的主要文字
        ImVec4 onSurfaceVariant;  // 次要文字
        ImVec4 surfaceContainer;  // 卡片/面板背景
        ImVec4 surfaceContainerHigh;
        ImVec4 surfaceContainerHighest;

        // Outline
        ImVec4 outline;           // 边框（输入框、分隔线）
        ImVec4 outlineVariant;    // 次要边框

        // 阴影/Elevation
        ImVec4 shadow;            // 投影色（半透明黑）
    };

    struct ShapeScheme
    {
        float cornerRadiusSmall  = 4.0f;   // 按钮、Chip
        float cornerRadiusMedium = 8.0f;   // 卡片、对话框
        float cornerRadiusLarge  = 12.0f;  // 大卡片、底部 Sheet
        float cornerRadiusFull   = 9999.0f; // 药丸形（Fab、Toggle）
    };

    // 预置配色方案
    ColorScheme DarkScheme();    // 深色主题
    ColorScheme LightScheme();   // 浅色主题

    // 应用主题到 ImGui
    void ApplyTheme(const ColorScheme& colors, const ShapeScheme& shape = {});
}
```

#### 4.2.2 实现 `ApplyTheme()`

**新文件**: `src/App/Systems/ImGuiSystem/Theme/MaterialTheme.cpp`

将 `ColorScheme` 映射到 `ImGuiStyle` 的各项属性：

```cpp
void MaterialTheme::ApplyTheme(const ColorScheme& c, const ShapeScheme& s)
{
    ImGuiStyle& style = ImGui::GetStyle();

    // 圆角
    style.WindowRounding    = s.cornerRadiusMedium;
    style.ChildRounding     = s.cornerRadiusMedium;
    style.FrameRounding     = s.cornerRadiusSmall;
    style.PopupRounding     = s.cornerRadiusMedium;
    style.ScrollbarRounding = s.cornerRadiusFull;
    style.GrabRounding      = s.cornerRadiusFull;
    style.TabRounding       = s.cornerRadiusSmall;

    // 间距（参考 Material 3 的 spacing: 4dp 基准）
    style.WindowPadding     = ImVec2(16, 16);
    style.FramePadding      = ImVec2(12, 8);
    style.ItemSpacing       = ImVec2(8, 8);
    style.ItemInnerSpacing  = ImVec2(8, 4);
    style.ScrollbarSize     = 10;
    style.GrabMinSize       = 10;

    // 边框
    style.WindowBorderSize  = 0;
    style.ChildBorderSize   = 0;
    style.FrameBorderSize   = 1;
    style.PopupBorderSize   = 0;
    style.TabBorderSize     = 0;

    // 颜色映射
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]           = c.surface;
    colors[ImGuiCol_ChildBg]            = c.surfaceContainer;
    colors[ImGuiCol_PopupBg]            = c.surfaceContainerHigh;
    colors[ImGuiCol_Border]             = c.outline;
    colors[ImGuiCol_FrameBg]            = c.surfaceVariant;
    colors[ImGuiCol_FrameBgHovered]     = c.secondaryContainer;
    colors[ImGuiCol_FrameBgActive]      = c.primaryContainer;
    colors[ImGuiCol_TitleBg]            = c.surfaceContainer;
    colors[ImGuiCol_TitleBgActive]      = c.surfaceContainerHigh;
    colors[ImGuiCol_MenuBarBg]          = c.surfaceContainer;
    colors[ImGuiCol_ScrollbarBg]        = c.surface;
    colors[ImGuiCol_ScrollbarGrab]      = c.onSurfaceVariant;
    colors[ImGuiCol_CheckMark]          = c.primary;
    colors[ImGuiCol_SliderGrab]         = c.primary;
    colors[ImGuiCol_Button]             = c.primary;
    colors[ImGuiCol_ButtonHovered]      = c.primaryContainer;
    colors[ImGuiCol_ButtonActive]       = c.onPrimaryContainer;
    colors[ImGuiCol_Header]             = c.secondaryContainer;
    colors[ImGuiCol_HeaderHovered]      = c.primaryContainer;
    colors[ImGuiCol_HeaderActive]       = c.primary;
    colors[ImGuiCol_Tab]                = c.surfaceContainer;
    colors[ImGuiCol_TabHovered]         = c.primaryContainer;
    colors[ImGuiCol_Text]               = c.onSurface;
    colors[ImGuiCol_TextDisabled]       = c.onSurfaceVariant;
    // ... 完整映射所有 ImGuiCol_ 项
}
```

#### 4.2.3 深色主题推荐色值

参考 Material Design 3 Dark Theme 官方色板（https://m3.material.io/styles/color/）：

```cpp
ColorScheme MaterialTheme::DarkScheme()
{
    return {
        .primary             = ImVec4(0.82f, 0.77f, 1.00f, 1.00f),  // #D0BCFF
        .onPrimary           = ImVec4(0.15f, 0.05f, 0.31f, 1.00f),  // #381E72
        .primaryContainer    = ImVec4(0.29f, 0.17f, 0.49f, 1.00f),  // #4F378B
        .onPrimaryContainer  = ImVec4(0.92f, 0.87f, 1.00f, 1.00f),  // #EADDFF
        .secondary           = ImVec4(0.80f, 0.77f, 0.90f, 1.00f),  // #CCC2DC
        .onSecondary         = ImVec4(0.20f, 0.17f, 0.30f, 1.00f),  // #332D41
        .secondaryContainer  = ImVec4(0.29f, 0.27f, 0.39f, 1.00f),  // #4A4458
        .onSecondaryContainer= ImVec4(0.91f, 0.87f, 0.99f, 1.00f),  // #E8DEF8
        .tertiary            = ImVec4(0.94f, 0.72f, 0.80f, 1.00f),  // #EFB8C8
        .onTertiary          = ImVec4(0.29f, 0.10f, 0.18f, 1.00f),  // #492532
        .error               = ImVec4(0.95f, 0.71f, 0.67f, 1.00f),  // #F2B8B5
        .onError             = ImVec4(0.37f, 0.07f, 0.07f, 1.00f),  // #601410
        .errorContainer      = ImVec4(0.56f, 0.11f, 0.11f, 1.00f),  // #8C1D18
        .surface             = ImVec4(0.08f, 0.07f, 0.09f, 1.00f),  // #141218
        .surfaceVariant      = ImVec4(0.28f, 0.27f, 0.31f, 1.00f),  // #49454F
        .onSurface           = ImVec4(0.90f, 0.88f, 0.93f, 1.00f),  // #E6E0E9
        .onSurfaceVariant    = ImVec4(0.79f, 0.76f, 0.82f, 1.00f),  // #CAC4D0
        .surfaceContainer    = ImVec4(0.12f, 0.11f, 0.14f, 1.00f),  // #211F26
        .surfaceContainerHigh= ImVec4(0.17f, 0.15f, 0.19f, 1.00f),  // #2B2930
        .surfaceContainerHighest = ImVec4(0.22f, 0.20f, 0.24f, 1.00f), // #36343B
        .outline             = ImVec4(0.57f, 0.54f, 0.60f, 1.00f),  // #938F99
        .outlineVariant      = ImVec4(0.28f, 0.27f, 0.31f, 1.00f),  // #49454F
        .shadow              = ImVec4(0.00f, 0.00f, 0.00f, 0.30f),
    };
}
```

#### 4.2.4 替换现有 Spectrum 主题

**文件**: 原来调用 `Spectrum::LoadStyle()` 的位置

改为调用：
```cpp
MaterialTheme::ApplyTheme(MaterialTheme::DarkScheme());
```

**保留 Spectrum 文件**不删除（用户可能想切换回来），但默认改用 MaterialTheme。

---

## 4.3 字体升级

### 现状

当前嵌入 SourceSansPro Regular 字体（约 119KB），默认字号 16px。
无图标字体，无中文支持。

### 改造方案

#### 4.3.1 更换正文字体为 Inter

[Inter](https://rsms.me/inter/) 是专为 UI 设计的无衬线字体，比 SourceSansPro 更现代、更清晰。

**操作**：
1. 下载 Inter Regular 和 Inter SemiBold 的 `.ttf` 文件
2. 使用二进制嵌入工具（如 `xxd -i` 或现有的嵌入方式）转为 C 数组
3. 替换 `include/App/Ailurus/Application/ImGuiSystem/Font/` 中的 SansPro 文件
4. 保留 JetBrains Mono 用于等宽文本

**文件结构**：
```
Font/
├── InterRegular.h       // 新增 (~300KB)
├── InterSemiBold.h      // 新增 (~300KB)
├── JetBrainsMonoRegular.h  // 已有，保留
├── MaterialSymbols.h    // 新增（见 4.3.2）
```

#### 4.3.2 嵌入图标字体 Material Symbols

[Material Symbols](https://fonts.google.com/icons) 是 Google 的图标字体，包含 2500+ 图标。

**操作**：
1. 下载 Material Symbols Outlined 的 `.ttf` 文件
2. 只取常用图标的 Unicode 范围（减小体积），或使用完整版（~300KB）
3. 嵌入为 C 数组

**使用方式**：
```cpp
// 在 ImGuiSystem 初始化时加载图标字体
ImFontConfig config;
config.MergeMode = true;  // 合并到主字体
static const ImWchar iconRanges[] = { 0xE000, 0xF8FF, 0 };  // Private Use Area
io.Fonts->AddFontFromMemoryTTF(materialSymbolsData, materialSymbolsSize,
    16.0f, &config, iconRanges);
```

**在 UI 中使用**：
```cpp
// 定义图标常量
#define ICON_SETTINGS   "\uE8B8"
#define ICON_FOLDER     "\uE2C7"
#define ICON_SEARCH     "\uE8B6"
#define ICON_VISIBILITY "\uE8F4"
#define ICON_DELETE     "\uE872"

// 使用
ImGui::Button(ICON_SETTINGS " Settings");
ImGui::Text(ICON_FOLDER " Assets");
```

**新文件**: `include/App/Ailurus/Application/ImGuiSystem/Font/MaterialIcons.h`
定义所有常用图标的 Unicode 宏。

#### 4.3.3 中文字体支持

嵌入 Noto Sans SC 子集（常用 3000 字），减小体积到约 1-2MB。

**操作**：
1. 使用 `pyftsubset`（fonttools）生成子集 TTF：
   ```bash
   pyftsubset NotoSansSC-Regular.ttf --text-file=common_3000.txt --output-file=NotoSansSC-Subset.ttf
   ```
2. 嵌入为 C 数组
3. 使用 `MergeMode` 合并到主字体：
   ```cpp
   config.MergeMode = true;
   io.Fonts->AddFontFromMemoryTTF(notoSansSCData, notoSansSCSize,
       16.0f, &config, io.Fonts->GetGlyphRangesChineseFull());
   ```

**注意**：如果嵌入体积过大（>2MB），可改用运行时从文件加载（用已有的 `CreateImGuiFont(path)` 方法），字体文件随 assets 分发。

#### 4.3.4 在 ImGuiSystem 中统一管理字体

**文件**: `include/App/Ailurus/Application/ImGuiSystem/ImGuiSystem.h`

新增字体枚举和访问：
```cpp
enum class FontType
{
    Regular,      // Inter Regular - 正文
    SemiBold,     // Inter SemiBold - 标题
    Mono,         // JetBrains Mono - 等宽/代码
    Icon,         // Material Symbols - 图标（合并到 Regular）
};

class ImGuiSystem
{
public:
    ImFont* GetFont(FontType type) const;
    // ...
private:
    ImFont* _fontRegular = nullptr;
    ImFont* _fontSemiBold = nullptr;
    ImFont* _fontMono = nullptr;
};
```

---

## 4.4 高阶组件封装

### 现状

ImGui 的原生 widget 功能完整但视觉朴素（灰色方块风格）。需要封装一层高阶组件来达到现代 UI 效果。

### 改造方案

**新目录**: `include/App/Ailurus/Application/ImGuiSystem/Widgets/`

#### 4.4.1 Card 卡片容器

```cpp
// Widgets/Card.h
namespace Ailurus::Widgets
{
    // 带圆角 + 投影的容器
    // elevation: 0=flat, 1=低投影, 2=中投影, 3=高投影
    bool BeginCard(const char* label, int elevation = 1,
                   const ImVec2& size = ImVec2(0, 0));
    void EndCard();
}
```

**实现原理**：
1. 用 `ImGui::BeginChild()` 创建子区域
2. 用 `ImDrawList::AddRectFilled()` 绘制圆角填充背景（MaterialTheme::surfaceContainer）
3. 用多层半透明 `AddRectFilled()` 从外到内模拟投影（elevation 越高，偏移越大，alpha 越小）
4. 圆角值取 `ShapeScheme::cornerRadiusMedium`

#### 4.4.2 Toggle 开关

```cpp
// Widgets/Toggle.h
namespace Ailurus::Widgets
{
    // Flutter/iOS 风格的 Toggle 开关，替代 ImGui::Checkbox
    // 返回值：是否发生了改变
    bool Toggle(const char* label, bool* value);
}
```

**实现原理**：
1. 绘制药丸形轨道（`AddRectFilled` + `cornerRadiusFull`）
2. 根据 `*value` 计算滑块位置（配合动画插值）
3. 滑块用 `AddCircleFilled` 绘制
4. 颜色：off 状态用 `surfaceVariant/outline`，on 状态用 `primary/onPrimary`

#### 4.4.3 SearchBar 搜索框

```cpp
// Widgets/SearchBar.h
namespace Ailurus::Widgets
{
    // 带图标的搜索框
    bool SearchBar(const char* hint, char* buf, size_t bufSize,
                   const ImVec2& size = ImVec2(-1, 0));
}
```

**实现原理**：
1. 圆角输入框（`FrameRounding` 设大值）
2. 左侧绘制搜索图标（Material Symbol `\uE8B6`）
3. 内部使用 `ImGui::InputTextWithHint()`

#### 4.4.4 Toast 非模态通知

```cpp
// Widgets/Toast.h
namespace Ailurus::Widgets
{
    enum class ToastType { Info, Success, Warning, Error };

    // 推送一条 Toast 消息
    void PushToast(const char* message, ToastType type = ToastType::Info,
                   float durationSeconds = 3.0f);

    // 每帧调用，渲染所有活跃的 Toast
    void RenderToasts();
}
```

**实现原理**：
1. 内部维护 `vector<ToastData>`，每条 Toast 有创建时间和持续时间
2. 从右下角向上堆叠，用 `ImGui::SetNextWindowPos()` 定位
3. 入场/离场动画：从右侧滑入，超时后向右滑出（配合 4.5 动画系统）
4. 颜色区分类型：Info=primary, Success=green, Warning=orange, Error=error

#### 4.4.5 PropertyGrid 属性编辑器

```cpp
// Widgets/PropertyGrid.h
namespace Ailurus::Widgets
{
    bool BeginPropertyGrid(const char* label);
    void EndPropertyGrid();

    // 属性行：左侧标签，右侧控件
    void PropertyLabel(const char* label);
    bool PropertyFloat(const char* label, float* value,
                       float min = 0, float max = 0);
    bool PropertyVec3(const char* label, float value[3]);
    bool PropertyColor(const char* label, float color[4]);
    bool PropertyBool(const char* label, bool* value);
    bool PropertyEnum(const char* label, int* current,
                      const char* const* items, int count);
    bool PropertyText(const char* label, char* buf, size_t bufSize);
}
```

**实现原理**：
1. 用 `ImGui::Columns(2)` 或 `ImGui::Table()` 创建两列布局
2. 左列固定宽度显示标签（右对齐，次要文字色）
3. 右列铺满显示控件
4. 每行交替背景色（surfaceContainer / surface）增强可读性
5. 这是编辑器 Inspector 面板的核心组件

#### 4.4.6 TreeView 树形视图

```cpp
// Widgets/TreeView.h
namespace Ailurus::Widgets
{
    struct TreeNode
    {
        const char* label;
        const char* icon;        // Material Symbol 图标（可选）
        bool isSelected;
        bool isExpanded;
        std::vector<TreeNode*> children;
    };

    // 渲染树形视图，返回点击的节点（null 表示无点击）
    TreeNode* TreeView(const char* id, const std::vector<TreeNode*>& roots);
}
```

**实现原理**：
1. 基于 `ImGui::TreeNodeEx()` 封装
2. 添加图标前缀（如文件夹/实体/组件图标）
3. 选中态高亮使用 `primaryContainer` 颜色
4. 这是编辑器 Hierarchy 面板的核心组件

#### 4.4.7 Breadcrumb 面包屑导航

```cpp
// Widgets/Breadcrumb.h
namespace Ailurus::Widgets
{
    // 返回点击的层级索引（-1 表示无点击）
    int Breadcrumb(const std::vector<std::string>& path);
}
```

**实现原理**：
1. 水平排列，每段之间用 `>` 或 `/` 分隔
2. 最后一段加粗（当前位置），前面各段可点击
3. 点击时返回层级索引，调用方处理导航

---

## 4.5 过渡动画系统

### 现状

ImGui 没有内置动画系统。所有 widget 状态变化都是瞬间切换（比如 hover 变色、展开/收缩）。要实现 Flutter 级的流畅度，需要一个简单的 tween 插值系统。

### 改造方案

**新文件**: `include/App/Ailurus/Application/ImGuiSystem/Animation/Tween.h`

```cpp
namespace Ailurus::Tween
{
    // 缓动函数类型
    enum class EaseType
    {
        Linear,
        EaseInOut,     // Material 3 标准缓动
        EaseOut,       // 退出动画
        EaseIn,        // 进入动画
        Spring,        // 弹性（按钮反馈）
    };

    // 获取一个平滑插值的 float 值
    // key: 唯一标识（用 ImGui ID）
    // target: 目标值
    // speed: 插值速度（值越大越快，建议 8.0-15.0）
    // 返回当前帧的平滑值
    float Animate(ImGuiID key, float target, float speed = 10.0f,
                  EaseType ease = EaseType::EaseInOut);

    // 颜色插值
    ImVec4 AnimateColor(ImGuiID key, const ImVec4& target, float speed = 10.0f);
}
```

**实现原理**：
1. 内部维护 `unordered_map<ImGuiID, AnimState>`
2. 每帧对每个活跃的动画做指数衰减插值：
   ```
   current = lerp(current, target, 1.0 - exp(-speed * deltaTime))
   ```
3. 当 `|current - target| < epsilon` 时移除动画状态
4. `deltaTime` 从 `ImGui::GetIO().DeltaTime` 获取

**使用示例**（在 Toggle 组件中）：
```cpp
float knobX = Tween::Animate(id, *value ? trackRight : trackLeft, 12.0f);
ImVec4 trackColor = Tween::AnimateColor(id + 1,
    *value ? theme.primary : theme.surfaceVariant, 10.0f);
```

**注意事项**：
- 动画状态需要在每帧被访问才会更新，未访问的自动清理
- 使用 ImGuiID 作为 key，与 ImGui 的 ID 系统一致
- 不需要手动管理动画生命周期

---

## 4.6 预设编辑器布局

### 现状

没有预设布局。如果引擎定位为工具 + 游戏引擎，需要一套类似 Unity/Godot 的面板布局作为参考实现。

### 改造方案

**新文件**: `include/App/Ailurus/Application/ImGuiSystem/EditorLayout.h`

```cpp
namespace Ailurus
{
    class EditorLayout
    {
    public:
        // 初始化默认布局（首次启动或重置时调用）
        static void SetupDefaultLayout(ImGuiID dockspaceId);

        // 预设面板绘制函数（由用户在 Loop 中调用）
        static void DrawSceneView();      // 3D 场景视口
        static void DrawHierarchy();      // 实体树形列表
        static void DrawInspector();      // 属性编辑器
        static void DrawConsole();        // 日志输出
        static void DrawAssetBrowser();   // 资源浏览器
    };
}
```

**`SetupDefaultLayout()` 实现**：
```cpp
void EditorLayout::SetupDefaultLayout(ImGuiID dockspaceId)
{
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

    ImGuiID center = dockspaceId;
    ImGuiID left   = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left,  0.20f, nullptr, &center);
    ImGuiID right  = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &center);
    ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down,  0.25f, nullptr, &center);

    // 布局：
    // ┌──────────┬──────────────────┬──────────┐
    // │          │                  │          │
    // │Hierarchy │   Scene View     │Inspector │
    // │          │                  │          │
    // ├──────────┴──────────────────┴──────────┤
    // │         Console / Assets               │
    // └────────────────────────────────────────┘

    ImGui::DockBuilderDockWindow("Hierarchy",    left);
    ImGui::DockBuilderDockWindow("Scene View",   center);
    ImGui::DockBuilderDockWindow("Inspector",    right);
    ImGui::DockBuilderDockWindow("Console",      bottom);
    ImGui::DockBuilderDockWindow("Assets",       bottom);  // 与 Console 同区域 Tab

    ImGui::DockBuilderFinish(dockspaceId);
}
```

**依赖关系**：
- 依赖 4.1（Docking）完成后才能实现
- 依赖 4.4 的 TreeView（Hierarchy 面板）和 PropertyGrid（Inspector 面板）

---

## 实施顺序

```
4.1 启用 Docking  ←  前置条件（仅子模块切换 + 几行代码）
     ↓
4.2 Material 3 主题  ←  独立，改完立刻见效
     ↓
4.3 字体升级  ←  独立，改完立刻见效
     ↓
4.5 Tween 动画系统  ←  4.4 的前置（组件需要动画）
     ↓
4.4 高阶组件  ←  依赖 4.2（主题色）+ 4.3（图标字体）+ 4.5（动画）
     ↓
4.6 编辑器布局  ←  依赖 4.1（Docking）+ 4.4（组件）
```

建议 4.1 → 4.2 → 4.3 可以并行快速完成（不互相依赖），然后 4.5 → 4.4 → 4.6 按序推进。
