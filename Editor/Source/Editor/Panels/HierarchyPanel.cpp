#include "include/HierarchyPanel.h"
#include "Components/All.h"

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

		int flags = ImGuiTreeNodeFlags_DefaultOpen;
		if (node->GetChildren().empty())
			flags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;
		
		if (node == GetCommonData().selectedNode)
			flags |= flags | ImGuiTreeNodeFlags_Selected;
		if (true) {
			flags |= ImGuiTreeNodeFlags_OpenOnArrow;
			flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick;
			//| ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

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
			if (ImGui::Selectable("Duplicate"))
			{
				GetCommonData().selectedNode = GetScene().CreateNode(node->GetParent());
				Duplicate(node, GetCommonData().selectedNode);
				ImGui::CloseCurrentPopup();
			}
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
			for (size_t i = 0; i < node->GetChildren().size(); i++)
			{
				RecursiveDraw(node->GetChildren()[i]);
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
	struct CompAdder {
		Entity& from, &to;
		template<class T>
		void TryCopy() {
			if (auto comp = from.TryGetComponent<T>()) {
				to.AddComponent<T>() = *comp;
			}
		}
		template<class T>
		void TryCopy(std::function<void(const T&, T&)> f) {
			if (auto comp = from.TryGetComponent<T>()) {
				auto& addedComp = to.AddComponent<T>();
				f(*comp, addedComp);
			}
		}
	};
	void HierarchyPanel::Duplicate(std::shared_ptr<SceneNode> from, std::shared_ptr<SceneNode> to)
	{
		CompAdder adder{ .from = from->GetEntity(), .to = to->GetEntity() };
		adder.TryCopy<Components::Transform>();
		adder.TryCopy<Components::Tag>([](const Components::Tag& from, Components::Tag& to) {
			to.tag = from.tag + " Copy";
		});
		adder.TryCopy<Components::Physics>([](const Components::Physics& from, Components::Physics& to) {
			to.Object() = from.Object();
		});
	}
}