#pragma once

#include "Menu.h"

namespace Nork::Editor {
	class FileMenu: public Menu
	{
	public:
		using Menu::Menu;
		void Content() override;
		const char* GetName() override { return "File"; }
	private:
		void LoadScene();
		void SaveScene();
	};
}