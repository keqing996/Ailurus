#include <algorithm>
#include "Ailurus/Systems/ImGuiSystem/Widgets/Card.h"
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"

namespace
{
	ImU32 ToU32(const ImVec4& color)
	{
		return ImGui::ColorConvertFloat4ToU32(color);
	}

	ImVec4 ShadowTint(const ImVec4& shadow, int elevation)
	{
		const float alpha = std::clamp(0.06f + (0.04f * static_cast<float>(elevation)), 0.06f, 0.24f);
		return ImVec4(shadow.x, shadow.y, shadow.z, alpha);
	}
}

namespace Ailurus::Widgets
{
	bool BeginCard(const char* label, int elevation, const ImVec2& size)
	{
		const auto& colors = MaterialTheme::GetColorScheme();
		const auto& shape = MaterialTheme::GetShapeScheme();

		ImGui::PushID(label);

		ImVec2 resolvedSize = size;
		const ImVec2 available = ImGui::GetContentRegionAvail();
		if (resolvedSize.x <= 0.0f)
			resolvedSize.x = available.x;

		if (resolvedSize.x > 0.0f && resolvedSize.y > 0.0f)
		{
			const ImVec2 min = ImGui::GetCursorScreenPos();
			const ImVec2 max = ImVec2(min.x + resolvedSize.x, min.y + resolvedSize.y);
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(min.x, min.y + 4.0f + static_cast<float>(elevation)),
				ImVec2(max.x, max.y + 4.0f + static_cast<float>(elevation)),
				ToU32(ShadowTint(colors.shadow, elevation)),
				shape.cornerRadiusMedium);
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, shape.cornerRadiusMedium);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, colors.surfaceContainer);
		ImGui::PushStyleColor(ImGuiCol_Border, colors.outlineVariant);

		const bool visible = ImGui::BeginChild("##card", resolvedSize,
			ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
			ImGuiWindowFlags_NoScrollbar);

		if (visible && label != nullptr && label[0] != '\0')
		{
			ImGui::TextUnformatted(label);
			ImGui::Spacing();
		}

		return visible;
	}

	void EndCard()
	{
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(3);
		ImGui::PopID();
	}
}