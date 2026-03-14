#include <cfloat>
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/PropertyGrid.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Toggle.h"
#include <imgui.h>

namespace
{
	void BeginPropertyRow(const char* label)
	{
		const auto& colors = Ailurus::MaterialTheme::GetColorScheme();
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::PushStyleColor(ImGuiCol_Text, colors.onSurfaceVariant);
		ImGui::TextUnformatted(label);
		ImGui::PopStyleColor();
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);
	}
}

namespace Ailurus::Widgets
{
	bool BeginPropertyGrid(const char* label)
	{
		const auto& colors = MaterialTheme::GetColorScheme();
		ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(colors.surfaceContainer.x, colors.surfaceContainer.y, colors.surfaceContainer.z, 0.35f));
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(colors.surfaceContainerHigh.x, colors.surfaceContainerHigh.y, colors.surfaceContainerHigh.z, 0.35f));
		ImGui::PushStyleColor(ImGuiCol_Border, colors.outlineVariant);

		const ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchProp
			| ImGuiTableFlags_BordersInnerV
			| ImGuiTableFlags_RowBg
			| ImGuiTableFlags_PadOuterX;

		if (!ImGui::BeginTable(label, 2, flags))
		{
			ImGui::PopStyleColor(3);
			return false;
		}

		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 156.0f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
		return true;
	}

	void EndPropertyGrid()
	{
		ImGui::EndTable();
		ImGui::PopStyleColor(3);
	}

	void PropertyLabel(const char* label)
	{
		BeginPropertyRow(label);
	}

	bool PropertyFloat(const char* label, float* value, float min, float max)
	{
		if (value == nullptr)
			return false;

		BeginPropertyRow(label);
		ImGui::PushID(label);
		const bool changed = max > min
			? ImGui::SliderFloat("##value", value, min, max)
			: ImGui::DragFloat("##value", value, 0.1f);
		ImGui::PopID();
		return changed;
	}

	bool PropertyVec3(const char* label, float value[3])
	{
		if (value == nullptr)
			return false;

		BeginPropertyRow(label);
		ImGui::PushID(label);
		const bool changed = ImGui::DragFloat3("##value", value, 0.05f);
		ImGui::PopID();
		return changed;
	}

	bool PropertyColor(const char* label, float color[4])
	{
		if (color == nullptr)
			return false;

		BeginPropertyRow(label);
		ImGui::PushID(label);
		const bool changed = ImGui::ColorEdit4("##value", color,
			ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Float);
		ImGui::PopID();
		return changed;
	}

	bool PropertyBool(const char* label, bool* value)
	{
		if (value == nullptr)
			return false;

		BeginPropertyRow(label);
		ImGui::PushID(label);
		const bool changed = Toggle("##value", value);
		ImGui::PopID();
		return changed;
	}

	bool PropertyEnum(const char* label, int* current, const char* const* items, int count)
	{
		if (current == nullptr || items == nullptr || count <= 0)
			return false;

		BeginPropertyRow(label);
		ImGui::PushID(label);
		const bool changed = ImGui::Combo("##value", current, items, count);
		ImGui::PopID();
		return changed;
	}

	bool PropertyText(const char* label, char* buf, size_t bufSize)
	{
		if (buf == nullptr || bufSize == 0)
			return false;

		BeginPropertyRow(label);
		ImGui::PushID(label);
		const bool changed = ImGui::InputText("##value", buf, bufSize);
		ImGui::PopID();
		return changed;
	}
}