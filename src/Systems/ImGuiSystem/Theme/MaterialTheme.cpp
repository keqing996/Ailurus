#include <algorithm>
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"

namespace
{
	ImVec4 Mix(const ImVec4& lhs, const ImVec4& rhs, float t)
	{
		const float clamped = std::clamp(t, 0.0f, 1.0f);
		return ImVec4(
			lhs.x + ((rhs.x - lhs.x) * clamped),
			lhs.y + ((rhs.y - lhs.y) * clamped),
			lhs.z + ((rhs.z - lhs.z) * clamped),
			lhs.w + ((rhs.w - lhs.w) * clamped));
	}

	ImVec4 WithAlpha(const ImVec4& color, float alpha)
	{
		return ImVec4(color.x, color.y, color.z, alpha);
	}

	Ailurus::MaterialTheme::ColorScheme g_activeColors = Ailurus::MaterialTheme::DarkScheme();
	Ailurus::MaterialTheme::ShapeScheme g_activeShape{};
}

namespace Ailurus::MaterialTheme
{
	ColorScheme DarkScheme()
	{
		return {
			.primary = ImVec4(0.82f, 0.77f, 1.00f, 1.00f),
			.onPrimary = ImVec4(0.15f, 0.05f, 0.31f, 1.00f),
			.primaryContainer = ImVec4(0.29f, 0.17f, 0.49f, 1.00f),
			.onPrimaryContainer = ImVec4(0.92f, 0.87f, 1.00f, 1.00f),
			.secondary = ImVec4(0.80f, 0.77f, 0.90f, 1.00f),
			.onSecondary = ImVec4(0.20f, 0.17f, 0.30f, 1.00f),
			.secondaryContainer = ImVec4(0.29f, 0.27f, 0.39f, 1.00f),
			.onSecondaryContainer = ImVec4(0.91f, 0.87f, 0.99f, 1.00f),
			.tertiary = ImVec4(0.94f, 0.72f, 0.80f, 1.00f),
			.onTertiary = ImVec4(0.29f, 0.10f, 0.18f, 1.00f),
			.error = ImVec4(0.95f, 0.71f, 0.67f, 1.00f),
			.onError = ImVec4(0.37f, 0.07f, 0.07f, 1.00f),
			.errorContainer = ImVec4(0.56f, 0.11f, 0.11f, 1.00f),
			.surface = ImVec4(0.08f, 0.07f, 0.09f, 1.00f),
			.surfaceVariant = ImVec4(0.28f, 0.27f, 0.31f, 1.00f),
			.onSurface = ImVec4(0.90f, 0.88f, 0.93f, 1.00f),
			.onSurfaceVariant = ImVec4(0.79f, 0.76f, 0.82f, 1.00f),
			.surfaceContainer = ImVec4(0.12f, 0.11f, 0.14f, 1.00f),
			.surfaceContainerHigh = ImVec4(0.17f, 0.15f, 0.19f, 1.00f),
			.surfaceContainerHighest = ImVec4(0.22f, 0.20f, 0.24f, 1.00f),
			.outline = ImVec4(0.57f, 0.54f, 0.60f, 1.00f),
			.outlineVariant = ImVec4(0.28f, 0.27f, 0.31f, 1.00f),
			.shadow = ImVec4(0.00f, 0.00f, 0.00f, 0.30f),
		};
	}

	ColorScheme LightScheme()
	{
		return {
			.primary = ImVec4(0.40f, 0.31f, 0.64f, 1.00f),
			.onPrimary = ImVec4(1.00f, 1.00f, 1.00f, 1.00f),
			.primaryContainer = ImVec4(0.92f, 0.87f, 1.00f, 1.00f),
			.onPrimaryContainer = ImVec4(0.15f, 0.05f, 0.31f, 1.00f),
			.secondary = ImVec4(0.38f, 0.36f, 0.44f, 1.00f),
			.onSecondary = ImVec4(1.00f, 1.00f, 1.00f, 1.00f),
			.secondaryContainer = ImVec4(0.91f, 0.87f, 0.97f, 1.00f),
			.onSecondaryContainer = ImVec4(0.12f, 0.09f, 0.16f, 1.00f),
			.tertiary = ImVec4(0.49f, 0.22f, 0.35f, 1.00f),
			.onTertiary = ImVec4(1.00f, 1.00f, 1.00f, 1.00f),
			.error = ImVec4(0.73f, 0.18f, 0.15f, 1.00f),
			.onError = ImVec4(1.00f, 1.00f, 1.00f, 1.00f),
			.errorContainer = ImVec4(0.98f, 0.87f, 0.86f, 1.00f),
			.surface = ImVec4(1.00f, 0.98f, 0.99f, 1.00f),
			.surfaceVariant = ImVec4(0.91f, 0.89f, 0.93f, 1.00f),
			.onSurface = ImVec4(0.11f, 0.11f, 0.13f, 1.00f),
			.onSurfaceVariant = ImVec4(0.29f, 0.27f, 0.31f, 1.00f),
			.surfaceContainer = ImVec4(0.96f, 0.94f, 0.98f, 1.00f),
			.surfaceContainerHigh = ImVec4(0.93f, 0.91f, 0.95f, 1.00f),
			.surfaceContainerHighest = ImVec4(0.89f, 0.87f, 0.91f, 1.00f),
			.outline = ImVec4(0.47f, 0.45f, 0.49f, 1.00f),
			.outlineVariant = ImVec4(0.79f, 0.76f, 0.82f, 1.00f),
			.shadow = ImVec4(0.00f, 0.00f, 0.00f, 0.18f),
		};
	}

	void ApplyTheme(const ColorScheme& colors, const ShapeScheme& shape)
	{
		g_activeColors = colors;
		g_activeShape = shape;

		ImGuiStyle& style = ImGui::GetStyle();
		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.55f;
		style.WindowPadding = ImVec2(16.0f, 16.0f);
		style.WindowRounding = shape.cornerRadiusMedium;
		style.WindowBorderSize = 0.0f;
		style.WindowMinSize = ImVec2(180.0f, 90.0f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.ChildRounding = shape.cornerRadiusMedium;
		style.ChildBorderSize = 0.0f;
		style.PopupRounding = shape.cornerRadiusMedium;
		style.PopupBorderSize = 0.0f;
		style.FramePadding = ImVec2(12.0f, 8.0f);
		style.FrameRounding = shape.cornerRadiusSmall;
		style.FrameBorderSize = 1.0f;
		style.ItemSpacing = ImVec2(8.0f, 8.0f);
		style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
		style.CellPadding = ImVec2(10.0f, 8.0f);
		style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
		style.IndentSpacing = 20.0f;
		style.ScrollbarSize = 10.0f;
		style.ScrollbarRounding = shape.cornerRadiusFull;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = shape.cornerRadiusFull;
		style.TabRounding = shape.cornerRadiusSmall;
		style.TabBorderSize = 0.0f;
		style.TabBarBorderSize = 0.0f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
		style.DisplayWindowPadding = ImVec2(12.0f, 12.0f);

		ImVec4* palette = style.Colors;
		palette[ImGuiCol_Text] = colors.onSurface;
		palette[ImGuiCol_TextDisabled] = colors.onSurfaceVariant;
		palette[ImGuiCol_WindowBg] = colors.surface;
		palette[ImGuiCol_ChildBg] = colors.surfaceContainer;
		palette[ImGuiCol_PopupBg] = colors.surfaceContainerHigh;
		palette[ImGuiCol_Border] = colors.outlineVariant;
		palette[ImGuiCol_BorderShadow] = WithAlpha(colors.shadow, 0.0f);
		palette[ImGuiCol_FrameBg] = colors.surfaceContainerHigh;
		palette[ImGuiCol_FrameBgHovered] = colors.surfaceContainerHighest;
		palette[ImGuiCol_FrameBgActive] = colors.primaryContainer;
		palette[ImGuiCol_TitleBg] = colors.surfaceContainer;
		palette[ImGuiCol_TitleBgActive] = colors.surfaceContainerHigh;
		palette[ImGuiCol_TitleBgCollapsed] = colors.surfaceContainer;
		palette[ImGuiCol_MenuBarBg] = colors.surfaceContainer;
		palette[ImGuiCol_ScrollbarBg] = colors.surface;
		palette[ImGuiCol_ScrollbarGrab] = colors.surfaceContainerHighest;
		palette[ImGuiCol_ScrollbarGrabHovered] = colors.secondaryContainer;
		palette[ImGuiCol_ScrollbarGrabActive] = colors.primaryContainer;
		palette[ImGuiCol_CheckMark] = colors.primary;
		palette[ImGuiCol_SliderGrab] = colors.primary;
		palette[ImGuiCol_SliderGrabActive] = colors.tertiary;
		palette[ImGuiCol_Button] = colors.primaryContainer;
		palette[ImGuiCol_ButtonHovered] = Mix(colors.primaryContainer, colors.primary, 0.35f);
		palette[ImGuiCol_ButtonActive] = Mix(colors.primaryContainer, colors.primary, 0.55f);
		palette[ImGuiCol_Header] = colors.secondaryContainer;
		palette[ImGuiCol_HeaderHovered] = Mix(colors.secondaryContainer, colors.primaryContainer, 0.45f);
		palette[ImGuiCol_HeaderActive] = colors.primaryContainer;
		palette[ImGuiCol_Separator] = colors.outlineVariant;
		palette[ImGuiCol_SeparatorHovered] = colors.outline;
		palette[ImGuiCol_SeparatorActive] = colors.primary;
		palette[ImGuiCol_ResizeGrip] = WithAlpha(colors.primary, 0.35f);
		palette[ImGuiCol_ResizeGripHovered] = WithAlpha(colors.primary, 0.6f);
		palette[ImGuiCol_ResizeGripActive] = WithAlpha(colors.primary, 0.8f);
		palette[ImGuiCol_InputTextCursor] = colors.primary;
		palette[ImGuiCol_TabHovered] = colors.surfaceContainerHighest;
		palette[ImGuiCol_Tab] = colors.surfaceContainer;
		palette[ImGuiCol_TabSelected] = colors.primaryContainer;
		palette[ImGuiCol_TabSelectedOverline] = colors.primary;
		palette[ImGuiCol_TabDimmed] = colors.surfaceContainer;
		palette[ImGuiCol_TabDimmedSelected] = colors.secondaryContainer;
		palette[ImGuiCol_TabDimmedSelectedOverline] = colors.secondary;
		palette[ImGuiCol_DockingPreview] = WithAlpha(colors.primary, 0.35f);
		palette[ImGuiCol_DockingEmptyBg] = colors.surface;
		palette[ImGuiCol_PlotLines] = colors.primary;
		palette[ImGuiCol_PlotLinesHovered] = colors.tertiary;
		palette[ImGuiCol_PlotHistogram] = colors.primaryContainer;
		palette[ImGuiCol_PlotHistogramHovered] = colors.primary;
		palette[ImGuiCol_TableHeaderBg] = colors.surfaceContainerHigh;
		palette[ImGuiCol_TableBorderStrong] = colors.outline;
		palette[ImGuiCol_TableBorderLight] = colors.outlineVariant;
		palette[ImGuiCol_TableRowBg] = WithAlpha(colors.surfaceContainer, 0.35f);
		palette[ImGuiCol_TableRowBgAlt] = WithAlpha(colors.surfaceContainerHigh, 0.35f);
		palette[ImGuiCol_TextLink] = colors.primary;
		palette[ImGuiCol_TextSelectedBg] = WithAlpha(colors.primaryContainer, 0.55f);
		palette[ImGuiCol_TreeLines] = colors.outlineVariant;
		palette[ImGuiCol_DragDropTarget] = colors.tertiary;
		palette[ImGuiCol_DragDropTargetBg] = WithAlpha(colors.tertiary, 0.25f);
		palette[ImGuiCol_UnsavedMarker] = colors.error;
		palette[ImGuiCol_NavCursor] = colors.primary;
		palette[ImGuiCol_NavWindowingHighlight] = WithAlpha(colors.primary, 0.75f);
		palette[ImGuiCol_NavWindowingDimBg] = WithAlpha(colors.shadow, 0.45f);
		palette[ImGuiCol_ModalWindowDimBg] = WithAlpha(colors.shadow, 0.35f);
	}

	const ColorScheme& GetColorScheme()
	{
		return g_activeColors;
	}

	const ShapeScheme& GetShapeScheme()
	{
		return g_activeShape;
	}
}