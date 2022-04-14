#include "pch.h"
#include "../LogPanel.h"

constexpr size_t maxStreamSize = 4 * 10000;
constexpr size_t streamSrinkSize = 4 * 100;

std::stringstream stream("");

Nork::Editor2::LogPanel::LogPanel(EditorData& d) : Panel("Log", d)
{
	Logger::PushStream(stream);
	Logger::Info("Pushed imgui logger");
}

Nork::Editor2::LogPanel::~LogPanel()
{
	stream.clear();
}

void Nork::Editor2::LogPanel::DrawContent()
{
	std::string str = "no logs now sry"; //stream.str();
	ImGui::InputTextMultiline("LOG", str.data(), str.size());
	if (str.size() > maxStreamSize)
	{
		stream = std::stringstream(str.substr(streamSrinkSize, str.size()));
	}
}
