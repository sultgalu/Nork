#include "include/FileMenu.h"
#include "Platform/FileDialog.h"
#include "Editor/Editor.h"

namespace Nork::Editor {
void FileMenu::Shortcuts() {
	if (GetInput().IsDown(Key::Ctrl))
	{
		if (GetInput().IsJustPressed(Key::O))
		{
			LoadScene();
		}
		else if (GetInput().IsJustPressed(Key::S))
		{
			SaveScene();
		}
		else if (GetInput().IsJustPressed(Key::I))
		{
			OpenProject();
		}
	}
}
void FileMenu::Content()
{
	if (ImGui::MenuItem("New Project.."))
	{
		Editor::Get().CreateProject();
	}
	if (ImGui::MenuItem("Open Project..", "Ctrl+I"))
	{
		OpenProject();
	}
	if (ImGui::MenuItem("Open Scene..", "Ctrl+O"))
	{
		LoadScene();
	}
	if (ImGui::MenuItem("Save Scene..", "Ctrl+S"))
	{
		SaveScene();
	}
	if (ImGui::MenuItem("Save Scene As.."))
	{
		SaveScene();
	}
	ImGui::MenuItem("Save All..", "Ctrl+Shift+S");
}
void FileMenu::OpenProject()
{
	using namespace FileDialog;
	std::string fileName = FileDialog::OpenFile(EngineFileTypes::Project, L"Open Project", L"Open");
	if (!fileName.empty())
	{
		Editor::Get().OpenProject(fileName);
	}
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
	if (GetScene().sceneUri.empty()) {
		SaveSceneAs();
	}
	else {
		GetScene().Save();
	}
}
void FileMenu::SaveSceneAs()
{
	using namespace FileDialog;
	std::string fileName = FileDialog::SaveFile(EngineFileTypes::None, L"Export Scene", L"Save");
	if (!fileName.empty())
	{
		GetScene().SaveAs(fileName);
	}
}
}