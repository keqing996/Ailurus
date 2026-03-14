#pragma once

#include <imgui.h>

namespace Ailurus::Widgets
{
	bool BeginCard(const char* label, int elevation = 1,
		const ImVec2& size = ImVec2(0.0f, 0.0f));
	void EndCard();
}