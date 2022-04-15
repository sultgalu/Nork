#pragma once
#include "../../View.h"

namespace Nork::Editor {

	class Menu: public View
	{
	public:
		void Draw()
		{
			if (ImGui::BeginMenu(GetName()))
			{
				Content();
				ImGui::EndMenu();
			}
		}
	protected:
		virtual const char* GetName() = 0;
	public:
	};
}