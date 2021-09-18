#include "pch.h"
#include "Panel.h"

namespace Nork::Editor
{
	void Panel::Draw()
	{
		// ImGui::Begin(this->name);
		// can set stuff based on global / local settings (color, font etc...)
		DrawContent();

		// ImGui::End();
	}
}