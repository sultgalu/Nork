#include "pch.h"
#include "include/GraphicsSettingsPanel.h"

namespace Nork::Editor {
	GraphicsSettingsPanel::GraphicsSettingsPanel()
	{
		this->panelState.windowFlags |= ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar; // ::ImGuiWindowFlags_AlwaysHorizontalScrollbar;
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
		if (ImGui::TreeNodeEx("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static bool immediate = false;
			ImGui::Checkbox("Immediate mode", &immediate);
			bool changed = false;

			auto& b = r.bloom;
			changed |= ImGui::SliderFloat("Divider", &b.divider, 1.1f, 10);
			int high = b.highResY;
			if (changed |= ImGui::SliderInt("Y High Resolution", &high, 100, 1080))
			{
				b.highResY = high;
			}
			int low = b.lowResY;
			if (changed |= ImGui::SliderInt("Y Low Resolution", &low, 1, 1080, "%d", ImGuiSliderFlags_Logarithmic))
			{
				b.lowResY = low;
			}
			if (changed && immediate)
			{
				b.InitTextures();
			}
			if (ImGui::Button("Apply##BloomChanges"))
			{
				b.InitTextures();
			}

			if (ImGui::TreeNodeEx("Passes", ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_FramePadding))
			{
				ImGui::Indent(0);
				static int size = 10;
				ImGui::DragInt("Size", &size, 1, 1);
				for (size_t i = 0; i < b.fbs.size(); i++)
				{
					ImGui::Image((ImTextureID)b.fbs[i]->GetAttachments().colors[0].first->GetHandle(), { size * 16.f, size * 9.f }, ImVec2(0, 1), ImVec2(1, 0),
						ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
					ImGui::SameLine();
					ImGui::Image((ImTextureID)b.fbs2[i]->GetAttachments().colors[0].first->GetHandle(), { size * 16.f, size * 9.f }, ImVec2(0, 1), ImVec2(1, 0),
						ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	}
}
