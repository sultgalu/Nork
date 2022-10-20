module Nork.Editor.Panels;

namespace Nork::Editor {
	GraphicsSettingsPanel::GraphicsSettingsPanel()
	{
	}

	void GraphicsSettingsPanel::Content()
	{
		auto& r = GetEngine().renderingSystem;

		if (ImGui::TreeNodeEx("Debug"))
		{
			auto& g = r.globalShaderUniform;
			auto colorOptions = ImGuiColorEditFlags_DefaultOptions_ | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaBar;
			ImGui::SliderInt("Point Size", &g.pointSize, 1, 1000, "%d", ImGuiSliderFlags_Logarithmic);
			ImGui::DragFloat("Point Internal size", &g.pointInternalSize, 0.01f, 0, 1, "%.2f");
			ImGui::DragFloat("Point Anti-Aliasing", &g.pointAA, 0.001, 0, 1, "%.3f");
			ImGui::ColorEdit4("Point Color", &g.pointColor.r, colorOptions);
			ImGui::SliderFloat("Point alpha (focused)", &g.pointAlpha, 0, 1, "%.2f");

			ImGui::SliderFloat("Line Width", &g.lineWidth, 0, 1, "%.3f", ImGuiSliderFlags_Logarithmic);
			ImGui::ColorEdit4("Line Color", &g.lineColor.r, colorOptions);
			ImGui::SliderFloat("Line alpha (focused)", &g.lineAlpha, 0, 1, "%.2f");

			ImGui::ColorEdit4("Triangle", &g.triColor.r, colorOptions);

			ImGui::ColorEdit3("Focused Color", &g.selectedColor.r, colorOptions);
			ImGui::TreePop();
		}
	}
}
