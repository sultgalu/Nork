#include "pch.h"
#include "include/PhysicsSettingsPanel.h"

namespace Nork::Editor {
	PhysicsSettingsPanel::PhysicsSettingsPanel()
	{}

	void PhysicsSettingsPanel::Content()
	{
		auto& p = GetEngine().physicsSystem.pipeline;
		ImGui::DragFloat("Coefficient", &p.coefficient);

		bool g = p.g != 0;
		static float savedG = 0;
		ImGui::DragFloat("Gravity", &p.g);
		if (ImGui::Checkbox("Gravity##LongPeriod", &g))
		{
			if (g) p.g = savedG;
			else
			{
				savedG = p.g;
				p.g = 0;
			}
		}
		ImGui::SliderFloat("Coefficient", &p.coefficient, 0, 1);

		ImGui::Separator();

		ImGui::DragFloat("Physics speed", &GetEngine().physicsSystem.physicsSpeed, 0.001f, 0, 10, "%.3f", ImGuiSliderFlags_Logarithmic);
		ImGui::Checkbox("Update Velocities", &p.updateVelocities);
		ImGui::Checkbox("Update Rotation", &p.updateRotation);
		ImGui::Checkbox("Detect Collisions", &p.detectCollisions);
		ImGui::Checkbox("Handle Collisions", &p.handleCollisions);
	}
}

