#include "pch.h"
#include "../AssetsPanel.h"

namespace Nork::Editor2
{
	void DrawProfileNode(Profiler::Node& node)
	{
		if (ImGui::TreeNodeEx(node.scope.data(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (node.entry != nullptr)
			{
				ImGui::Text(std::to_string(node.entry->actual).append("ms").c_str());
			}
			if (node.childs.size() > 0)
			{
				for (size_t i = 0; i < node.childs.size(); i++)
				{
					DrawProfileNode(*node.childs[i]);
				}
			}
			ImGui::TreePop();
		}
	}

	void AssetsPanel::DrawContent()
	{
		Profiler::GenerateTree();
		auto& root = Profiler::GetTree();
		DrawProfileNode(root);

		static char name[100];
		if (ImGui::InputTextWithHint("file", "<filename>", name, 100, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			name[0] = 'A';
		}

		if (ImGui::BeginTable("directory", 5, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
		{
			for (int i = 0; i < 15; i++)
			{
				ImGui::TableNextColumn();
				ImGui::Selectable(("item#" + std::to_string(i)).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);
			}
			ImGui::EndTable();
		}
		ImGui::Separator();
		if (ImGui::BeginTable("directory2", 3, ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_Resizable))
		{
			for (int i = 0; i < 10; i++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Selectable(("item#" + std::to_string(i)).c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
				ImGui::TableNextColumn();
				ImGui::Selectable(("coool" + std::to_string(i)).c_str(), false);
				ImGui::TableNextColumn();
				ImGui::Selectable(("row3_" + std::to_string(i)).c_str(), false);
			}
			ImGui::EndTable();
		}

		if (ImGui::BeginListBox("dir"))
		{
			for (int i = 0; i < 10; i++)
			{
				if (ImGui::Selectable(("item#" + std::to_string(i)).c_str()))
					ImGui::SetItemDefaultFocus();
				/*if (isSelected)
				ImGui::SetItemDefaultFocus();*/
			}
			ImGui::EndListBox();
		}
	}
}