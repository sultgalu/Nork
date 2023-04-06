#pragma once
#include "Panels/include/Panel.h"
#include "Menus/include/Menu.h"

namespace Nork::Editor {
	class Panel;
	class Menu;

	class Editor
	{
	public:
		Editor();
		~Editor();
		static Editor& Get();
		void Render();
		void Update();
		void UpdateImguiInputs();
		void AddPanel(std::shared_ptr<Panel>);
	public:
		void DrawPanelManager();
		template<std::derived_from<Panel> T> std::shared_ptr<T> GetPanel()
		{
			for (auto& panel : panels)
			{
				if (auto casted = std::dynamic_pointer_cast<T>(panel))
				{
					return casted;
				}
			}
			return nullptr;
		}
	public:
		std::vector<std::shared_ptr<Panel>> panels;
		std::vector<std::shared_ptr<Menu>> menus;
		CommonData data;
	public:
	};
}