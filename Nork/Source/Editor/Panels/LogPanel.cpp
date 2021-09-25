#include "pch.h"
#include "LogPanel.h"

constexpr size_t maxStreamSize = 4 * 10000;
constexpr size_t streamSrinkSize = 4 * 100;

std::stringstream stream("");

Nork::Editor::LogPanel::LogPanel() : Panel("Log", true)
{
	Logger::PushStream(stream);
	Logger::Info("Pushed imgui logger");
}

Nork::Editor::LogPanel::~LogPanel()
{
	stream.clear();
}

void Nork::Editor::LogPanel::DrawContent()
{
	ImGui::Begin("Log");

	std::string str = stream.str();
	ImGui::InputTextMultiline("LOG", str.data(), str.size());
	if (str.size() > maxStreamSize)
	{
		stream = std::stringstream(str.substr(streamSrinkSize, str.size()));
	}

	ImGui::End();
}
