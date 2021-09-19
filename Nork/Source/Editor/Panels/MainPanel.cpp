#include "pch.h"
#include "MainPanel.h"

namespace Nork::Editor
{
	void MainPanel::DrawContent()
	{
		static ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;

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
			ImGui::EndMainMenuBar();
		}

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, dockFlags);
	}
}