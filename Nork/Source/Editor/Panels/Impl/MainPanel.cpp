#include "pch.h"
#include "../MainPanel.h"
#include "Platform/FileDialog.h"

namespace Nork::Editor
{
	MainPanel::MainPanel(EditorData& d) : Panel("MainPanel", d)
	{
		this->events.Subscribe<Event::Types::KeyDown>([&](const Event::Types::KeyDown& ev)
			{
				using namespace Input;
				if (ev.key == Input::KeyType::S && data.engine.window.GetInputState().Is<KeyState::Down>(KeyType::Ctrl))
				{
					SaveScene();
				}
				else if (ev.key == Input::KeyType::Q && data.engine.window.GetInputState().Is<KeyState::Down>(KeyType::Ctrl))
				{
					LoadScene();
				}
			});
	}
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
			data.selectedEnt = data.engine.scene.registry.GetUnderlyingMutable().data()[0];
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