#pragma once

#include "Core/Engine.h"
#include "Panels/include/Panel.h"
#include "Menus/include/Menu.h"

namespace Nork::Editor {
	class Panel;
	class Menu;

	class Editor
	{
	public:
		Editor(Engine& engine);
		static Editor& Get();
		void Render();
		void Update();
		void UpdateImguiInputs();
		void AddViewportPanel();
		void AddPanel(std::shared_ptr<Panel>);
	private:
		void DrawPanelManager();
		template<std::derived_from<Panel> T> std::shared_ptr<T> GetPanel()
		{
			for (auto& panel : panels)
			{
				if (dynamic_cast<T*>(panel.get()))
				{
					return panel;
				}
			}
			return nullptr;
		}
	private:
		std::vector<std::shared_ptr<Panel>> panels;
		std::vector<std::shared_ptr<Menu>> menus;
		CommonData data;
		Engine& engine;
		int viewportPanelCount = 0;
	};
}