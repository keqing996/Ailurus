#include <algorithm>
#include "Ailurus/Systems/ImGuiSystem/Animation/Tween.h"
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Toggle.h"

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

	ImU32 ToU32(const ImVec4& color)
	{
		return ImGui::ColorConvertFloat4ToU32(color);
	}

	bool HasVisibleLabel(const char* label)
	{
		return label != nullptr && label[0] != '\0' && !(label[0] == '#' && label[1] == '#');
	}
}

namespace Ailurus::Widgets
{
	bool Toggle(const char* label, bool* value)
	{
		if (value == nullptr)
			return false;

		const auto& colors = MaterialTheme::GetColorScheme();

		ImGui::BeginGroup();
		ImGui::PushID(label != nullptr ? label : "toggle");

		const float height = ImGui::GetFrameHeight();
		const float width = height * 1.9f;
		const float radius = height * 0.5f;
		const ImVec2 pos = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton("##toggle", ImVec2(width, height));
		const bool changed = ImGui::IsItemClicked();
		if (changed)
			*value = !*value;

		const ImGuiID id = ImGui::GetItemID();
		const bool hovered = ImGui::IsItemHovered();
		const float progress = Tween::Animate(id, *value ? 1.0f : 0.0f, 12.0f);

		const ImVec4 offTrack = hovered
			? Mix(colors.surfaceVariant, colors.outline, 0.25f)
			: colors.surfaceVariant;
		const ImVec4 onTrack = hovered
			? Mix(colors.primaryContainer, colors.primary, 0.35f)
			: colors.primaryContainer;
		const ImVec4 trackColor = Tween::AnimateColor(id + 1, *value ? onTrack : offTrack, 12.0f);
		const ImVec4 knobColor = Tween::AnimateColor(id + 2,
			*value ? colors.primary : colors.onSurface, 12.0f, Tween::EaseType::EaseOut);

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), ToU32(trackColor), radius);

		const float knobRadius = std::max(radius - 3.0f, 3.0f);
		const float knobCenterX = (pos.x + radius) + ((width - (radius * 2.0f)) * progress);
		drawList->AddCircleFilled(ImVec2(knobCenterX, pos.y + radius), knobRadius, ToU32(knobColor));

		if (HasVisibleLabel(label))
		{
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
		}

		ImGui::PopID();
		ImGui::EndGroup();
		return changed;
	}
}