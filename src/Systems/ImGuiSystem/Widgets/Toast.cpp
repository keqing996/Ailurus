#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include "Ailurus/Systems/ImGuiSystem/Animation/Tween.h"
#include "Ailurus/Systems/ImGuiSystem/Font/MaterialIcons.h"
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Toast.h"

namespace
{
	struct ToastData
	{
		std::uint64_t id = 0;
		std::string message;
		Ailurus::Widgets::ToastType type = Ailurus::Widgets::ToastType::Info;
		float duration = 3.0f;
		double createdAt = 0.0;
	};

	std::vector<ToastData> g_toasts;
	std::uint64_t g_nextToastId = 1;

	ImVec4 Mix(const ImVec4& lhs, const ImVec4& rhs, float t)
	{
		const float clamped = std::clamp(t, 0.0f, 1.0f);
		return ImVec4(
			lhs.x + ((rhs.x - lhs.x) * clamped),
			lhs.y + ((rhs.y - lhs.y) * clamped),
			lhs.z + ((rhs.z - lhs.z) * clamped),
			lhs.w + ((rhs.w - lhs.w) * clamped));
	}

	ImVec4 AccentColor(Ailurus::Widgets::ToastType type)
	{
		switch (type)
		{
		case Ailurus::Widgets::ToastType::Success:
			return ImVec4(0.27f, 0.74f, 0.53f, 1.0f);
		case Ailurus::Widgets::ToastType::Warning:
			return ImVec4(0.95f, 0.66f, 0.23f, 1.0f);
		case Ailurus::Widgets::ToastType::Error:
			return Ailurus::MaterialTheme::GetColorScheme().error;
		case Ailurus::Widgets::ToastType::Info:
		default:
			return Ailurus::MaterialTheme::GetColorScheme().primary;
		}
	}

	const char* IconForToast(Ailurus::Widgets::ToastType type)
	{
		switch (type)
		{
		case Ailurus::Widgets::ToastType::Success:
			return Ailurus::MaterialIcons::AutoAwesome;
		case Ailurus::Widgets::ToastType::Warning:
			return Ailurus::MaterialIcons::Tune;
		case Ailurus::Widgets::ToastType::Error:
			return Ailurus::MaterialIcons::Delete;
		case Ailurus::Widgets::ToastType::Info:
		default:
			return Ailurus::MaterialIcons::Console;
		}
	}
}

namespace Ailurus::Widgets
{
	void PushToast(const char* message, ToastType type, float durationSeconds)
	{
		if (message == nullptr || message[0] == '\0')
			return;

		g_toasts.push_back(ToastData{
			.id = g_nextToastId++,
			.message = message,
			.type = type,
			.duration = std::max(durationSeconds, 0.8f),
			.createdAt = ImGui::GetTime(),
		});
	}

	void RenderToasts()
	{
		if (g_toasts.empty())
			return;

		const auto& colors = MaterialTheme::GetColorScheme();
		const auto& shape = MaterialTheme::GetShapeScheme();
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const float width = 340.0f;
		const float height = 76.0f;
		const float margin = 22.0f;
		const float spacing = 10.0f;
		float y = viewport->WorkPos.y + viewport->WorkSize.y - margin;
		const double now = ImGui::GetTime();

		for (int index = static_cast<int>(g_toasts.size()) - 1; index >= 0; --index)
		{
			ToastData& toast = g_toasts[static_cast<size_t>(index)];
			const float age = static_cast<float>(now - toast.createdAt);
			const float target = age < toast.duration ? 1.0f : 0.0f;
			const ImGuiID animationId = static_cast<ImGuiID>(toast.id);
			const float visibility = Tween::Animate(animationId, target, 14.0f,
				target > 0.0f ? Tween::EaseType::EaseOut : Tween::EaseType::EaseIn);

			if (age > toast.duration + 0.45f && visibility <= 0.01f)
			{
				g_toasts.erase(g_toasts.begin() + index);
				continue;
			}

			const ImVec4 accent = AccentColor(toast.type);
			const ImVec4 bg = Mix(colors.surfaceContainerHigh, accent, 0.12f);
			const ImVec4 border = Mix(colors.outlineVariant, accent, 0.45f);

			y -= height;
			const float x = viewport->WorkPos.x + viewport->WorkSize.x - margin - width + ((1.0f - visibility) * 42.0f);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, shape.cornerRadiusLarge);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(bg.x, bg.y, bg.z, visibility));
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(border.x, border.y, border.z, visibility));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colors.onSurface.x, colors.onSurface.y, colors.onSurface.z, visibility));

			const std::string windowName = "##toast-" + std::to_string(toast.id);
			const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoNav
				| ImGuiWindowFlags_NoDocking;

			ImGui::Begin(windowName.c_str(), nullptr, flags);
			ImGui::TextUnformatted(IconForToast(toast.type));
			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::TextUnformatted(toast.type == ToastType::Info ? "Info" : toast.type == ToastType::Success ? "Success" : toast.type == ToastType::Warning ? "Warning" : "Error");
			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 280.0f);
			ImGui::TextWrapped("%s", toast.message.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndGroup();
			ImGui::End();

			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar(3);
			y -= spacing;
		}
	}
}