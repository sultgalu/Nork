#include "../MeshEditor.h"

namespace Nork::Editor
{
	static std::unordered_set<uint32_t> selected;
	static uint32_t current = 0;

	void MeshEditorPanel::SelectVertex(uint32_t i)
	{
		using namespace Input;
		auto& input = data.engine.window.GetInputState();
		if (input.Is(KeyType::Shift, KeyState::Down))
		{
			auto start = current > i ? i : current;
			auto end = current > i ? current : i;
			while (start <= end)
			{
				meshes.vertices[start].selected = 1;
				selected.insert(start++);
			}
		}
		else if (input.Is(KeyType::Ctrl, KeyState::Down))
		{
			if (selected.contains(i))
			{
				meshes.vertices[i].selected = 0;
				selected.erase(i);
			}
			else
			{
				meshes.vertices[i].selected = 1;
				selected.insert(i);
			}
		}
		else
		{
			for (uint32_t j : selected)
			{
				meshes.vertices[j].selected = 0;
			}
			selected.clear();
			meshes.vertices[i].selected = 1;
			selected.insert(i);
		}
		current = i;
	}
	void MeshEditorPanel::DrawContent()
	{
		glm::vec3 old = meshes.vertices[current].pos;
		glm::vec3 _new = old;
		if (ImGui::DragFloat3("position##meshowrld", &_new.x, 0.01f))
		{
			for (uint32_t i : selected)
			{
				meshes.vertices[i].pos += _new - old;
			}
		}

		if (ImGui::Button("Add##Vertex"))
		{
			uint32_t idx = meshes.Add(Engine::Vertex({ 0, 0, 0 }, true));
			selected.insert(idx);
		}

		if (ImGui::Button("Connect##Vertex"))
		{
			auto begin = selected.begin();
			for (auto first = selected.begin(); first != selected.end(); ++first)
			{
				for (auto second = std::next(first); second != selected.end(); ++second)
				{
					meshes.Connect(first._Ptr->_Myval, second._Ptr->_Myval);
				}
			}
		}

		if (ImGui::BeginTable("Meshes", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Vertices");
			ImGui::TableSetupColumn("Neighbours");

			ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
			ImGui::TableSetColumnIndex(0);
			ImGui::TableHeader("Vertices");
			ImGui::TableSetColumnIndex(1);
			ImGui::TableHeader("Neighbours");

			for (size_t i = 0; i < meshes.vertices.size(); i++)
			{
				auto rowName = std::to_string(i);
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::Selectable(std::to_string(i).c_str(), selected.contains(i)))
					SelectVertex(i);
				ImGui::TableSetColumnIndex(1);
				for (uint32_t n : meshes.neighbours[i])
				{
					if (ImGui::SmallButton(std::to_string(n).append("##").append(rowName).c_str()))
						SelectVertex(n);
					ImGui::SameLine();
				}
				ImGui::Text(""); // cause of sameline
			}

			ImGui::EndTable();
		}

		ImGui::Checkbox("Draw Triangles", &data.engine.drawTriangles);
		ImGui::Checkbox("Draw Lines", &data.engine.drawLines);
		ImGui::Checkbox("Draw Points", &data.engine.drawPoints);

		auto options = ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar;
		ImGui::ColorEdit4("Point Color", &data.engine.pointColor.r, options);
		ImGui::ColorEdit4("Line Color", &data.engine.lineColor.r, options);
		ImGui::ColorEdit4("Triangle Color", &data.engine.triangleColor.r, options);
		ImGui::ColorEdit3("Focused Color", &data.engine.selectedColor.r, options);
		ImGui::SliderFloat("Point alpha (focused)", &data.engine.pointAlpha, 0, 1, "%.2f");
		ImGui::SliderFloat("Line alpha (focused)", &data.engine.lineAlpha, 0, 1, "%.2f");
		ImGui::SliderFloat("Triangle alpha (focused)", &data.engine.triAlpha, 0, 1, "%.2f");
		ImGui::SliderFloat("Line Width", &data.engine.lineWidth, 0, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::SliderInt("Point Size", &data.engine.pointSize, 1, 1000, "%d", ImGuiSliderFlags_Logarithmic);
		ImGui::DragFloat("Point Internal size", &data.engine.pointInternalSize, 0.01f, 0, 1, "%.2f");
		ImGui::DragFloat("Point Anti-Aliasing", &data.engine.pointAA, 0.001, 0, 1, "%.3f");
	}
}