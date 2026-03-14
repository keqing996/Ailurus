#pragma once

#include <imgui.h>

namespace Ailurus::MaterialTheme
{
	struct ColorScheme
	{
		ImVec4 primary;
		ImVec4 onPrimary;
		ImVec4 primaryContainer;
		ImVec4 onPrimaryContainer;
		ImVec4 secondary;
		ImVec4 onSecondary;
		ImVec4 secondaryContainer;
		ImVec4 onSecondaryContainer;
		ImVec4 tertiary;
		ImVec4 onTertiary;
		ImVec4 error;
		ImVec4 onError;
		ImVec4 errorContainer;
		ImVec4 surface;
		ImVec4 surfaceVariant;
		ImVec4 onSurface;
		ImVec4 onSurfaceVariant;
		ImVec4 surfaceContainer;
		ImVec4 surfaceContainerHigh;
		ImVec4 surfaceContainerHighest;
		ImVec4 outline;
		ImVec4 outlineVariant;
		ImVec4 shadow;
	};

	struct ShapeScheme
	{
		float cornerRadiusSmall = 4.0f;
		float cornerRadiusMedium = 10.0f;
		float cornerRadiusLarge = 14.0f;
		float cornerRadiusFull = 9999.0f;
	};

	ColorScheme DarkScheme();
	ColorScheme LightScheme();
	void ApplyTheme(const ColorScheme& colors, const ShapeScheme& shape = {});
	const ColorScheme& GetColorScheme();
	const ShapeScheme& GetShapeScheme();
}