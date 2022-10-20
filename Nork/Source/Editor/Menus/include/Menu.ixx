export module Nork.Editor.Menus:Menu;

import Nork.Editor.Views;

export namespace Nork::Editor {

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