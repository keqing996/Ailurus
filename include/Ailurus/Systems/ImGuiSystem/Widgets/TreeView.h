#pragma once

#include <vector>

namespace Ailurus::Widgets
{
	struct TreeNode
	{
		const char* label = nullptr;
		const char* icon = nullptr;
		bool isSelected = false;
		bool isExpanded = false;
		void* userData = nullptr;
		std::vector<TreeNode*> children;
	};

	TreeNode* TreeView(const char* id, const std::vector<TreeNode*>& roots);
}