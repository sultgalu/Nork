module Nork.Editor.Menus;

import Nork.FileDialog;

namespace Nork::Editor {
	void FileMenu::Content()
	{
		if (GetInput().IsDown(Key::Ctrl))
		{
			if (GetInput().IsDown(Key::O))
			{
				LoadScene();
			}
			else if (GetInput().IsDown(Key::S))
			{
				SaveScene();
			}
		}
		if (ImGui::MenuItem("Open Scene..", "Ctrl+O"))
		{
			LoadScene();
		}
		if (ImGui::MenuItem("Save Scene..", "Ctrl+S"))
		{
			SaveScene();
		}
		ImGui::MenuItem("Save All..", "Ctrl+Shift+S");
	}
	void FileMenu::LoadScene()
	{
		using namespace FileDialog;
		std::string fileName = FileDialog::OpenFile(EngineFileTypes::None, L"Import Scene", L"Load");
		if (!fileName.empty())
		{
			GetScene().Load(fileName);
			GetCommonData().selectedNode = nullptr;
		}
	}
	void FileMenu::SaveScene()
	{
		using namespace FileDialog;
		std::string fileName = FileDialog::SaveFile(EngineFileTypes::None, L"Export Scene", L"Save");
		if (!fileName.empty())
		{
			GetScene().Save(fileName);
		}
	}
}