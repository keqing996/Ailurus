#pragma once

#include <cstddef>
#include <imgui.h>

namespace Ailurus::Widgets
{
	bool SearchBar(const char* hint, char* buf, size_t bufSize,
		const ImVec2& size = ImVec2(-1.0f, 0.0f));
}