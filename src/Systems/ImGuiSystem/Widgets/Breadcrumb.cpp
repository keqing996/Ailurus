#include "Ailurus/Application.h"
#include "Ailurus/Systems/ImGuiSystem/Font/MaterialIcons.h"
#include "Ailurus/Systems/ImGuiSystem/Theme/MaterialTheme.h"
#include "Ailurus/Systems/ImGuiSystem/Widgets/Breadcrumb.h"

namespace Ailurus::Widgets
{
	int Breadcrumb(const std::vector<std::string>& path)
	{
		if (path.empty())
			return -1;

		const auto& colors = MaterialTheme::GetColorScheme();
		ImGuiSystem* imgui = Application::Get<ImGuiSystem>();
		int clicked = -1;

		for (size_t index = 0; index < path.size(); ++index)
		{
			if (index > 0)
			{
				ImGui::SameLine(0.0f, 6.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, colors.onSurfaceVariant);
				ImGui::TextUnformatted(MaterialIcons::ChevronRight);
				ImGui::PopStyleColor();
				ImGui::SameLine(0.0f, 6.0f);
			}

			if (index + 1 == path.size())
			{
				if (imgui != nullptr)
					ImGui::PushFont(imgui->GetFont(FontType::SemiBold));
				ImGui::TextUnformatted(path[index].c_str());
				if (imgui != nullptr)
					ImGui::PopFont();
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors.surfaceContainerHigh);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors.surfaceContainerHighest);
				if (ImGui::SmallButton(path[index].c_str()))
					clicked = static_cast<int>(index);
				ImGui::PopStyleColor(3);
			}

			if (index + 1 < path.size())
				ImGui::SameLine(0.0f, 0.0f);
		}

		return clicked;
	}
}