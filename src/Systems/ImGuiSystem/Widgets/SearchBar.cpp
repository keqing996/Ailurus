#include <algorithm>
#include <cfloat>
#include "Ailurus/Systems/ImGuiSystem/Font/MaterialIcons.h"
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/SearchBar.h"

namespace Ailurus::Widgets
{
	bool SearchBar(const char* hint, char* buf, size_t bufSize, const ImVec2& size)
	{
		if (buf == nullptr || bufSize == 0)
			return false;

		const auto& colors = MaterialTheme::GetColorScheme();
		const auto& shape = MaterialTheme::GetShapeScheme();
		const ImGuiStyle& style = ImGui::GetStyle();

		ImGui::PushID(hint != nullptr ? hint : "search-bar");
		ImGui::BeginGroup();
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, shape.cornerRadiusFull);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, colors.surfaceContainerHigh);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, colors.surfaceContainerHighest);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, colors.primaryContainer);

		ImGui::AlignTextToFramePadding();
		ImGui::PushStyleColor(ImGuiCol_Text, colors.onSurfaceVariant);
		ImGui::TextUnformatted(MaterialIcons::Search);
		ImGui::PopStyleColor();
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);

		if (size.x > 0.0f)
		{
			const float iconWidth = ImGui::CalcTextSize(MaterialIcons::Search).x;
			const float targetWidth = std::max(1.0f, size.x - iconWidth - style.ItemInnerSpacing.x);
			ImGui::SetNextItemWidth(targetWidth);
		}
		else if (size.x < 0.0f)
		{
			ImGui::SetNextItemWidth(-FLT_MIN);
		}

		const bool changed = ImGui::InputTextWithHint("##search", hint, buf, bufSize);

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::EndGroup();
		ImGui::PopID();
		return changed;
	}
}