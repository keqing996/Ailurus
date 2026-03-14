#include "Ailurus/Systems/ImGuiSystem/Widgets/TreeView.h"
#include <imgui.h>

namespace
{
	Ailurus::Widgets::TreeNode* DrawTreeNode(Ailurus::Widgets::TreeNode* node)
	{
		if (node == nullptr || node->label == nullptr)
			return nullptr;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
		if (node->children.empty())
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (node->isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;
		if (node->isExpanded)
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		const bool open = (node->icon != nullptr && node->icon[0] != '\0')
			? ImGui::TreeNodeEx(reinterpret_cast<void*>(node), flags, "%s %s", node->icon, node->label)
			: ImGui::TreeNodeEx(reinterpret_cast<void*>(node), flags, "%s", node->label);

		node->isExpanded = open;

		Ailurus::Widgets::TreeNode* clicked = nullptr;
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			clicked = node;

		if (open && !node->children.empty())
		{
			for (Ailurus::Widgets::TreeNode* child : node->children)
			{
				if (Ailurus::Widgets::TreeNode* childClicked = DrawTreeNode(child))
					clicked = childClicked;
			}
			ImGui::TreePop();
		}

		return clicked;
	}
}

namespace Ailurus::Widgets
{
	TreeNode* TreeView(const char* id, const std::vector<TreeNode*>& roots)
	{
		TreeNode* clicked = nullptr;
		ImGui::PushID(id != nullptr ? id : "tree-view");
		for (TreeNode* root : roots)
		{
			if (TreeNode* nodeClicked = DrawTreeNode(root))
				clicked = nodeClicked;
		}
		ImGui::PopID();
		return clicked;
	}
}