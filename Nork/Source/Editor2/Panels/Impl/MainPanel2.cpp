#include "pch.h"
#include "../MainPanel.h"
#include "Platform/FileDialog.h"
#include "App/Application.h"

namespace Nork::Editor2
{
	MainPanel::MainPanel(EditorData& d) : Panel("MainPanel", d)
	{
	}
	void MainPanel::Begin()
	{
	}
	
	void MainPanel::DrawContent()
	{
		auto& input = Application::Get().engine.window.Input();
		if (input.IsDown(Key::Ctrl))
		{
			if (input.IsDown(Key::O))
			{
				LoadScene();
			}
			else if (input.IsDown(Key::S))
			{
				SaveScene();
			}
		}
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Open Scene..", "Ctrl+O"))
				{
					LoadScene();
				}
				if (ImGui::MenuItem("Save Scene..", "Ctrl+S"))
				{
					SaveScene();
				}
				ImGui::MenuItem("Save All..", "Ctrl+Shift+S");
				ImGui::EndMenu();
			}
			static Timer t;
			float ms = t.Reset();
			ImGui::Text(std::to_string(ImGui::GetIO().Framerate).c_str());
			ImGui::Text("fps");
			ImGui::Text(std::to_string(ms).c_str());
			ImGui::Text("ms");
			ImGui::EndMainMenuBar();
		}
	}

	void MainPanel::LoadScene()
	{
		using namespace FileDialog;
		std::string fileName = FileDialog::OpenFile(EngineFileTypes::None, L"Import Scene", L"Load");
		if (!fileName.empty())
		{
			data.engine.scene.Load(fileName);
			data.selectedPoly = nullptr;
			data.selectedNode = nullptr;
		}
	}

	void MainPanel::SaveScene()
	{
		using namespace FileDialog;
		std::string fileName = FileDialog::SaveFile(EngineFileTypes::None, L"Export Scene", L"Save");
		if (!fileName.empty())
			data.engine.scene.Save(fileName);
	}

	void MainPanel::End()
	{
		static constexpr ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::DockSpaceOverViewport(viewport, dockFlags);
	}
}