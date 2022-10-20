module Nork.Editor.Panels;

import Nork.Scene;
import Nork.Components;

namespace Nork::Editor {
	static bool del = false;

	static std::shared_ptr<SceneNode> addChild = nullptr;
	static std::shared_ptr<SceneNode> to = nullptr;

	void HierarchyPanel::RecursiveDraw(std::shared_ptr<SceneNode> node)
	{
		auto& ent = node->GetEntity();
		auto* name = ent.TryGetComponent<Components::Tag>();

		std::string str;
		if (name == nullptr)
			str = std::string(("UNNAMED(ID=" + std::to_string(static_cast<int>(ent.Id())) + ")").c_str());
		else
			str = name->tag;

		auto flags = ImGuiTreeNodeFlags_DefaultOpen;
		if (node->GetChildren().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;
		
		if (node == GetCommonData().selectedNode)
			flags |= ImGuiTreeNodeFlags_Selected;
		if (true)
			flags |= ImGuiTreeNodeFlags_OpenOnArrow
			| ImGuiTreeNodeFlags_OpenOnDoubleClick;
			//| ImGuiTreeNodeFlags_NoTreePushOnOpen;

		bool open = ImGui::TreeNodeEx(str.c_str(), flags);
		constexpr int dragDrogVerifier = 24234;
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("NodeDrag", &dragDrogVerifier, sizeof(dragDrogVerifier));
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			auto payload = ImGui::AcceptDragDropPayload("NodeDrag");
			if (payload && payload->DataSize == sizeof(dragDrogVerifier) && *(const int*)(payload->Data) == dragDrogVerifier)
			{
				addChild = GetCommonData().selectedNode;
				to = node;
			}
			ImGui::EndDragDropTarget();
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			GetCommonData().selectedNode = node;
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			GetCommonData().selectedNode = node;
			ImGui::OpenPopup("entRightClick");
		}
		if (ImGui::BeginPopup("entRightClick"))
		{
			if (ImGui::Selectable("Add Child"))
			{
				GetCommonData().selectedNode = GetScene().CreateNode(*GetCommonData().selectedNode);
				auto& entity = GetCommonData().selectedNode->GetEntity();
				entity.AddComponent<Components::Tag>().tag =
					std::string("ent #" + std::to_string(static_cast<int>(entity.Id())));
				entity.AddComponent<Components::Transform>();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Delete"))
			{
				del = true;
				// data.engine.scene.DeleteNode(*data.selectedNode);
				// data.selectedNode = nullptr;
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Selectable("Orphan"))
			{
				while (!GetCommonData().selectedNode->GetChildren().empty())
				{
					GetScene().DeleteNode(*GetCommonData().selectedNode->GetChildren().front());
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (open)
		{
			for (auto& child : node->GetChildren())
			{
				RecursiveDraw(child);
			}
			ImGui::TreePop();
		}
	}
	HierarchyPanel::HierarchyPanel()
	{
	}
	void HierarchyPanel::Content()
	{
		del = false;
		RecursiveDraw(GetScene().root);
		if (del)
		{
			GetScene().DeleteNode(*GetCommonData().selectedNode);
			GetCommonData().selectedNode = nullptr;
		}
		if (addChild && to)
		{
			to->AddChild(addChild);
			to = nullptr;
			addChild = nullptr;
		}

		ImGui::Separator();

		if (ImGui::Button("Add new"))
		{
			GetCommonData().selectedNode = GetScene().CreateNode();
			auto& entity = GetCommonData().selectedNode->GetEntity();
			entity.AddComponent<Components::Tag>().tag = 
				std::string("ent #" + std::to_string(static_cast<int>(entity.Id())));
			entity.AddComponent<Components::Transform>();
		}

		if (ImGui::Button("Reset to empty"))
		{
			ImGui::OpenPopup("SceneResetPopup");
		}
		if (ImGui::BeginPopupModal("SceneResetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Reset The Whole Scene To Empty?");
			if (ImGui::Button("Reset"))
			{
				GetScene().Reset();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}