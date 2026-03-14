#pragma once

#include <imgui.h>

namespace Ailurus
{
	class EditorLayout
	{
	public:
		static void SetupDefaultLayout(ImGuiID dockspaceId);
		static void DrawSceneView();
		static void DrawHierarchy();
		static void DrawInspector();
		static void DrawConsole();
		static void DrawAssetBrowser();
	};
}