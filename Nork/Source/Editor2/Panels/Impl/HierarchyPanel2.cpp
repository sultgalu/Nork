#include "../HierarchyPanel.h"

namespace Nork::Editor2 {
	static bool del = false;
	static void RecursiveDraw(EditorData& data, SceneNode& node)
	{
		auto& ent = node.GetEntity();
		auto* name = ent.TryGetComponent<Components::Tag>();

		std::string str;
		if (name == nullptr)
			str = std::string(("UNNAMED(ID=" + std::to_string(static_cast<int>(ent.Id())) + ")").c_str());
		else
			str = name->tag;

		// if (ImGui::Selectable(str.c_str(), data.selectedNode == &node))
		// {
		// 	data.selectedNode = &node;
		// }

		auto flags = ImGuiTreeNodeFlags_DefaultOpen;
		if (node.GetChildren().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;
		if (&node == data.selectedNode)
			flags |= ImGuiTreeNodeFlags_Selected;
		if (true)
			flags |= ImGuiTreeNodeFlags_OpenOnArrow 
			| ImGuiTreeNodeFlags_OpenOnDoubleClick;

		bool open = ImGui::TreeNodeEx(str.c_str(), flags);
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			data.selectedNode = &node;
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			data.selectedNode = &node;
			ImGui::OpenPopup("entRightClick");
		}
		if (ImGui::BeginPopup("entRightClick"))
		{
			if (ImGui::Selectable("Delete"))
			{
				del = true;
				// data.engine.scene.DeleteNode(*data.selectedNode);
				// data.selectedNode = nullptr;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Orphan"))
			{
				while (!data.selectedNode->GetChildren().empty())
				{
					data.engine.scene.DeleteNode(*data.selectedNode->GetChildren().front());
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (open)
		{
			for (auto& child : node.GetChildren())
			{
				RecursiveDraw(data, *child);
			}
			ImGui::TreePop();
		}
		// ImGui::Indent(10);
		// ImGui::Unindent(10);
	}
}

void Nork::Editor2::HierarchyPanel::DrawContent()
{
	del = false;
	RecursiveDraw(data, *data.engine.scene.root);
	if (del)
	{
		data.engine.scene.DeleteNode(*data.selectedNode);
		data.selectedNode = nullptr;
	}

	ImGui::Separator();

	if (ImGui::Button("Add new"))
	{
		entt::entity newEnt = reg.create();
		auto& name = reg.emplace<Components::Tag>(newEnt).tag;
		name = std::string("ent #" + std::to_string(static_cast<int>(newEnt)));
		reg.emplace<Components::Transform>(newEnt);
	}
	//ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - ImGui::GetTextLineHeight());
	if (ImGui::Button("Reset to empty"))
	{
		ImGui::OpenPopup("SceneResetPopup");
	}
	if (ImGui::BeginPopupModal("SceneResetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Reset The Whole Scene To Empty?");
		if (ImGui::Button("Reset"))
		{
			data.engine.scene.Reset();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	/*ImGui::SliderFloat("Outliner Thickness", &scene.GetOutlinerThickness(), 0, 1);
	ImGui::ColorEdit4("Outliner Color", &(scene.GetOutlinerColor().r));*/
}
