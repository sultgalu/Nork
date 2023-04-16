#pragma once
#include "../../View.h"

namespace Nork::Editor {

	class Menu: public View
	{
	public:
		void Draw()
		{
			Shortcuts();
			if (ImGui::BeginMenu(GetName()))
			{
				Content();
				ImGui::EndMenu();
			}
		}
	protected:
		virtual void Shortcuts() = 0;
		virtual const char* GetName() = 0;
	public:
	};
}