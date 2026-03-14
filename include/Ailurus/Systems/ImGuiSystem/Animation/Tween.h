#pragma once

#include <imgui.h>

namespace Ailurus::Tween
{
	enum class EaseType
	{
		Linear,
		EaseInOut,
		EaseOut,
		EaseIn,
		Spring,
	};

	float Animate(ImGuiID key, float target, float speed = 10.0f,
		EaseType ease = EaseType::EaseInOut);
	ImVec4 AnimateColor(ImGuiID key, const ImVec4& target, float speed = 10.0f,
		EaseType ease = EaseType::EaseInOut);
}