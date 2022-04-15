#include "../MeshEditor.h"
#include "Modules/Physics/Pipeline/CollisionDetectionCPU.h"
#include "App/Application.h"

namespace Nork::Editor2
{
	static std::unordered_set<uint32_t> selected;
	static uint32_t current = 0;
	static uint32_t activeMesh;
	MeshEditorPanel::MeshEditorPanel(EditorData& d)
		:Panel("Mesh Editor", d)
	{}
	void MeshEditorPanel::DrawContent()
	{
		if (data.selectedPoly == nullptr)
		{
			ImGui::Text("Select a Poly from the inspector");
		}
		else
		{
			glm::vec3 old = data.selectedPoly->vertices[current];
			glm::vec3 _new = old;
			
			data.idQueryMode.set(IdQueryMode::Click);

			static float scaleSpeed = 0.01f;
			ImGui::InputFloat("Scale Drag Speed By", &scaleSpeed);


			glm::vec3 scale = glm::vec3(1);
			if (ImGui::DragFloat3("Scale#ASDAD", &scale.x, scaleSpeed))
			{
				for (size_t i = 0; i < data.selectedPoly->vertices.size(); i++)
				{
					data.selectedPoly->vertices[i] *= scale;
				}
			}

			if (ImGui::DragFloat3("position##meshowrld", &_new.x, 0.01f))
			{
				for (uint32_t i : selected)
				{
					data.selectedPoly->vertices[i] += _new - old;
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

				data.selectedPoly->vertices[0] += glm::vec3(1, 1, 1);
				data.selectedPoly->vertices[0] += glm::vec3(1, 1, 1);

				selected.clear();
			}

			if (ImGui::Button("Add##Vertex"))
			{
				uint32_t idx = data.selectedPoly->Add({ 0, 0, 0 });
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
						; //SelectVertex(i);
					ImGui::TableSetColumnIndex(1);
					for (uint32_t n : data.selectedPoly->neighbours[i])
					{
						if (ImGui::SmallButton(std::to_string(n).append("##").append(rowName).c_str()))
							;//SelectVertex(n);
						ImGui::SameLine();
					}
					ImGui::Text(""); // cause of sameline
				}

				ImGui::EndTable();
			}
		}
		
		ImGui::Text(std::string("AABB DELTA: ").append(std::to_string(Physics::AABBTest::GetDelta())).append("ms").c_str());
		ImGui::Text("AABB result:");
		if (data.engine.physicsSystem.aabbRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");
		ImGui::Text("Clip result:");
		if (data.engine.physicsSystem.clipRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");
		ImGui::Text("GJK result:");
		if (data.engine.physicsSystem.gjkRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");
		ImGui::Text("SAT result:");
		if (data.engine.physicsSystem.satRes) ImGui::TextColored(ImVec4(0, 1, 0, 1), "  COLLISION!!");
		else ImGui::TextColored(ImVec4(1, 0, 0, 1), "  no collision");

		bool renderColliders = data.engine.renderingSystem.colliderVao != nullptr;
		if (ImGui::Checkbox("Render Colliders", &renderColliders))
		{
			data.engine.renderingSystem.SetRenderColliders(renderColliders);
		}
		bool physicsUpdate = data.engine.physicsUpdate;
		if (ImGui::Checkbox("Physics Update##LongPeriod", &physicsUpdate))
		{
			if (physicsUpdate)
				data.engine.StartPhysics();
			else
				data.engine.StopPhysics();
		}

		bool g = data.engine.physicsSystem.pipeline.g != 0;
		static float savedG = 0;
		ImGui::DragFloat("Gravity", &data.engine.physicsSystem.pipeline.g);
		if (ImGui::Checkbox("Gravity##LongPeriod", &g))
		{
			if (g) data.engine.physicsSystem.pipeline.g = savedG;
			else
			{
				savedG = data.engine.physicsSystem.pipeline.g;
				data.engine.physicsSystem.pipeline.g = 0;
			}
		}
		ImGui::SliderFloat("Coefficient", &data.engine.physicsSystem.pipeline.coefficient, 0, 1);

		ImGui::Separator();

		ImGui::DragFloat("Physics speed", &data.engine.physicsSystem.physicsSpeed, 0.001f, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::Checkbox("Update polies", &data.engine.physicsSystem.updatePoliesForPhysics);
		ImGui::Checkbox("Update Velocities", &data.engine.physicsSystem.pipeline.updateVelocities);
		ImGui::Checkbox("Update Rotation", &data.engine.physicsSystem.pipeline.updateRotation);
		ImGui::Checkbox("Detect Collisions", &data.engine.physicsSystem.pipeline.detectCollisions);
		ImGui::Checkbox("Handle Collisions", &data.engine.physicsSystem.pipeline.handleCollisions);
		ImGui::Checkbox("Draw Polies", &data.engine.physicsSystem.drawPolies);
		ImGui::Checkbox("Draw Triangles", &data.engine.physicsSystem.drawTriangles);
		ImGui::Checkbox("Draw Lines", &data.engine.physicsSystem.drawLines);
		ImGui::Checkbox("Draw Points", &data.engine.physicsSystem.drawPoints);

		auto options = ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar;
		ImGui::SliderInt("Point Size", &data.engine.renderingSystem.globalShaderUniform.pointSize, 1, 1000, "%d", ImGuiSliderFlags_Logarithmic);
		ImGui::DragFloat("Point Internal size", &data.engine.renderingSystem.globalShaderUniform.pointInternalSize, 0.01f, 0, 1, "%.2f");
		ImGui::DragFloat("Point Anti-Aliasing", &data.engine.renderingSystem.globalShaderUniform.pointAA, 0.001, 0, 1, "%.3f");
		ImGui::ColorEdit4("Point Color", &data.engine.renderingSystem.globalShaderUniform.pointColor.r, options);
		ImGui::SliderFloat("Point alpha (focused)", &data.engine.renderingSystem.globalShaderUniform.pointAlpha, 0, 1, "%.2f");

		ImGui::SliderFloat("Line Width", &data.engine.renderingSystem.globalShaderUniform.lineWidth, 0, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::ColorEdit4("Line Color", &data.engine.renderingSystem.globalShaderUniform.lineColor.r, options);
		ImGui::SliderFloat("Line alpha (focused)", &data.engine.renderingSystem.globalShaderUniform.lineAlpha, 0, 1, "%.2f");

		ImGui::ColorEdit4("Triangle", &data.engine.renderingSystem.globalShaderUniform.triColor.r, options);

		ImGui::ColorEdit3("Focused Color", &data.engine.renderingSystem.globalShaderUniform.selectedColor.r, options);
	}
}