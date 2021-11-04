#include "../MeshEditor.h"

namespace Nork::Editor
{
	static std::unordered_set<uint32_t> selected;
	static uint32_t current = 0;
	static uint32_t activeMesh;
	MeshEditorPanel::MeshEditorPanel(EditorData& d)
		:Panel("Mesh Editor", d)
	{
		using namespace Event::Types;
		data.engine.appEventMan.GetReceiver().Subscribe<IdQueryResult>([&](const IdQueryResult& e)
			{
				if (data.selectedPoly != nullptr)
				{
					auto& verts = data.selectedPoly->vertices;
					for (size_t i = 0; i < verts.size(); i++)
					{
						if (verts[i].id == e.id)
						{
							SelectVertex(i);
							return;
						}
					}
				}
				
			});
	}
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
				data.selectedPoly->vertices[start].selected = 1;
				selected.insert(start++);
			}
		}
		else if (input.Is(KeyType::Ctrl, KeyState::Down))
		{
			if (selected.contains(i))
			{
				data.selectedPoly->vertices[i].selected = 0;
				selected.erase(i);
			}
			else
			{
				data.selectedPoly->vertices[i].selected = 1;
				selected.insert(i);
			}
		}
		else
		{
			for (uint32_t j : selected)
			{
				data.selectedPoly->vertices[j].selected = 0;
			}
			selected.clear();
			data.selectedPoly->vertices[i].selected = 1;
			selected.insert(i);
		}
		current = i;
	}
	void MeshEditorPanel::DrawContent()
	{
		if (data.selectedPoly == nullptr)
		{
			ImGui::Text("Select a Poly from the inspector");
		}
		else
		{
			glm::vec3 old = data.selectedPoly->vertices[current].pos;
			glm::vec3 _new = old;
			
			data.idQueryMode.set(IdQueryMode::Click);

			static float scaleSpeed = 0.01f;
			ImGui::InputFloat("Scale Drag Speed By", &scaleSpeed);


			glm::vec3 scale = glm::vec3(1);
			if (ImGui::DragFloat3("Scale#ASDAD", &scale.x, scaleSpeed))
			{
				for (size_t i = 0; i < data.selectedPoly->vertices.size(); i++)
				{
					data.selectedPoly->vertices[i].pos *= scale;
				}
			}

			if (ImGui::DragFloat3("position##meshowrld", &_new.x, 0.01f))
			{
				for (uint32_t i : selected)
				{
					data.selectedPoly->vertices[i].pos += _new - old;
				}
			}

			if (ImGui::Button("SetupTetras##Vertex"))
			{
				for (size_t i = 0; i < 4; i++)
				{
					for (size_t j = 0; j < 4; j++)
					{
						data.selectedPoly->Connect(i, j);
						data.selectedPoly->Connect(i, j);
					}
				}

				for (size_t i = 7; i > 3; i--)
				{
					data.selectedPoly->Remove(i);
					data.selectedPoly->Remove(i);
				}

				data.selectedPoly->vertices[0].pos += glm::vec3(1, 1, 1);
				data.selectedPoly->vertices[0].pos += glm::vec3(1, 1, 1);

				selected.clear();
			}

			if (ImGui::Button("Add##Vertex"))
			{
				uint32_t idx = data.selectedPoly->Add(Components::Vertex({ 0, 0, 0 }, true));
				selected.insert(idx);
			}
			if (ImGui::Button("Remove##Vertex"))
			{
				std::vector<uint32_t> toDel;
				toDel.reserve(selected.size());
				for (auto idx : selected)
				{
					toDel.push_back(idx);
				}
				data.selectedPoly->Remove(toDel);
				selected.clear();
				current = 0;
			}

			if (ImGui::Button("Connect##Vertex"))
			{
				auto begin = selected.begin();
				for (auto first = selected.begin(); first != selected.end(); ++first)
				{
					for (auto second = std::next(first); second != selected.end(); ++second)
					{
						data.selectedPoly->Connect(first._Ptr->_Myval, second._Ptr->_Myval);
					}
				}
			}
			if (ImGui::Button("Disconnect##Vertex"))
			{
				auto begin = selected.begin();
				for (auto first = selected.begin(); first != selected.end(); ++first)
				{
					for (auto second = std::next(first); second != selected.end(); ++second)
					{
						data.selectedPoly->Disconnect(first._Ptr->_Myval, second._Ptr->_Myval);
					}
				}
			}
			if (ImGui::Button("Select all##Vertex"))
			{
				for (size_t i = 0; i < data.selectedPoly->vertices.size(); i++)
				{
					selected.insert(i);
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

				for (size_t i = 0; i < data.selectedPoly->vertices.size(); i++)
				{
					auto rowName = std::to_string(i);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					if (ImGui::Selectable(std::to_string(i).c_str(), selected.contains(i)))
						SelectVertex(i);
					ImGui::TableSetColumnIndex(1);
					for (uint32_t n : data.selectedPoly->neighbours[i])
					{
						if (ImGui::SmallButton(std::to_string(n).append("##").append(rowName).c_str()))
							SelectVertex(n);
						ImGui::SameLine();
					}
					ImGui::Text(""); // cause of sameline
				}

				ImGui::EndTable();
			}
		}
		
		ImGui::Text("AABB result:");
		if (data.engine.aabbRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");
		ImGui::Text("Clip result:");
		if (data.engine.clipRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");
		ImGui::Text("GJK result:");
		if (data.engine.gjkRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");
		ImGui::Text("SAT result:");
		if (data.engine.satRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");

		static bool setBack = false;
		if (setBack)
		{
			data.engine.physicsUpdate = false;
			setBack = false;
		}
		if (data.engine.satRes)
		{
			if(ImGui::Button("Physics Update"))
			{
				data.engine.physicsUpdate = true;
				setBack = true;
			}
		}

		ImGui::Checkbox("Physics Update##LongPeriod", &data.engine.physicsUpdate);

		bool g = data.engine.pSystem.g != 0;
		static float savedG = 0;
		ImGui::DragFloat("Gravity", &data.engine.pSystem.g);
		if (ImGui::Checkbox("Gravity##LongPeriod", &g))
		{
			if (g) data.engine.pSystem.g = savedG;
			else
			{
				savedG = data.engine.pSystem.g;
				data.engine.pSystem.g = 0;
			}
		}

		ImGui::SliderFloat("Coefficient", &data.engine.pSystem.coefficient, 0, 1);
		ImGui::Checkbox("Update Velocities", &data.engine.pSystem.updateVelocities);
		ImGui::Checkbox("Detect Collisions", &data.engine.pSystem.detectCollisions);
		ImGui::Checkbox("Resolve Collisions", &data.engine.pSystem.resolveCollisions);
		ImGui::Checkbox("AABB", &data.engine.pSystem.aabb);
		ImGui::Checkbox("GJK", &data.engine.pSystem.gjk);
		if (data.engine.pSystem.gjkRes)
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "Coll");
		else
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "no coll");
		ImGui::Checkbox("SAT", &data.engine.sat);
		ImGui::Checkbox("Clip", &data.engine.clip);
		ImGui::Checkbox("Draw Polies", &data.engine.drawPolies);
		ImGui::Checkbox("Draw Triangles", &data.engine.drawTriangles);
		ImGui::Checkbox("Draw Lines", &data.engine.drawLines);
		ImGui::Checkbox("Draw Points", &data.engine.drawPoints);
		ImGui::Checkbox("Draw Sky", &data.engine.drawSky);

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