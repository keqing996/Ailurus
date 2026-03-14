#include <algorithm>
#include <cmath>
#include <unordered_map>
#include "Ailurus/Systems/ImGuiSystem/Animation/Tween.h"

namespace
{
	struct FloatState
	{
		float value = 0.0f;
		int lastFrameSeen = 0;
	};

	struct ColorState
	{
		ImVec4 value = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		int lastFrameSeen = 0;
	};

	std::unordered_map<ImGuiID, FloatState> g_floatStates;
	std::unordered_map<ImGuiID, ColorState> g_colorStates;

	float EaseAlpha(float alpha, Ailurus::Tween::EaseType ease)
	{
		const float t = std::clamp(alpha, 0.0f, 1.0f);
		switch (ease)
		{
		case Ailurus::Tween::EaseType::Linear:
			return t;
		case Ailurus::Tween::EaseType::EaseIn:
			return t * t;
		case Ailurus::Tween::EaseType::EaseOut:
			return 1.0f - ((1.0f - t) * (1.0f - t));
		case Ailurus::Tween::EaseType::Spring:
			return std::clamp((t * 1.45f) - (0.12f * t * (1.0f - t)), 0.0f, 1.0f);
		case Ailurus::Tween::EaseType::EaseInOut:
		default:
			if (t < 0.5f)
				return 2.0f * t * t;
			return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
		}
	}

	void CleanupStaleStates(int currentFrame)
	{
		if (currentFrame <= 0 || (currentFrame % 180) != 0)
			return;

		std::erase_if(g_floatStates, [currentFrame](const auto& entry)
		{
			return (currentFrame - entry.second.lastFrameSeen) > 180;
		});

		std::erase_if(g_colorStates, [currentFrame](const auto& entry)
		{
			return (currentFrame - entry.second.lastFrameSeen) > 180;
		});
	}

	float StepFloat(float current, float target, float deltaTime, float speed,
		Ailurus::Tween::EaseType ease)
	{
		if (deltaTime <= 0.0f)
			return target;

		const float rawAlpha = 1.0f - std::exp(-std::max(speed, 0.01f) * deltaTime);
		const float easedAlpha = EaseAlpha(rawAlpha, ease);
		const float next = current + ((target - current) * easedAlpha);
		return std::abs(next - target) < 0.0005f ? target : next;
	}

	ImVec4 StepColor(const ImVec4& current, const ImVec4& target, float deltaTime,
		float speed, Ailurus::Tween::EaseType ease)
	{
		return ImVec4(
			StepFloat(current.x, target.x, deltaTime, speed, ease),
			StepFloat(current.y, target.y, deltaTime, speed, ease),
			StepFloat(current.z, target.z, deltaTime, speed, ease),
			StepFloat(current.w, target.w, deltaTime, speed, ease));
	}
}

namespace Ailurus::Tween
{
	float Animate(ImGuiID key, float target, float speed, EaseType ease)
	{
		ImGuiIO& io = ImGui::GetIO();
		const int currentFrame = ImGui::GetFrameCount();
		CleanupStaleStates(currentFrame);

		auto [it, inserted] = g_floatStates.try_emplace(key, FloatState{ target, currentFrame });
		it->second.lastFrameSeen = currentFrame;
		if (inserted)
			return target;

		it->second.value = StepFloat(it->second.value, target, io.DeltaTime, speed, ease);
		return it->second.value;
	}

	ImVec4 AnimateColor(ImGuiID key, const ImVec4& target, float speed, EaseType ease)
	{
		ImGuiIO& io = ImGui::GetIO();
		const int currentFrame = ImGui::GetFrameCount();
		CleanupStaleStates(currentFrame);

		auto [it, inserted] = g_colorStates.try_emplace(key, ColorState{ target, currentFrame });
		it->second.lastFrameSeen = currentFrame;
		if (inserted)
			return target;

		it->second.value = StepColor(it->second.value, target, io.DeltaTime, speed, ease);
		return it->second.value;
	}
}