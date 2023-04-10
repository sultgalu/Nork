#include "include/FileMenu.h"
#include "Platform/FileDialog.h"

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
	}
}
void FileMenu::Content()
{
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