#include "pch.h"
#include "../MainPanel.h"

namespace Nork::Editor
{
	void MainPanel::Begin()
	{
	}
	
	void MainPanel::DrawContent()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Open Scene..", "Ctrl+O"))
				{
					/*std::string fileName = FileDialog::OpenFile(EngineFileTypes::None, L"Import Scene", L"Load");
					if (!fileName.empty())
						scene.Load(fileName);*/
				}
				if (ImGui::MenuItem("Save Scene..", "Ctrl+S"))
				{
					/*std::string fileName = FileDialog::SaveFile(EngineFileTypes::None, L"Export Scene", L"Save");
					if (!fileName.empty())
						scene.Save(fileName);*/
				}
				ImGui::MenuItem("Save All..", "Ctrl+Shift+S");
				ImGui::EndMenu();
			}
			ImGui::Text(std::to_string(ImGui::GetIO().Framerate).c_str());
			ImGui::Text("fps");
			ImGui::EndMainMenuBar();
		}
	}


	void MainPanel::End()
	{
		static constexpr ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, dockFlags);
	}
}