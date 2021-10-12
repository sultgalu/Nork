#include "../HierarchyPanel.h"

void Nork::Editor::HierarchyPanel::DrawContent()
{
	reg.each([this](entt::entity ent)
		{
			auto* name = this->reg.try_get<Components::Tag>(ent);

			std::string str;
			if (name == nullptr)
				str = std::string(("UNNAMED(ID=" + std::to_string(static_cast<int>(ent)) + ")").c_str());
			else
				str = name->tag;

			if (ImGui::Selectable(str.c_str(), data.selectedEnt == ent))
				data.selectedEnt = ent;
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				data.selectedEnt = ent;
				ImGui::OpenPopup("entRightClick");
			}
		});

	if (ImGui::BeginPopup("entRightClick"))
	{
		if (ImGui::Selectable("Delete"))
		{
			reg.destroy(data.selectedEnt);
			data.selectedEnt = entt::null;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::Separator();

	if (ImGui::Button("Add new"))
	{
		entt::entity newEnt = reg.create();
		auto& name = reg.emplace<Components::Tag>(newEnt).tag;
		name = std::string("ent #" + std::to_string(static_cast<int>(newEnt)));
		reg.emplace<Components::Transform>(newEnt);
	}

	/*ImGui::SliderFloat("Outliner Thickness", &scene.GetOutlinerThickness(), 0, 1);
	ImGui::ColorEdit4("Outliner Color", &(scene.GetOutlinerColor().r));*/
}
