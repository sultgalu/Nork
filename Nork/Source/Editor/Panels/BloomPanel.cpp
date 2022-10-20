module Nork.Editor.Panels;

namespace Nork::Editor {
	BloomPanel::BloomPanel()
	{
		this->panelState.windowFlags |= ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar
			| ImGuiWindowFlags_MenuBar;
	}
	void BloomPanel::Content()
	{
		auto& b = GetEngine().renderingSystem.bloom;
		static int size = 10;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Settings"))
			{
				ImGui::SliderFloat("Divider", &b.divider, 1.1f, 10);
				int high = b.highResY;
				if (ImGui::SliderInt("Y High Resolution", &high, 100, 1080))
				{
					b.highResY = high;
				}
				int low = b.lowResY;
				if (ImGui::SliderInt("Y Low Resolution", &low, 1, 1080, "%d", ImGuiSliderFlags_Logarithmic))
				{
					b.lowResY = low;
				}
				if (ImGui::Button("Apply##BloomChanges"))
				{
					b.InitTextures();
				}
				ImGui::EndMenu();
			}
			ImGui::DragInt("Size", &size, 1, 1);

			ImGui::EndMenuBar();
		}

		for (size_t i = 0; i < b.fbs.size(); i++)
		{
			ImGui::Image((ImTextureID)b.fbs[i]->GetAttachments().colors[0].first->GetHandle(), { size * 16.f, size * 9.f }, ImVec2(0, 1), ImVec2(1, 0),
				ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::SameLine();
			ImGui::Image((ImTextureID)b.fbs2[i]->GetAttachments().colors[0].first->GetHandle(), { size * 16.f, size * 9.f }, ImVec2(0, 1), ImVec2(1, 0),
				ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}

		if (ImGui::IsWindowHovered() && GetInput().IsDown(Key::Ctrl))
		{
			float offs = GetInput().ScrollOffs();
			if (offs != 0)
			{
				offs = size * offs * 0.1f;
				if (std::abs(offs) < 1)
				{
					offs = offs > 0 ? 1 : -1;
				}
				size += offs;

				if (size < 1)
				{
					size = 1;
				}
			}
		}
	}
}
