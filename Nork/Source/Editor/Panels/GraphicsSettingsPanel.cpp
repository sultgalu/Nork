#include "pch.h"
#include "include/GraphicsSettingsPanel.h"

namespace Nork::Editor {
	GraphicsSettingsPanel::GraphicsSettingsPanel()
	{
	}

	void GraphicsSettingsPanel::Content()
	{
		auto& r = GetEngine().renderingSystem;

		if (ImGui::TreeNodeEx("Debug"))
		{
		}
	}
}
